"""SQLite persistence. All connections enforce foreign keys and close after use."""
import hashlib
import hmac
import json
import secrets
import sqlite3
import time
import uuid
from contextlib import contextmanager
from datetime import datetime, timezone
from pathlib import Path


def now():
    return datetime.now(timezone.utc).isoformat(timespec='milliseconds').replace('+00:00', 'Z')


def uid():
    return str(uuid.uuid4())


def password_hash(password):
    salt = secrets.token_hex(16)
    digest = hashlib.pbkdf2_hmac('sha256', password.encode(), bytes.fromhex(salt), 600000).hex()
    return f'pbkdf2_sha256$600000${salt}${digest}'


def password_matches(password, encoded):
    _, rounds, salt, expected = encoded.split('$')
    actual = hashlib.pbkdf2_hmac('sha256', password.encode(), bytes.fromhex(salt), int(rounds)).hex()
    return hmac.compare_digest(actual, expected)


class Database:
    def __init__(self, path):
        self.path = Path(path).resolve()
        self.path.parent.mkdir(parents=True, exist_ok=True)
        with self.connect() as db:
            db.execute('PRAGMA journal_mode=WAL')
            db.execute('CREATE TABLE IF NOT EXISTS schema_migrations (version TEXT PRIMARY KEY, applied_at TEXT NOT NULL)')
            for migration in sorted((Path(__file__).parent / 'migrations').glob('*.sql')):
                if not db.execute('SELECT 1 FROM schema_migrations WHERE version=?', (migration.name,)).fetchone():
                    # DDL and version marker commit atomically; startup owns the app lock.
                    db.executescript('BEGIN IMMEDIATE;\n' + migration.read_text(encoding='utf-8'))
                    db.execute('INSERT INTO schema_migrations VALUES (?,?)', (migration.name, now()))
                    db.commit()
            db.execute('INSERT OR IGNORE INTO settings VALUES (?,?)', ('device_key', uid()))
            db.execute('DELETE FROM sessions WHERE expires_at <= ?', (int(time.time()),))

    @contextmanager
    def connect(self):
        db = sqlite3.connect(self.path, timeout=15)
        db.row_factory = sqlite3.Row
        db.execute('PRAGMA foreign_keys=ON')
        db.execute('PRAGMA busy_timeout=15000')
        try:
            with db:
                yield db
        finally:
            db.close()

    def recover(self):
        with self.connect() as db:
            db.execute("UPDATE scans SET status='failed',error_code='interrupted',completed_at=? WHERE status='running'", (now(),))

    def user(self, user_id):
        with self.connect() as db:
            row = db.execute('SELECT u.id,u.email,u.username,u.created_at,p.display_name,p.bio FROM users u JOIN profiles p ON p.user_id=u.id WHERE u.id=?', (user_id,)).fetchone()
            return dict(row) if row else None

    def register(self, email, username, password):
        user_id, created = uid(), now()
        encoded = password_hash(password)
        with self.connect() as db:
            db.execute('INSERT INTO users VALUES (?,?,?,?,?)', (user_id,email,username,encoded,created))
            db.execute('INSERT INTO profiles VALUES (?,?,?,?)', (user_id,username,'',created))
        return self.user(user_id)

    def session(self, user_id, hours):
        token = secrets.token_urlsafe(32)
        with self.connect() as db:
            db.execute('DELETE FROM sessions WHERE expires_at <= ?', (int(time.time()),))
            db.execute('INSERT INTO sessions VALUES (?,?,?,?)', (hashlib.sha256(token.encode()).hexdigest(),user_id,int(time.time())+hours*3600,now()))
        return token

    def authenticate(self, token):
        with self.connect() as db:
            row = db.execute('SELECT user_id FROM sessions WHERE token_hash=? AND expires_at>?', (hashlib.sha256(token.encode()).hexdigest(),int(time.time()))).fetchone()
            return self.user(row['user_id']) if row else None

    def logout(self, token):
        with self.connect() as db:
            db.execute('DELETE FROM sessions WHERE token_hash=?', (hashlib.sha256(token.encode()).hexdigest(),))

    def throttle(self, bucket, maximum=15):
        stamp = int(time.time())
        with self.connect() as db:
            db.execute('BEGIN IMMEDIATE')
            db.execute('DELETE FROM login_attempts WHERE reset_at<=?', (stamp,))
            db.execute('INSERT INTO login_attempts VALUES (?,1,?) ON CONFLICT(bucket) DO UPDATE SET attempts=attempts+1', (bucket,stamp+900))
            return db.execute('SELECT attempts FROM login_attempts WHERE bucket=?', (bucket,)).fetchone()[0] > maximum

    def device(self, user_id, name):
        stamp = now()
        with self.connect() as db:
            key = db.execute("SELECT value FROM settings WHERE key='device_key'").fetchone()[0]
            db.execute('INSERT INTO devices VALUES (?,?,?,?,?,?,?) ON CONFLICT(user_id,device_key) DO NOTHING', (uid(),user_id,key,name,'{}',stamp,stamp))
            return db.execute('SELECT id FROM devices WHERE user_id=? AND device_key=?', (user_id,key)).fetchone()[0]

    def start_scan(self, user_id, device_id, scan_type, source, scan_id=None):
        scan_id = scan_id or uid()
        with self.connect() as db:
            db.execute('INSERT INTO scans (id,user_id,device_id,scan_type,source,status,created_at) VALUES (?,?,?,?,?,?,?)', (scan_id,user_id,device_id,scan_type,source,'running',now()))
        return scan_id

    def finish_scan(self, scan_id, result=None, error=None):
        stamp = now()
        with self.connect() as db:
            row = db.execute('SELECT * FROM scans WHERE id=?', (scan_id,)).fetchone()
            if not row:
                return
            status = 'failed' if error else ('partial' if result.get('status') == 'partial' else 'completed')
            if result is not None:
                result = dict(result, scan_id=scan_id, created_at=row['created_at'])
            db.execute('UPDATE scans SET status=?,result_json=?,error_code=?,completed_at=? WHERE id=?', (status,json.dumps(result,ensure_ascii=False) if result is not None else None,error,stamp,scan_id))
            if result and row['source'] == 'native':
                db.execute('UPDATE devices SET system_json=?,updated_at=? WHERE id=? AND user_id=?', (json.dumps(result['system']),stamp,row['device_id'],row['user_id']))

    def scan(self, user_id, scan_id):
        with self.connect() as db:
            row = db.execute('SELECT * FROM scans WHERE id=? AND user_id=?', (scan_id,user_id)).fetchone()
            if not row:
                return None
            item = dict(row)
            item['result'] = json.loads(item.pop('result_json')) if row['result_json'] else None
            return item

    def backup(self, destination):
        destination = Path(destination).resolve()
        if destination == self.path or destination.exists():
            raise ValueError('Backup destination must be a new file.')
        destination.parent.mkdir(parents=True, exist_ok=True)
        with self.connect() as source:
            target = sqlite3.connect(destination)
            try:
                source.backup(target)
            finally:
                target.close()
