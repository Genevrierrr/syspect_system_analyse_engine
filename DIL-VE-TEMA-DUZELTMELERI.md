# Dil ve koyu tema düzeltmeleri — 19 Ekim 2026

## Yapılan değişiklikler

- Ana sayfa, indirme, giriş/kayıt, kontrol paneli, profil, geçmiş, derin analiz ve teşhis sayfalarının Türkçe/İngilizce metinleri kontrol edildi.
- Eksik teşhis açıklamaları, kanıtlar, önerilen adımlar, sayfa başlıkları, hesap hataları ve tarama durumları çevrildi.
- Dil geçişinde kaynak metinlerin korunması sağlandı. Sonradan değişen metin düğümleri, araç ipuçları ve erişilebilirlik etiketleri artık güncelleniyor.
- Kullanıcının form girdileri ve kimlik bilgileri çeviri dışında tutuldu. Tarihler seçilen dile göre biçimlendiriliyor.
- Ham motor raporu, hata kodları, dosya yolları ve teknik veriler özgün biçiminde korunuyor. Ham rapor bölümünde bunu belirten çevrilebilir bir etiket bulunuyor.
- Koyu temada seçenekler, sonuç/kanıt kutuları, bağlantı uyarıları, zaman çizelgesi, ikincil düğmeler ve form alanları ortak koyu yeşil palete uyarlandı.
- Ana metin rengi açık tona getirildi; açık yeşil düğmelerde koyu yazı kullanıldı. Uyarı, hata, odak ve seçili durumların renkleri düzenlendi.

## Doğrulama

- 694 sözlük, hesap mesajı, dinamik metin, öznitelik ve tekrarlanan dil geçişi kontrolü başarılı.
- Tüm HTML dosyalarındaki betikler ve ortak JavaScript dosyaları sözdizimi kontrolünden geçti.
- Statik arayüz metinleri ve teşhis veri modellerindeki soru, seçenek, açıklama, kanıt ve önerilerin çeviri kapsamı kontrol edildi. Marka adları, teknik tanımlayıcılar ve örnek kullanıcı adı çevrilmedi.
- 11 temel koyu tema metin/zemin çiftinin hesaplanan kontrastı 6,35:1 ile 14,65:1 arasında. Bu hesaplama tüm olası ekran durumları için erişilebilirlik sertifikası değildir.
- Tarayıcı önizlemesi ortamın erişim kısıtları nedeniyle tamamlanamadı; gerçek tarayıcıda görsel doğrulama yapılmadı. Yerel teşhis motoru ve kurulum dosyaları çalıştırılmadı.
- SHA256-MANIFEST.json güncel paket dosyalarına göre yenilendi.

## Kullanım

ZIP dosyasını ayrı bir klasöre çıkarın ve mevcut proje başlatma yönteminizi kullanın. Web dosyalarını mevcut yayına kopyalıyorsanız i18n.js, account.js, theme.css ve Syspect.html dosyalarını birlikte güncelleyin; tarayıcıda Ctrl+F5 ile yenileyin.
