"""Syspect desktop API. Loopback only; no third-party Python dependencies."""
import argparse
import hashlib
import json
import logging
import mimetypes
import os
import re
import socket
import sqlite3
import subprocess
import sys
import tempfile
import threading
import time
from http.cookies import SimpleCookie
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from urllib.parse import parse_qs, unquote, urlsplit

sys.path.insert(0, str(Path(__file__).resolve().parent))
from database import Database, now, password_hash, password_matches, uid

ROOT = Path(__file__).resolve().parent.parent
VERSION = '1.1.0'
PUBLIC = {p.name for p in ROOT.iterdir() if p.is_file() and p.suffix in {'.html','.css','.js','.png','.svg','.ico'}}
PROTECTED = {'dashboard.html','profile.html','history.html','Syspect.html','diagnosis.html','bsod.html','inputlag.html'}
MAX_BODY = 1024 * 1024
MAX_OUTPUT = 16 * 1024 * 1024


class APIError(Exception):
    def __init__(self, code, message, status=400):
        super().__init__(message)
        self.code, self.message, self.status = code, message, status


def text_field(data, key, minimum=1, maximum=120):
    value = data.get(key)
    if not isinstance(value, str) or not minimum <= len(value.strip()) <= maximum:
        raise APIError('validation_error', f'{key}: {minimum}-{maximum} characters required.')
    return value.strip()


def read_password(data, key='password'):
    value = data.get(key)
    if not isinstance(value, str) or not 10 <= len(value) <= 128:
        raise APIError('invalid_password', 'Use a password with 10-128 characters.')
    return value


def load_env(path):
    if path.exists():
        for line in path.read_text(encoding='utf-8-sig').splitlines():
            line = line.strip()
            if line and not line.startswith('#'):
                key, sep, value = line.partition('=')
                if not sep or not re.fullmatch(r'SYSPECT_[A-Z_]+', key.strip()):
                    raise ValueError('Invalid Syspect .env configuration.')
                os.environ.setdefault(key.strip(), os.path.expandvars(value.strip().strip('"')))


