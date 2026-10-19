# Ngrok ile deneme

1. ZIP'i tamamen çıkart.
2. `Start-Ngrok-Test.cmd` dosyasına çift tıkla.
3. Ngrok panelindeki [Your Authtoken](https://dashboard.ngrok.com/get-started/your-authtoken) değerini açılan pencereye yapıştır. Girdi ekranda görünmez; sohbetle paylaşman gerekmez. Token diske kaydedilmez.
4. Pencerede gösterilen adresi aç: **https://prognosis-scarily-qualify.ngrok-free.dev**.
5. Tarayıcının ilk parola sorusunda kullanıcı adı **syspect**, parola ise başlatıcı penceresindeki **Browser access password** değeridir.
6. Ardından Syspect ekranında bir test hesabı oluştur; giriş, profil ve tarama geçmişini dene.

Ngrok bağlanırken birkaç saniye beklemek gerekebilir. Token/adres hesabınla eşleşmiyorsa ngrok hatası pencerede görünür. Testi durdurmak için başlatıcıda **Ctrl+C** kullan. Tünel ve ona ait Syspect sunucusu kapanır.

Telefon veya başka bilgisayar bu bağlantıyı açabilir. **Motor taraması, bağlantıyı açan telefonda değil bu Windows bilgisayarında yapılır.** Ngrok yeni bir bulut veritabanı oluşturmaz.

Bu test `%LOCALAPPDATA%\Syspect\ngrok-test` altında ayrı ve kalıcı bir SQLite veritabanı kullanır; normal kurulumdaki hesaplar/raporlar paylaşılmaz. Her başlatmada dış erişim parolası değişir. Gerçek motor taraması yaparsan rapor ngrok üzerinden eriştiğin tarayıcıya iletilir.

Ngrok agent 3.39.11 resmi kaynaktan indirilmiştir; Windows dijital imzası doğrulanmıştır. Tünelde trafik gövdesi inceleme kapalıdır (`--inspect=false`). Kimlik doğrulama [ngrok Basic Auth Traffic Policy](https://ngrok.com/docs/gateway/traffic-policy/actions/basic-auth) ile uygulanır.

Backend yalnızca bu **tam HTTPS adresini** ek Host/Origin olarak kabul eder; diğer adresler reddedilir. Public origin üzerinden oluşturulan oturum cookie'leri Secure'dür. CSRF ve kullanıcı izolasyonu kontrolleri korunur. Yerel uygulama normal başlatıldığında localhost-only davranışı değişmez.

Tünel bağlantısını açmak için hesap sahibinin authtoken girmesi gerekir. Hazırlık sırasında canlı hesap kimlik bilgileri kullanılmadığından internet üzerinden uçtan uca bağlantı henüz doğrulanmamıştır.
