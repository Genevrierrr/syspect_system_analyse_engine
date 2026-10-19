r"""Run: runtime\python.exe -m unittest discover -s tests -v"""
import http.client
import json
import sqlite3
import sys
import tempfile
import threading
import time
import unittest
import uuid
from pathlib import Path

sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'backend'))
from server import App, Server, APIError
from database import Database

class Client:
    def __init__(self, server):
        self.port=server.server_port
        self.cookie=''
    def request(self,method,path,data=None,headers=None):
        conn=http.client.HTTPConnection('127.0.0.1',self.port,timeout=15)
        payload=json.dumps(data,ensure_ascii=False).encode() if data is not None else None
        fields={'X-Syspect-Client':'desktop','Content-Type':'application/json','Cookie':self.cookie}
        fields.update(headers or {})
        conn.request(method,path,payload,fields)
        response=conn.getresponse()
        cookie=response.getheader('Set-Cookie')
        if cookie:self.cookie=cookie.split(';')[0]
        raw=response.read();status=response.status
        try:value=json.loads(raw)
        except ValueError:value=raw
        conn.close()
        return status,value,cookie
    def register(self,suffix='a'):
        return self.request('POST','/api/v1/auth/register',{'email':suffix+'@example.test','username':'user_'+suffix,'password':'test-long-password'})

def report():
    return {'schema_version':'1.0','engine':{'mode':'native'},'system':{'cpu':'Test CPU','ram_gb':16},'hardware':{},'drivers':{},'events':{},'coverage':{},'raw_report':'Ünicode <script>not markup</script>','status':'partial'}