class App:
    def __init__(self, db_path, engine=None, timeout=300, session_hours=168, public_origin=''):
        self.public_origin = public_origin.rstrip('/')
        parsed = urlsplit(self.public_origin)
        if self.public_origin and (parsed.scheme != 'https' or not parsed.hostname or parsed.username or parsed.password or parsed.path or parsed.query or parsed.fragment or parsed.netloc != parsed.hostname):
            raise ValueError('SYSPECT_PUBLIC_ORIGIN must be one HTTPS origin without a path or port.')
        self.public_host = parsed.netloc
        self.db = Database(db_path)
        self.engine = Path(engine or ROOT / 'diagnosticengine.exe').resolve()
        self.timeout, self.session_hours = timeout, session_hours
        self.scan_lock = threading.Lock()
        self.workers = []
        self.stopping = False
        self.cookie_name = 'syspect_' + hashlib.sha256(str(self.db.path).encode()).hexdigest()[:12]
        self.dummy_hash = password_hash('not-an-account-' + uid())
        self.last_request = time.monotonic()

    def run_engine(self):
        if not self.engine.is_file():
            raise APIError('engine_not_found', 'diagnosticengine.exe is missing.', 503)
        with tempfile.TemporaryDirectory(prefix='syspect-scan-') as folder:
            output, errors = Path(folder)/'stdout.json', Path(folder)/'stderr.txt'
            with output.open('wb') as out, errors.open('wb') as err:
                try:
                    process = subprocess.Popen([str(self.engine),'--json'], cwd=folder, stdout=out, stderr=err,
                                               creationflags=subprocess.CREATE_NO_WINDOW if os.name == 'nt' else 0)
                except OSError as exc:
                    raise APIError('engine_start_failed', 'Diagnostic engine could not start.', 503) from exc
                start = time.monotonic()
                try:
                    while process.poll() is None:
                        if time.monotonic()-start > self.timeout:
                            raise APIError('engine_timeout', 'Diagnostic engine timed out.', 504)
                        if output.stat().st_size + errors.stat().st_size > MAX_OUTPUT:
                            raise APIError('engine_output_too_large', 'Diagnostic output exceeded its limit.', 502)
                        time.sleep(.1)
                finally:
                    if process.poll() is None:
                        process.kill()
                    process.wait()
            if output.stat().st_size + errors.stat().st_size > MAX_OUTPUT:
                raise APIError('engine_output_too_large', 'Diagnostic output exceeded its limit.', 502)
            if process.returncode:
                raise APIError('engine_failed', 'Diagnostic engine exited with an error.', 502)
            try:
                result = json.loads(output.read_text(encoding='utf-8-sig'))
            except (ValueError, UnicodeError) as exc:
                raise APIError('invalid_engine_json', 'Diagnostic engine returned invalid JSON.', 502) from exc
            if not isinstance(result, dict) or result.get('schema_version') != '1.0' or not isinstance(result.get('engine'), dict) or result['engine'].get('mode') != 'native' or not isinstance(result.get('raw_report'), str):
                raise APIError('invalid_engine_schema', 'Unsupported engine report.', 502)
            if any(not isinstance(result.get(key), dict) for key in ['system','hardware','drivers','events','coverage']):
                raise APIError('invalid_engine_schema', 'Engine report is missing required fields.', 502)
            result['engine'].update(agent_version=VERSION, transport='syspect-desktop-localhost')
            return result

    def native_scan(self, user_id, background=True):
        if not self.scan_lock.acquire(blocking=False):
            raise APIError('scan_in_progress', 'Another diagnostic scan is running.', 409)
        try:
            device_id = self.db.device(user_id, socket.gethostname())
            scan_id = self.db.start_scan(user_id, device_id, 'full', 'native')
        except Exception:
            self.scan_lock.release()
            raise

        def work():
            try:
                self.db.finish_scan(scan_id, self.run_engine())
            except APIError as exc:
                self.db.finish_scan(scan_id, error=exc.code)
            except Exception:
                logging.exception('Scan failed: %s', scan_id)
                self.db.finish_scan(scan_id, error='internal_error')
            finally:
                self.scan_lock.release()
        if background:
            thread = threading.Thread(target=work, daemon=False)
            self.workers = [t for t in self.workers if t.is_alive()]
            self.workers.append(thread)
            thread.start()
        else:
            work()
        return self.db.scan(user_id, scan_id)


class Server(ThreadingHTTPServer):
    daemon_threads = True
    allow_reuse_address = False
    def __init__(self, port, app):
        self.app = app
        super().__init__(('127.0.0.1', port), Handler)


