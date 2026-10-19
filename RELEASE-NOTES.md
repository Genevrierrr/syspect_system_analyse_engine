# 1.1.0 — Yerel hesaplar ve kalıcı raporlar

v1.0.1 Desktop üzerine gerçek SQLite veritabanı ve API entegre edildi. Giriş/kayıt formları, profil, cihazlar ve tarama geçmişi backend'e bağlandı. Yerel hesaplar bilgisayarda tutulur; bulut senkronizasyonu yoktur.

Native C motoru ve mevcut soru-cevap modülleri sonuçları kullanıcıya özel kaydeder. Motor sonuçları asenkron çalışır; başarısız/yarım kalmış işler geçmişte görünür. Ana paneldeki sabit donanım örnekleri kaldırıldı. Kalıcı migration, yedekleme, env ayarları, paketlenmiş çalışma ortamı ve entegrasyon testleri eklendi.

Varsayılan hesap, demo raporu veya kullanıcının gerçek test verileri dağıtımda bulunmaz.