class IntegrationTests(unittest.TestCase):
    def setUp(self):
        self.temp=tempfile.TemporaryDirectory()
        self.path=Path(self.temp.name)/'db.sqlite3'
        self.app=App(self.path)
        self.server=Server(0,self.app)
        self.thread=threading.Thread(target=self.server.serve_forever,daemon=True);self.thread.start()
        self.client=Client(self.server)
    def tearDown(self):
        for thread in self.app.workers:thread.join(10)
        self.server.shutdown();self.server.server_close();self.thread.join()
        self.temp.cleanup()
    def call(self,method,path,data=None,expected=200,client=None,headers=None):
        status,result,_=(client or self.client).request(method,path,data,headers)
        self.assertEqual(status,expected,result)
        return result
    def sign_in(self):
        status,result,cookie=self.client.register();self.assertEqual(status,201,result)
        return result['user'],cookie
    def questionnaire(self):
        return self.call('POST','/api/v1/scans',{'source':'questionnaire','scan_type':'bsod','request_id':str(uuid.uuid4()),'result':{'answers':{'q1':1},'diagnoses':{},'text':'öçşığÜ <img src=x onerror=alert(1)>'}},201)['scan']
    def test_registration_passwords_cookie_and_duplicate(self):
        user,cookie=self.sign_in();self.assertIn('HttpOnly',cookie);self.assertIn('SameSite=Strict',cookie)
        self.assertNotIn('password',str(user))
        with self.app.db.connect() as db:
            encoded=db.execute('SELECT password_hash FROM users').fetchone()[0]
            self.assertTrue(encoded.startswith('pbkdf2_sha256$600000$'))
            self.assertNotIn('test-long-password',encoded)
            self.assertEqual(db.execute('PRAGMA foreign_keys').fetchone()[0],1)
        self.call('POST','/api/v1/auth/register',{'email':'A@EXAMPLE.TEST','username':'other','password':'test-long-password'},409)
        self.call('POST','/api/v1/auth/register',{'email':'other@example.test','username':'other','password':'short'},400)
    def test_login_logout_password_rotation(self):
        self.sign_in();old_cookie=self.client.cookie
        self.call('POST','/api/v1/auth/logout',{})
        self.call('GET','/api/v1/auth/me',expected=401)
        self.call('POST','/api/v1/auth/login',{'email':'a@example.test','password':'wrong-password'},401)
        self.call('POST','/api/v1/auth/login',{'email':'a@example.test','password':'test-long-password'})
        self.call('POST','/api/v1/auth/password',{'current_password':'test-long-password','new_password':'new-long-password'})
        self.call('GET','/api/v1/auth/me',expected=401)
        self.client.cookie=old_cookie;self.call('GET','/api/v1/auth/me',expected=401)
        self.call('POST','/api/v1/auth/login',{'email':'a@example.test','password':'new-long-password'})
    def test_profile_unicode_survives_database_reopen(self):
        user,_=self.sign_in()
        self.call('PATCH','/api/v1/profile',{'display_name':'İbrahim Şahin','bio':'çığ <b>metin</b>'})
        reopened=Database(self.path)
        self.assertEqual(reopened.user(user['id'])['display_name'],'İbrahim Şahin')
        self.assertEqual(reopened.user(user['id'])['bio'],'çığ <b>metin</b>')
    def test_scan_ownership_export_delete_and_devices(self):
        self.sign_in();scan=self.questionnaire()
        self.assertEqual(scan['source'],'questionnaire')
        other=Client(self.server);self.assertEqual(other.register('b')[0],201)
        self.assertEqual(self.call('GET','/api/v1/scans',client=other)['total'],0)
        for suffix in ['', '/export']:
            self.call('GET','/api/v1/scans/'+scan['id']+suffix,expected=404,client=other)
        self.call('DELETE','/api/v1/scans/'+scan['id'],{},404,other)
        self.call('PATCH','/api/v1/devices/'+scan['device_id'],{'name':'stolen'},404,other)
        exported=self.call('GET','/api/v1/scans/'+scan['id']+'/export')
        self.assertEqual(exported['result']['text'],'öçşığÜ <img src=x onerror=alert(1)>')
        self.call('PATCH','/api/v1/devices/'+scan['device_id'],{'name':'My PC'})
        self.assertEqual(self.call('GET','/api/v1/devices')['devices'][0]['name'],'My PC')
        self.call('DELETE','/api/v1/scans/'+scan['id'],{})
        self.assertEqual(self.call('GET','/api/v1/scans')['total'],0)
    def test_idempotency_and_pagination(self):
        self.sign_in();key=str(uuid.uuid4())
        body={'source':'questionnaire','scan_type':'inputlag','request_id':key,'result':{'answers':{},'diagnoses':{}}}
        self.call('POST','/api/v1/scans',body,201)
        self.call('POST','/api/v1/scans',body)
        self.questionnaire();self.questionnaire()
        page=self.call('GET','/api/v1/scans?limit=1&offset=1')
        self.assertEqual(page['total'],3);self.assertEqual(len(page['scans']),1)
        self.call('GET','/api/v1/scans?limit=oops',expected=400)
    def test_native_async_persists_and_prevents_parallel_execution(self):
        self.sign_in();release=threading.Event()
        def engine():release.wait(5);return report()
        self.app.run_engine=engine
        scan=self.call('POST','/api/v1/scans',{'source':'native'},202)['scan']
        self.call('POST','/api/v1/scans',{'source':'native'},409)
        self.call('DELETE','/api/v1/scans/'+scan['id'],{},409)
        release.set();self.app.workers[-1].join()
        saved=self.call('GET','/api/v1/scans/'+scan['id'])['scan']
        self.assertEqual(saved['status'],'partial');self.assertEqual(saved['result']['raw_report'],report()['raw_report'])
        self.assertEqual(self.call('GET','/api/v1/devices')['devices'][0]['system']['cpu'],'Test CPU')
    def test_failed_and_interrupted_scans_remain_in_history(self):
        user,_=self.sign_in()
        def broken():raise APIError('engine_timeout','Timeout',504)
        self.app.run_engine=broken
        scan=self.call('POST','/api/v1/scans',{},202)['scan'];self.app.workers[-1].join()
        saved=self.call('GET','/api/v1/scans/'+scan['id'])['scan'];self.assertEqual(saved['status'],'failed');self.assertEqual(saved['error_code'],'engine_timeout')
        pending=self.app.db.start_scan(user['id'],scan['device_id'],'full','native')
        reopened=Database(self.path);reopened.recover()
        self.assertEqual(reopened.scan(user['id'],pending)['error_code'],'interrupted')
    def test_static_allowlist_auth_and_csrf(self):
        self.call('GET','/profile.html',expected=302)
        for path in ['/backend/server.py','/.env','/runtime/python.exe','/../backend/database.py','/%2e%2e/backend/database.py','/desktop/SyspectHost.ps1']:
            self.call('GET',path,expected=404)
        self.call('GET','/api/v1/app',expected=403,headers={'Host':'attacker.test'})
        self.call('GET','/api/v1/health',expected=403,headers={'Origin':'https://attacker.test'})
        self.call('POST','/api/v1/auth/login',{},403,headers={'X-Syspect-Client':''})
        self.call('GET','/api/v1/scans',expected=401)
    def test_login_rate_limit(self):
        for _ in range(15):
            self.call('POST','/api/v1/auth/login',{'email':'none@example.test','password':'test-long-password'},401)
        self.call('POST','/api/v1/auth/login',{'email':'none@example.test','password':'test-long-password'},429)
    def test_migrations_backup_and_no_seed_account(self):
        with self.app.db.connect() as db:self.assertEqual(db.execute('SELECT count(*) FROM users').fetchone()[0],0)
        self.sign_in();self.questionnaire()
        Database(self.path);Database(self.path)
        target=Path(self.temp.name)/'backup.sqlite3';self.app.db.backup(target)
        with sqlite3.connect(target) as db:
            self.assertEqual(db.execute('PRAGMA integrity_check').fetchone()[0],'ok')
            self.assertEqual(db.execute('SELECT count(*) FROM scans').fetchone()[0],1)
            self.assertEqual(db.execute('SELECT count(*) FROM schema_migrations').fetchone()[0],1)
        db.close()
    def test_input_validation_and_sql_injection(self):
        self.sign_in()
        self.call('POST','/api/v1/scans',{'source':'native_fake'},400)
        self.call('PATCH','/api/v1/profile',{'display_name':"Robert'); DROP TABLE users;--",'bio':''})
        self.assertIn('DROP TABLE',self.call('GET','/api/v1/auth/me')['user']['display_name'])
        self.call('POST','/api/v1/scans',{'source':'questionnaire','scan_type':'full','result':{}},400)
        self.call('POST','/api/v1/auth/logout',{},415,headers={'Content-Type':'text/plain'})
    def test_expired_sessions(self):
        self.sign_in()
        with self.app.db.connect() as db:db.execute('UPDATE sessions SET expires_at=0')
        self.call('GET','/api/v1/auth/me',expected=401)

    def test_ngrok_exact_origin_cookie_and_csrf(self):
        self.app.public_origin='https://prognosis-scarily-qualify.ngrok-free.dev'
        self.app.public_host='prognosis-scarily-qualify.ngrok-free.dev'
        headers={'Host':self.app.public_host,'Origin':self.app.public_origin}
        status,value,cookie=self.client.request('POST','/api/v1/auth/register',{'email':'tunnel@example.test','username':'tunnel_user','password':'test-long-password'},headers)
        self.assertEqual(status,201,value);self.assertIn('; Secure',cookie)
        self.call('GET','/api/v1/auth/me',headers=headers)
        self.call('PATCH','/api/v1/profile',{'display_name':'Tunnel user','bio':''},headers=headers)
        self.call('GET','/api/v1/auth/me',expected=403,headers={**headers,'Origin':'https://other.ngrok-free.dev'})
        self.call('GET','/api/v1/auth/me',expected=403,headers={**headers,'Host':'other.ngrok-free.dev'})
        self.call('POST','/api/v1/auth/logout',{},403,headers={**headers,'X-Syspect-Client':''})

    def test_public_origin_configuration_validation(self):
        for origin in ['http://example.com','https://example.com/path','https://user:pass@example.com','https://example.com:443','https://example.com?x=1']:
            with self.assertRaises(ValueError):App(Path(self.temp.name)/'invalid.sqlite3',public_origin=origin)

if __name__=='__main__':unittest.main()
