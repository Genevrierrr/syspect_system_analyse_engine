# Syspect 1.1.0 — Veritabanı entegrasyonu

Ngrok üzerinden denemek için **Start-Ngrok-Test.cmd** dosyasını aç. Adres ve adımlar: [NGROK-TEST.md](NGROK-TEST.md). Normal başlatma yerel kalır; ngrok başlatıcısı ayrı test veritabanı kullanır.

## Kullanım

1. ZIP dosyasının tamamını bir klasöre çıkart.
2. `Syspect.exe` ile doğrudan aç veya masaüstü kısayolu için `SyspectSetup.exe` çalıştır.
3. **Create Account / Hesap oluştur** ekranından kendi hesabını oluştur.
4. **Deep System Report / Derin sistem analizi** ekranından tarama başlat.
5. **Scan history / Tarama geçmişi** üzerinden önceki raporları görüntüle, JSON indir veya sil.

Windows 10/11 x64, Windows PowerShell ve Edge kullanılır. Python/Node/Docker/veritabanı sunucusu kurman gerekmez: resmi Python 3.14.7 gömülü çalışma ortamı ve SQLite pakete dahildir. `SyspectSetup.exe` tek başına taşınmaz; ZIP içindeki klasörlerle birlikte çalışır. Çalıştırılabilir dosyalar kod imzalı değildir.

## Tamamlananlar

- Gerçek hesap oluşturma, giriş, çıkış, oturum süresi ve parola değiştirme.
- Kullanıcı adı, e-posta, görünen ad, açıklama ve kayıt tarihi.
- Hesaba bağlı cihazlar, cihaz adı düzenleme ve en son gerçek donanım bilgileri.
- Asenkron C motoru taraması; sayfa kapansa da sonuç arka planda kaydedilir.
- FPS, ısınma, BSOD ve input lag soru-cevap raporlarının otomatik kaydı; başarısız kayıtta tekrar deneme.
- Sayfalı geçmiş, rapor ayrıntısı, JSON dışa aktarma ve silme.
- Hatalı, zaman aşımına uğramış ve uygulama kapanırken yarım kalmış taramaların kaydı.
- Otomatik, sürümlü ve işlemsel SQL migration; tutarlı SQLite yedeği.
- Kullanıcı verisi içermeyen dağıtım. Varsayılan hesap veya demo seed yoktur.

## Depolama ve kapsam

Hesaplar ve raporlar **bu bilgisayarda** `%LOCALAPPDATA%\Syspect\data\syspect.sqlite3` dosyasına kaydedilir. Kurulum/güncelleme/kaldırma bu klasörü silmez. Bu sürüm bulut hesabı veya bilgisayarlar arası senkronizasyon sağlamaz. E-posta bir giriş kimliğidir; e-posta gönderimi, doğrulama veya e-postayla parola sıfırlama hizmeti yoktur.

SQLite mevcut çevrimdışı masaüstü mimarisine uygundur: ayrı hizmet veya uzak sunucu hesabı gerektirmez. Web arayüzü yalnızca yerel HTTP API ile konuşur. Veritabanı ve backend kaynakları HTTP üzerinden sunulmaz. Dark/light ve EN/TR tercihleri mevcut tarayıcı depolamasında kalır.

Parolalar rastgele salt ile PBKDF2-HMAC-SHA256 / 600.000 tur kullanılarak saklanır. Oturumlar 256 bit rastgele token kullanır; DB'de yalnızca token özeti bulunur. Cookie HttpOnly + SameSite=Strict'tir. API yalnızca loopback'te dinler; Host/Origin ve özel istek başlığı kontrolleri yapar. SQL sorguları parametrelidir ve her kişisel kayıt kullanıcı kimliğiyle filtrelenir. DB, Windows kullanıcısının özel uygulama verisi dizinindedir; ayrıca disk şifrelemesi uygulanmaz.

Soru-cevap raporları `questionnaire`, gerçek C motoru çıktıları `native` olarak işaretlenir. Motorun GPU veya dump alanlarını sağlayamaması uydurma verilerle doldurulmaz; `partial` kapsamı korunur.

## Yapılandırma

`.env.example` dosyasını isteğe bağlı olarak `.env` adına kopyala. Hiçbir değişiklik yapmadan varsayılanlarla çalışır. Ortam değişkenleri `.env` değerlerinden önceliklidir.

| Değişken | Varsayılan | Amaç |
|---|---|---|
| `SYSPECT_PORT` | `37841` | Doluysa sonraki 19 port denenir |
| `SYSPECT_DATA_DIR` | `%LOCALAPPDATA%\Syspect\data` | Yerel veri klasörü; OneDrive/ağ paylaşımı kullanma |
| `SYSPECT_SCAN_TIMEOUT_SECONDS` | `300` | Motor zaman aşımı |
| `SYSPECT_SESSION_HOURS` | `168` | Oturum ömrü; 1–720 saat |