class Handler(BaseHTTPRequestHandler):
    server_version = 'Syspect/' + VERSION
    sys_version = ''

    def setup(self):
        super().setup()
        self.connection.settimeout(15)

    def log_message(self, fmt, *args):
        # Do not log payloads, passwords, tokens or query strings.
        logging.info('%s %s', self.command, self.path.split('?')[0])

    @property
    def app(self):
        return self.server.app

    def send(self, status, value=None, extra=None, content_type='application/json; charset=utf-8'):
        body = value if isinstance(value, bytes) else json.dumps(value, ensure_ascii=False, allow_nan=False).encode('utf-8')
        self.send_response(status)
        self.send_header('Content-Type', content_type)
        self.send_header('Content-Length', str(len(body)))
        self.send_header('Cache-Control','no-store')
        self.send_header('X-Content-Type-Options','nosniff')
        self.send_header('X-Frame-Options','DENY')
        self.send_header('Referrer-Policy','no-referrer')
        self.send_header('Content-Security-Policy', "default-src 'self'; script-src 'self' 'unsafe-inline'; style-src 'self' 'unsafe-inline' https://fonts.googleapis.com; font-src 'self' https://fonts.gstatic.com; img-src 'self' data:; connect-src 'self'; object-src 'none'; base-uri 'none'; frame-ancestors 'none'; form-action 'self'")
        self.send_header('Connection', 'close')
        for key, item in (extra or {}).items():
            self.send_header(key,item)
        self.end_headers()
        self.close_connection = True
        if self.command != 'HEAD':
            self.wfile.write(body)

    def cookie(self, token='', expire=False):
        secure = '; Secure' if self.app.public_origin and self.headers.get('Host') == self.app.public_host else ''
        return f'{self.app.cookie_name}={token}; Path=/; HttpOnly; SameSite=Strict; Max-Age={0 if expire else self.app.session_hours*3600}' + secure

    def token(self):
        cookie = SimpleCookie()
        try:
            cookie.load(self.headers.get('Cookie',''))
            return cookie[self.app.cookie_name].value if self.app.cookie_name in cookie else ''
        except Exception:
            return ''

    def body(self):
        if self.headers.get('Transfer-Encoding'):
            raise APIError('invalid_request', 'Transfer-Encoding is not supported.')
        raw_length = self.headers.get_all('Content-Length') or []
        if len(raw_length) != 1 or not raw_length[0].isdigit():
            raise APIError('invalid_request', 'One Content-Length header is required.')
        length = int(raw_length[0])
        if length > MAX_BODY:
            raise APIError('payload_too_large', 'Request exceeds 1 MB.', 413)
        if self.headers.get_content_type() != 'application/json':
            raise APIError('invalid_content_type', 'JSON body required.', 415)
        raw = self.rfile.read(length)
        if len(raw) != length:
            raise APIError('invalid_request', 'Incomplete request.')
        try:
            value = json.loads(raw.decode('utf-8'), parse_constant=lambda _: (_ for _ in ()).throw(ValueError()))
        except (ValueError, UnicodeError, RecursionError):
            raise APIError('invalid_json', 'Invalid JSON.')
        if not isinstance(value, dict):
            raise APIError('invalid_json', 'JSON object required.')
        return value

    def dispatch(self):
        port = self.server.server_port
        allowed_hosts = {f'127.0.0.1:{port}',f'localhost:{port}'}
        allowed_origins = {f'http://127.0.0.1:{port}',f'http://localhost:{port}'}
        if self.app.public_origin:
            allowed_hosts.add(self.app.public_host)
            allowed_origins.add(self.app.public_origin)
        if self.headers.get('Host') not in allowed_hosts:
            raise APIError('forbidden_host', 'Invalid host.', 403)
        origin = self.headers.get('Origin')
        if origin is not None and origin not in allowed_origins:
            raise APIError('forbidden_origin', 'This origin is not allowed.', 403)
        if self.headers.get('Sec-Fetch-Site') == 'cross-site':
            raise APIError('forbidden_origin', 'Cross-site requests are not allowed.', 403)
        if self.command in {'POST','PATCH','DELETE'} and self.headers.get('X-Syspect-Client') != 'desktop':
            raise APIError('csrf_rejected', 'Syspect request header required.', 403)
        self.app.last_request = time.monotonic()
        path = urlsplit(self.path).path
        method = self.command
        db = self.app.db
        if path == '/api/v1/app' and method == 'GET':
            return self.send(200, {'name':'Syspect','version':VERSION,'desktop':True,'host':'syspect-desktop-host','platform':'windows','transport':'localhost','database':'sqlite','storage':'local','stopping':self.app.stopping,'instance':hashlib.sha256((str(ROOT)+str(db.path)).encode()).hexdigest()})
        if path == '/api/v1/health' and method == 'GET':
            with db.connect() as conn:
                conn.execute('SELECT 1').fetchone()
            return self.send(200, {'status':'ok','version':VERSION,'database':'ok','engine_mode':'native','engine_available':self.app.engine.is_file(),'scan_in_progress':self.app.scan_lock.locked()})
        if path in {'/api/v1/auth/register','/api/v1/auth/login'} and method == 'POST':
            data = self.body()
            email = text_field(data,'email',3,254).lower()
            if not re.fullmatch(r'[^\s@]+@[^\s@]+\.[^\s@]+',email):
                raise APIError('invalid_email', 'Enter a valid email address.')
            if db.throttle('all-auth',100) or db.throttle('auth:'+hashlib.sha256(email.encode()).hexdigest()):
                raise APIError('rate_limited','Too many attempts. Try again in 15 minutes.',429)
            password = read_password(data)
            if path.endswith('/register'):
                username = text_field(data,'username',3,32)
                if not re.fullmatch(r'[\w.-]+',username):
                    raise APIError('invalid_username','Username may contain letters, numbers, dots, hyphens and underscores.')
                try:
                    user = db.register(email,username,password)
                except sqlite3.IntegrityError:
                    raise APIError('account_exists','Email or username is already registered.',409)
            else:
                with db.connect() as conn:
                    row = conn.execute('SELECT id,password_hash FROM users WHERE email=?',(email,)).fetchone()
                valid = password_matches(password,row['password_hash'] if row else self.app.dummy_hash)
                if not row or not valid:
                    raise APIError('invalid_credentials','Email or password is incorrect.',401)
                user = db.user(row['id'])
            db.logout(self.token())
            token = db.session(user['id'],self.app.session_hours)
            return self.send(201 if path.endswith('/register') else 200, {'user':user}, {'Set-Cookie':self.cookie(token)})
        user = db.authenticate(self.token())
        if not path.startswith('/api/'):
            if method not in {'GET','HEAD'}:
                raise APIError('method_not_allowed','GET required.',405)
            name = unquote(path).lstrip('/') or 'index.html'
            if name not in PUBLIC:
                raise APIError('not_found','Resource not found.',404)
            if name in PROTECTED and not user:
                return self.send(302,b'',{'Location':'/login.html'})
            return self.send(200,(ROOT/name).read_bytes(),content_type=(mimetypes.guess_type(name)[0] or 'application/octet-stream') + ('; charset=utf-8' if name.endswith(('.html','.css','.js')) else ''))
        if not user:
            raise APIError('unauthorized','Sign in to continue.',401)
        user_id = user['id']
        if path == '/api/v1/auth/me' and method == 'GET':
            return self.send(200,{'user':user})
        if path == '/api/v1/app/exit' and method == 'POST':
            self.body()
            if self.app.scan_lock.locked():
                raise APIError('scan_in_progress','Wait for the scan to finish.',409)
            self.app.stopping = True
            return self.send(200,{'ok':True})
        if path == '/api/v1/auth/logout' and method == 'POST':
            self.body()
            db.logout(self.token())
            return self.send(200,{'ok':True},{'Set-Cookie':self.cookie(expire=True)})
        if path == '/api/v1/auth/password' and method == 'POST':
            data = self.body()
            if db.throttle('password:'+user_id):
                raise APIError('rate_limited','Too many attempts. Try again later.',429)
            current, new = read_password(data,'current_password'), read_password(data,'new_password')
            with db.connect() as conn:
                row = conn.execute('SELECT password_hash FROM users WHERE id=?',(user_id,)).fetchone()
                if not password_matches(current,row[0]):
                    raise APIError('invalid_credentials','Current password is incorrect.',401)
                encoded = password_hash(new)
                # Compare-and-swap avoids racing password changes.
                changed = conn.execute('UPDATE users SET password_hash=? WHERE id=? AND password_hash=?',(encoded,user_id,row[0])).rowcount
                if not changed:
                    raise APIError('conflict','Password changed. Sign in again.',409)
                conn.execute('DELETE FROM sessions WHERE user_id=?',(user_id,))
            return self.send(200,{'ok':True},{'Set-Cookie':self.cookie(expire=True)})
        if path == '/api/v1/profile' and method in {'GET','PATCH'}:
            if method == 'PATCH':
                data = self.body()
                name, bio = text_field(data,'display_name',1,80), text_field(data,'bio',0,500)
                with db.connect() as conn:
                    conn.execute('UPDATE profiles SET display_name=?,bio=?,updated_at=? WHERE user_id=?',(name,bio,now(),user_id))
            return self.send(200,{'user':db.user(user_id)})
        if path == '/api/v1/devices' and method == 'GET':
            with db.connect() as conn:
                rows = conn.execute('SELECT id,name,system_json,created_at,updated_at FROM devices WHERE user_id=? ORDER BY updated_at DESC',(user_id,)).fetchall()
            items = []
            for row in rows:
                item = dict(row)
                item['system'] = json.loads(item.pop('system_json'))
                items.append(item)
            return self.send(200,{'devices':items})
        if path.startswith('/api/v1/devices/') and method == 'PATCH':
            name = text_field(self.body(),'name',1,80)
            with db.connect() as conn:
                count = conn.execute('UPDATE devices SET name=?,updated_at=? WHERE id=? AND user_id=?',(name,now(),path.rsplit('/',1)[1],user_id)).rowcount
            if not count:
                raise APIError('not_found','Device not found.',404)
            return self.send(200,{'ok':True})
        if path == '/api/v1/scans' and method == 'GET':
            query = parse_qs(urlsplit(self.path).query)
            try:
                limit = min(100,max(1,int(query.get('limit',['20'])[0])))
                offset = max(0,int(query.get('offset',['0'])[0]))
            except ValueError:
                raise APIError('validation_error','Invalid pagination.')
            with db.connect() as conn:
                total = conn.execute('SELECT count(*) FROM scans WHERE user_id=?',(user_id,)).fetchone()[0]
                rows = conn.execute('SELECT s.id,s.device_id,d.name AS device_name,s.scan_type,s.source,s.status,s.error_code,s.created_at,s.completed_at FROM scans s JOIN devices d ON d.id=s.device_id WHERE s.user_id=? ORDER BY s.created_at DESC,s.id DESC LIMIT ? OFFSET ?',(user_id,limit,offset)).fetchall()
            return self.send(200,{'scans':[dict(r) for r in rows],'total':total,'limit':limit,'offset':offset})
        if path == '/api/v1/scans' and method == 'POST':
            data = self.body()
            source = data.get('source','native')
            if source == 'native':
                scan = self.app.native_scan(user_id)
                return self.send(202,{'scan':scan})
            if source != 'questionnaire' or data.get('scan_type') not in {'fpsDrops','overheating','bsod','inputlag'}:
                raise APIError('validation_error','Unsupported diagnostic type.')
            result = data.get('result')
            if not isinstance(result,dict) or not isinstance(result.get('answers'),dict) or not isinstance(result.get('diagnoses'),dict):
                raise APIError('validation_error','Questionnaire answers and diagnoses are required.')
            # Client-generated reports are always labelled questionnaire, never native telemetry.
            result = dict(result, source='questionnaire',status='completed')
            request_id = text_field(data,'request_id',36,36)
            if not re.fullmatch(r'[0-9a-fA-F-]{36}',request_id):
                raise APIError('validation_error','Invalid request ID.')
            existing = db.scan(user_id,request_id)
            if existing:
                return self.send(200,{'scan':existing})
            device_id = db.device(user_id,socket.gethostname())
            try:
                scan_id = db.start_scan(user_id,device_id,data['scan_type'],source,request_id)
            except sqlite3.IntegrityError:
                existing = db.scan(user_id,request_id)
                if existing:
                    return self.send(200,{'scan':existing})
                raise APIError('conflict','Request ID is already in use.',409)
            db.finish_scan(scan_id,result)
            return self.send(201,{'scan':db.scan(user_id,scan_id)})
        if path == '/api/v1/scan' and method == 'POST':
            self.body()
            scan = self.app.native_scan(user_id,background=False)
            if scan['status'] == 'failed':
                raise APIError(scan['error_code'],'Scan failed. See scan history.',502)
            return self.send(200,scan['result'])
        match = re.fullmatch(r'/api/v1/scans/([\w-]+)(/export)?',path)
        if match and method in {'GET','DELETE'}:
            scan_id, exporting = match.groups()
            scan = db.scan(user_id,scan_id)
            if not scan:
                raise APIError('not_found','Scan not found.',404)
            if method == 'DELETE':
                self.body()
                if scan['status'] == 'running':
                    raise APIError('scan_in_progress','Wait for the scan to finish.',409)
                with db.connect() as conn:
                    conn.execute('DELETE FROM scans WHERE id=? AND user_id=?',(scan_id,user_id))
                return self.send(200,{'ok':True})
            return self.send(200,scan if exporting else {'scan':scan}, {'Content-Disposition':f'attachment; filename="syspect-{scan_id}.json"'} if exporting else None)
        raise APIError('not_found','Endpoint not found.',404)

    def handle_request(self):
        try:
            self.dispatch()
        except APIError as exc:
            self.send(exc.status,{'error':{'code':exc.code,'message':exc.message}})
        except (BrokenPipeError,ConnectionResetError,TimeoutError):
            pass
        except Exception:
            logging.exception('Request failed')
            try:
                self.send(500,{'error':{'code':'internal_error','message':'Request failed. Check the local application log.'}})
            except OSError:
                pass

    do_GET = do_POST = do_PATCH = do_DELETE = do_HEAD = handle_request


