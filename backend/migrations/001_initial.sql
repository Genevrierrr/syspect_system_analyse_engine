CREATE TABLE users (
 id TEXT PRIMARY KEY, email TEXT NOT NULL COLLATE NOCASE UNIQUE,
 username TEXT NOT NULL COLLATE NOCASE UNIQUE, password_hash TEXT NOT NULL,
 created_at TEXT NOT NULL
);
CREATE TABLE profiles (
 user_id TEXT PRIMARY KEY REFERENCES users(id) ON DELETE CASCADE,
 display_name TEXT NOT NULL, bio TEXT NOT NULL DEFAULT '', updated_at TEXT NOT NULL
);
CREATE TABLE sessions (
 token_hash TEXT PRIMARY KEY, user_id TEXT NOT NULL REFERENCES users(id) ON DELETE CASCADE,
 expires_at INTEGER NOT NULL, created_at TEXT NOT NULL
);
CREATE INDEX sessions_user ON sessions(user_id);
CREATE TABLE devices (
 id TEXT PRIMARY KEY, user_id TEXT NOT NULL REFERENCES users(id) ON DELETE CASCADE,
 device_key TEXT NOT NULL, name TEXT NOT NULL, system_json TEXT NOT NULL DEFAULT '{}',
 created_at TEXT NOT NULL, updated_at TEXT NOT NULL,
 UNIQUE(user_id,device_key), UNIQUE(id,user_id)
);
CREATE TABLE scans (
 id TEXT PRIMARY KEY, user_id TEXT NOT NULL REFERENCES users(id) ON DELETE CASCADE,
 device_id TEXT NOT NULL, scan_type TEXT NOT NULL,
 source TEXT NOT NULL CHECK(source IN ('native','questionnaire')),
 status TEXT NOT NULL CHECK(status IN ('running','completed','partial','failed')),
 result_json TEXT, error_code TEXT, created_at TEXT NOT NULL, completed_at TEXT,
 FOREIGN KEY(device_id,user_id) REFERENCES devices(id,user_id) ON DELETE CASCADE
);
CREATE INDEX scans_history ON scans(user_id,created_at DESC,id DESC);
CREATE INDEX scans_device ON scans(device_id);
CREATE TABLE login_attempts (bucket TEXT PRIMARY KEY, attempts INTEGER NOT NULL, reset_at INTEGER NOT NULL);
CREATE TABLE settings (key TEXT PRIMARY KEY, value TEXT NOT NULL);