Uygulamayı tamamen kapatmak için üst çubuktaki **Quit Syspect / Syspect’i kapat** düğmesini kullan. Güncelleme sırasında eski host çalışıyorsa kurulum bunu bildirir. İlgisiz bir servis portu kullanıyorsa launcher ona bağlanmaz. Veriler standart uygulama yolundan ayrı tutulur.

## Geliştirme ve kontroller

Paket kökünde:

```powershell
# Otomatik testler
.\runtime\python.exe -m unittest discover -s tests -v
# Migration (uygulama kapalıyken)
.\runtime\python.exe backend\server.py --migrate
# Tutarlı yedek: yeni bir hedef dosya adı seç
.\runtime\python.exe backend\server.py --backup C:\Yedekler\syspect-2026-09-28.sqlite3
# UI açmadan servis
.\runtime\python.exe backend\server.py --port 37841
# Windows başlatıcılarını yeniden derle
.\desktop\Build-NativeLauncher.ps1
.\installer\Build-Setup.ps1
```

Geri yükleme: Syspect’i tamamen kapat, mevcut veri klasörünü bir yere taşıyarak koru, boş veri klasörüne yedeği `syspect.sqlite3` adıyla kopyala. Eski `-wal` / `-shm` dosyalarını geri yüklenen DB ile karıştırma. Canlı veritabanını dosya kopyalama yoluyla yedeklemek yerine yukarıdaki backup komutunu kullan.

## Mimari ve kaynak

`HTML/CSS/JS → backend/server.py → backend/database.py → SQLite`

`backend/server.py → diagnosticengine.exe --json → doğrulanan JSON → scans.result_json`

- En güncel bulunan Desktop tabanı: `Syspect-v1.0.1-Desktop.zip`.
- C motoru kaynakları: bulunan v1.0.0 tam kaynak paketinden `engine-source/` altında korundu. Dağıtılan motor v1.0.1 paketindeki binary'dir; motor algoritması değiştirilmedi.
- Eski tek iş parçacıklı PowerShell HTTP sunucusu, paralel istekleri ve kalıcı API'yi destekleyen Python host ile değiştirildi. Windows launcher, mevcut arayüz ve gerçek C motoru korunur.
- `backend/migrations/001_initial.sql`: users, profiles, sessions, devices, scans, login_attempts, settings. `schema_migrations` migration sürümlerini takip eder.
- `tests/`: izolasyon, parola/oturum, CSRF/Host/Origin, migration, yedek, hata, kalıcılık, sayfalama ve tarama testleri. Testler geçici DB kullanır.
- `runtime/LICENSE.txt`: dağıtılan Python çalışma ortamının lisansı.

Teknoloji belgeleri: [SQLite kullanım alanları](https://www.sqlite.org/whentouse.html), [SQLite WAL](https://www.sqlite.org/wal.html), [resmi Python Windows paketleri](https://www.python.org/downloads/windows/).

## API

Tüm yollar `/api/v1` altındadır. Yazma istekleri `Content-Type: application/json` ve `X-Syspect-Client: desktop` ister. Oturum cookie ile taşınır. Kayıt, giriş, app ve health dışındaki API'ler giriş gerektirir.

| Yöntem ve yol | İşlev |
|---|---|
| `POST /auth/register` | `{email, username, password}` |
| `POST /auth/login` | `{email, password}` |
| `GET /auth/me` | Aktif kullanıcı |
| `POST /auth/logout` | `{}`; oturumu iptal et |
| `POST /auth/password` | `{current_password,new_password}`; tüm oturumları iptal et |
| `GET /profile` | Profil |
| `PATCH /profile` | `{display_name,bio}` |
| `GET /devices` | Kullanıcının cihazları |
| `PATCH /devices/{id}` | `{name}` |
| `POST /scans` | `{source:"native"}`; 202 ve scan.id döner |
| `POST /scans` | `{source:"questionnaire",scan_type,request_id,result:{answers,diagnoses,...}}`; UUID ile tekrar kaydı önler |
| `GET /scans?limit=20&offset=0` | Sayfalı geçmiş; en çok 100 kayıt |
| `GET /scans/{id}` | Durum/sonuç sorgulama |
| `GET /scans/{id}/export` | JSON indirme |
| `DELETE /scans/{id}` | `{}`; bitmiş raporu sil |
| `POST /scan` | `{}`; eski arayüz uyumluluğu için senkron, kalıcı motor taraması |
| `GET /health` | DB ve motor erişilebilirliği |
| `GET /app` | Sürüm ve host kimliği |
| `POST /app/exit` | `{}`; aktif tarama yoksa host'u kapat |
