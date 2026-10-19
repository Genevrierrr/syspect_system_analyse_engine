# Doğrulama — 28 Eylül 2026

- Windows üzerinde **12/12** otomatik entegrasyon testi geçti.
- 8 JavaScript dosyası/satır içi script sözdizimi kontrolünden geçti.
- Tarayıcıda gerçek kayıt, oturum açma, profil düzenleme ve FPS soru-cevap raporu oluşturma akışları çalıştırıldı.
- Gerçek `diagnosticengine.exe --json` taraması API üzerinden çalıştırıldı. Yaklaşık 94 KB UTF-8 metin raporu, donanım/olay bilgileri ve `partial` kapsam bilgisi SQLite'a kaydedildi.
- Motor raporu ve soru-cevap raporu tarama geçmişinde ayrı kayıtlar olarak görüntülendi.
- Host kapatılıp yeniden başlatıldı; hesap, oturum ve raporlar korundu.
- Dolu 37921 portu ile launcher 37922'ye geçti. İkinci açılış aynı doğru host'u yeniden kullandı.
- Syspect.exe ve SyspectSetup.exe Windows'un .NET Framework derleyicisiyle yeniden derlendi. Launcher'ın sunucuyu başlatması gerçek Windows ortamında doğrulandı.
- Migration tekrar çalıştırılabilirliği, SQLite bütünlüğü ve tutarlı yedek doğrulandı.
- Hesaplar arası erişim, oturum iptali/süre sonu, parola değiştirme, SQL parametreleme, Host/Origin/CSRF ve özel dosyaların HTTP'den erişilememesi test edildi.

Kurulum sihirbazı kullanıcının mevcut kurulumuna uygulanmadı; dağıtım taşınabilir klasörde test edildi. Mevcut C motoru GPU alanını sağlamıyor ve bu bilgisayarda dump analizini tamamlayamadı. Bu eksikler `partial` olarak korunur; veritabanı entegrasyonu bu motor sınırlamalarını değiştirmez.

Test hesapları ve gerçek bilgisayara ait raporlar teslim ZIP'ine dahil edilmemiştir. İlk kullanımda veritabanı ve hesaplar sıfırdan oluşturulur.