def main():
    load_env(ROOT/'.env')
    parser = argparse.ArgumentParser()
    parser.add_argument('--port',type=int,default=int(os.getenv('SYSPECT_PORT','37841')))
    parser.add_argument('--data-dir',default=os.getenv('SYSPECT_DATA_DIR',str(Path(os.getenv('LOCALAPPDATA',str(ROOT)))/'Syspect'/'data')))
    parser.add_argument('--ready-file',type=Path)
    parser.add_argument('--migrate',action='store_true')
    parser.add_argument('--backup',type=Path)
    args = parser.parse_args()
    data_dir = Path(args.data_dir).resolve()
    data_dir.mkdir(parents=True,exist_ok=True)
    logging.basicConfig(filename=data_dir/'server.log',level=logging.INFO,format='%(asctime)s %(levelname)s %(message)s',encoding='utf-8')
    db_path = data_dir/'syspect.sqlite3'
    if args.backup:
        Database(db_path).backup(args.backup)
        return
    # A process-wide lock prevents two hosts from recovering each other's scans.
    lock = (data_dir/'host.lock').open('a+b')
    try:
        if os.name == 'nt':
            import msvcrt
            lock.seek(0)
            if not lock.read(1):
                lock.write(b'0'); lock.flush()
            lock.seek(0)
            for attempt in range(15):
                try:
                    msvcrt.locking(lock.fileno(),msvcrt.LK_NBLCK,1)
                    break
                except OSError:
                    if attempt == 14:
                        raise
                    time.sleep(.2)
        else:
            import fcntl
            fcntl.flock(lock,fcntl.LOCK_EX | fcntl.LOCK_NB)
    except OSError:
        logging.error('Another Syspect host is using this data directory.')
        return 2
    app = App(db_path,timeout=int(os.getenv('SYSPECT_SCAN_TIMEOUT_SECONDS','300')),session_hours=int(os.getenv('SYSPECT_SESSION_HOURS','168')),public_origin=os.getenv('SYSPECT_PUBLIC_ORIGIN',''))
    if app.timeout < 1 or not 1 <= app.session_hours <= 720:
        raise ValueError('Invalid timeout or session lifetime.')
    app.db.recover()
    if args.migrate:
        return 0
    server = None
    for port in range(args.port,args.port+20):
        try:
            server = Server(port,app)
            break
        except OSError:
            continue
    if not server:
        raise RuntimeError('No free Syspect port found.')
    if args.ready_file:
        args.ready_file.write_text(json.dumps({'port':server.server_port,'pid':os.getpid()}),encoding='utf-8')
    logging.info('Syspect %s listening on 127.0.0.1:%s',VERSION,server.server_port)
    # Long idle exit; the scan worker keeps the host alive while a scan runs.
    server.timeout = 1
    try:
        while not app.stopping and (time.monotonic()-app.last_request < 14400 or app.scan_lock.locked()):
            server.handle_request()
    except KeyboardInterrupt:
        pass
    finally:
        for worker in app.workers:
            worker.join()
        server.server_close()
        lock.close()
    return 0


if __name__ == '__main__':
    sys.exit(main())
