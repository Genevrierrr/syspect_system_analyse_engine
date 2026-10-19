(function(){
  const KEY='syspect-language';
  const TR={
    'Sign In':'Giriş Yap','Sign In to access the diagnostic engine':'Teşhis motoruna erişmek için giriş yapın','Sign in to access the diagnostic engine':'Teşhis motoruna erişmek için giriş yapın',
    'Create Account':'Hesap Oluştur','Welcome Back.':'Tekrar Hoş Geldiniz.','Email Address':'E-posta Adresi','Password':'Şifre','Confirm Password':'Şifreyi Onayla','Username':'Kullanıcı Adı',
    "Don't have an account?":'Hesabınız yok mu?','Already have an account?':'Zaten hesabınız var mı?','← Back to Home':'← Ana Sayfaya Dön','← Back to Workspace':'← Çalışma Alanına Dön',
    'WORKSPACE ACCESS':'ÇALIŞMA ALANI ERİŞİMİ','NEW WORKSPACE PROFILE':'YENİ ÇALIŞMA ALANI PROFİLİ','Get personalized troubleshooting reports':'Kişiselleştirilmiş sorun giderme raporları alın',
    'ENGINE / v3.0':'MOTOR / v3.0','SYSTEM DIAGNOSTIC WORKSPACE':'SİSTEM TEŞHİS ÇALIŞMA ALANI','Stop Guessing.':'Tahmin Etmeyi Bırak.','Start Fixing.':'Çözmeye Başla.',
    'Run System Scan':'Sistem Taramasını Başlat','DIAGNOSTIC ARCHITECTURE':'TEŞHİS MİMARİSİ','Evidence-Based Telemetry & Triage':'Kanıta Dayalı Telemetri ve Önceliklendirme',
    'Windows · x64 Kernel & Hardware Correlation':'Windows · x64 Çekirdek ve Donanım Korelasyonu','Diagnostic Engine v3.0':'Teşhis Motoru v3.0',
    'FPS & Thermal':'FPS ve Termal','Throttling':'Darboğaz','Scan':'Tarama','GPU/CPU load & storage IOPS':'GPU/CPU yükü ve depolama IOPS',
    'Crash Dump':'Çökme Dökümü','Parser':'Ayrıştırıcı','Minidump & BugCheck codes':'Minidump ve BugCheck kodları','Boot & Latency':'Açılış ve Gecikme','Startup':'Başlangıç','Audit':'Denetim',
    'Fast Startup & disk health':'Hızlı Başlatma ve disk sağlığı','Kernel Workspace':'Çekirdek Çalışma Alanı','Deep':'Derin','Analysis':'Analiz','192 devices · PnP timeline':'192 cihaz · PnP zaman çizelgesi',
    '4 diagnostic modules active':'4 teşhis modülü aktif','Rule-based correlation matrix':'Kural tabanlı korelasyon matrisi','From data to evidence. From evidence to diagnosis.':'Veriden kanıta. Kanıttan teşhise.',
    'Diagnostic Modules':'Teşhis Modülleri','Select a workspace module to begin triage':'Önceliklendirmeye başlamak için bir çalışma alanı modülü seçin','Performance':'Performans','Active Module':'Aktif Modül',
    'FPS Drops & Stutters':'FPS Düşüşleri ve Takılmalar','Thermal & Storage IOPS':'Termal ve Depolama IOPS','Launch Module':'Modülü Başlat','New: Minidump Analysis':'Yeni: Minidump Analizi',
    'Blue Screen (BSOD)':'Mavi Ekran (BSOD)','BugCheck & Kernel Stack':'BugCheck ve Çekirdek Yığını','System Latency':'Sistem Gecikmesi','Slow Boot & System Lag':'Yavaş Açılış ve Sistem Gecikmesi',
    'Boot Pipeline & SMART':'Açılış Süreci ve SMART','Kernel Telemetry':'Çekirdek Telemetrisi','Workspace Preview':'Çalışma Alanı Önizlemesi','Deep System Analysis':'Derin Sistem Analizi',
    'Open Report View':'Rapor Görünümünü Aç','Professional PC diagnostics and performance optimization solutions.':'Profesyonel PC teşhis ve performans optimizasyon çözümleri.','Contact':'İletişim','Address':'Adres',
    'DIAGNOSTIC CONTROL CENTER':'TEŞHİS KONTROL MERKEZİ','Available Diagnostics':'Kullanılabilir Teşhisler','Deep System Report':'Derin Sistem Raporu','ACTIVE WORKSPACE':'AKTİF ÇALIŞMA ALANI',
    'Local System Profile':'Yerel Sistem Profili','Telemetry Ready':'Telemetri Hazır','Processor':'İşlemci','logical processors':'mantıksal işlemci','Memory':'Bellek','Available physical capacity':'Kullanılabilir fiziksel kapasite',
    'Graphics':'Ekran Kartı','Operating System':'İşletim Sistemi','Build number':'Derleme numarası','devices detected':'cihaz algılandı','problematic devices':'problemli cihaz','Interactive diagnostic modules ready':'Etkileşimli teşhis modülleri hazır',
    'Interactive Modules':'Etkileşimli Modüller','Rule-based triage & telemetry correlation':'Kural tabanlı önceliklendirme ve telemetri korelasyonu','Ready to Scan':'Taramaya Hazır','FPS Drops':'FPS Düşüşleri',
    'Identify thermal throttling, RAM limitations, and storage bottlenecks in gaming.':'Oyunlarda termal kısıtlama, RAM sınırlamaları ve depolama darboğazlarını belirleyin.','Start Diagnosis':'Teşhisi Başlat',
    'Thermals':'Termal','Overheating':'Aşırı Isınma','Monitor core temperatures and pinpoint heavy hardware stress parameters.':'Çekirdek sıcaklıklarını izleyin ve ağır donanım yükü parametrelerini belirleyin.',
    'Kernel Crash':'Çekirdek Çökmesi','BSOD':'BSOD','Analyze crash codes, dump files, and system behavior to find faulty drivers.':'Hatalı sürücüleri bulmak için çökme kodlarını, döküm dosyalarını ve sistem davranışını analiz edin.',
    'Boot & Lag':'Açılış ve Gecikme','Slow Startup':'Yavaş Başlangıç','Detect bloated startup registries, background processes, and drive degradation.':'Şişmiş başlangıç kayıtlarını, arka plan işlemlerini ve disk bozulmasını tespit edin.',
    'Dashboard view':'Kontrol Paneli görünümü','Logout':'Çıkış','Welcome,':'Hoş geldin,','User':'Kullanıcı','Profile':'Profil','ACCOUNT PROFILE':'HESAP PROFİLİ','ACCOUNT DETAILS':'HESAP AYRINTILARI','LOCAL SETTINGS':'YEREL AYARLAR','Your Profile.':'Profiliniz.','Manage your Syspect workspace identity and local preferences.':'Syspect çalışma alanı kimliğinizi ve yerel tercihlerinizi yönetin.','Account Overview':'Hesap Özeti','Account details':'Hesap ayrıntıları','Local workspace profile':'Yerel çalışma alanı profili','Account Type':'Hesap Türü','Local Profile':'Yerel Profil','Workspace':'Çalışma Alanı','Preferences':'Tercihler','Stored in this browser':'Bu tarayıcıda saklanır','Language':'Dil','Theme':'Tema','Controlled from the header':'Üst çubuktan yönetilir','Back to Dashboard':'Kontrol Paneline Dön','Not set':'Ayarlanmadı','Profile data is currently local to this demo workspace.':'Profil verileri şu anda bu demo çalışma alanında yerel olarak tutulur.','Syspect — Profile':'Syspect — Profil',
    'Open Syspect':'Syspect’i Aç','Syspect is already installed on this PC.':'Syspect bu bilgisayarda zaten yüklü.','PC DIAGNOSTIC ENGINE':'PC TEŞHİS MOTORU','SYSTEM WORKSPACE':'SİSTEM ÇALIŞMA ALANI','Device Information':'Cihaz Bilgileri','Your computer':'Bilgisayarın','Not scanned yet':'Henüz taranmadı','Waiting for scan':'Tarama bekleniyor',
    'Start Scan':'Taramayı Başlat','Checking connection…':'Bağlantı kontrol ediliyor…','Looking for local scan service.':'Yerel tarama servisi aranıyor.','Check Connection':'Bağlantıyı Kontrol Et','JavaScript must be enabled in your browser for scanning.':'Tarama için tarayıcınızda JavaScript açık olmalı.',
    'Total physical capacity':'Toplam fiziksel kapasite','Windows build number':'Windows derleme numarası','Devices:':'Cihazlar:','Problematic devices:':'Problemli cihazlar:','Overview':'Genel bakış','Timeline':'Zaman çizelgesi','Waiting for results':'Sonuç bekleniyor','Last crash':'Son çökme',
    'Module detection alone is not a definitive root cause.':'Modül tespiti tek başına kesin kök neden değildir.','Drivers and findings':'Sürücüler ve bulgular','Driver-backed devices counted by engine':'Motorun saydığı sürücülü cihazlar','Event history':'Olay geçmişi','EVENTS AROUND CRASH':'ÇÖKME ÇEVRESİNDEKİ OLAYLAR',
    'Detailed engine report':'Ayrıntılı motor raporu','Collected evidence and explanations for unreadable data.':'Toplanan kanıtlar ve okunamayan verilere ilişkin açıklamalar.','Download Result':'Sonucu İndir','Show Text Report':'Metin Raporunu Göster','Local scan / v0.3':'Yerel tarama / v0.3',
    'Back':'Geri','Continue':'Devam','Generate Report':'Rapor Oluştur','Analyze Engine':'Motoru Analiz Et','Run New Scan':'Yeni Tarama Başlat','Process Another Log':'Başka Bir Günlüğü İşle','Process New Boot Assessment':'Yeni Açılış Değerlendirmesi Başlat',
    'Step':'Adım','Severity:':'Önem Derecesi:','System Metrics':'Sistem Metrikleri','Diagnostic Evidence':'Teşhis Kanıtları','Recommended Action Plan':'Önerilen Eylem Planı','Targeted Remediation Plan':'Hedefli Düzeltme Planı','Targeted Remediation Steps':'Hedefli Düzeltme Adımları','Correlated Evidence Data':'İlişkili Kanıt Verileri',
    'Minor':'Düşük','Moderate':'Orta','Critical':'Kritik','Case #':'Vaka #','Minidump Case #':'Minidump Vakası #','Latency Report #':'Gecikme Raporu #',
    'Blue Screen (BSOD) Analysis':'Mavi Ekran (BSOD) Analizi','Error Identification':'Hata Tanımlama','What BSOD code did you get?':'Hangi BSOD kodunu aldınız?','Select the category that matches your stop code, or upload a minidump file.':'Durma kodunuzla eşleşen kategoriyi seçin veya bir minidump dosyası yükleyin.',
    'RAM / Memory Errors':'RAM / Bellek Hataları','Driver / System Faults':'Sürücü / Sistem Hataları','GPU / Video Errors':'GPU / Video Hataları','CPU / Motherboard':'CPU / Anakart','Disk / Storage':'Disk / Depolama','Unknown / I don’t remember':'Bilinmiyor / Hatırlamıyorum','I missed the code or it rebooted too fast.':'Kodu göremedim veya sistem çok hızlı yeniden başladı.',
    'Memory Configuration':'Bellek Yapılandırması','RAM Diagnostics: Did you tweak your memory?':'RAM Teşhisi: Bellek ayarlarını değiştirdiniz mi?','Answer based on your current BIOS and hardware setup.':'Mevcut BIOS ve donanım kurulumunuza göre yanıtlayın.',
    'XMP/EXPO is Enabled or RAM is Overclocked':'XMP/EXPO Etkin veya RAM Hız Aşırtmalı','Running above stock speeds':'Standart hızların üzerinde çalışıyor','Recently installed new RAM sticks':'Yakın zamanda yeni RAM takıldı','Hardware was physically changed':'Donanım fiziksel olarak değiştirildi','Stock settings, nothing changed':'Standart ayarlar, değişiklik yok','Everything is factory default':'Her şey fabrika ayarlarında',
    'Testing':'Test','Have you run Memtest86 or Windows Memory Diagnostic?':'Memtest86 veya Windows Bellek Tanılama çalıştırdınız mı?','Physical error checking is crucial for memory management BSODs.':'Bellek yönetimi BSOD’larında fiziksel hata kontrolü çok önemlidir.','Yes, and it found errors':'Evet, hatalar buldu','Hardware fault confirmed':'Donanım arızası doğrulandı','No, haven’t tested yet':'Hayır, henüz test etmedim','Status unknown':'Durum bilinmiyor',
    'Video Hardware':'Video Donanımı','GPU Diagnostics: What is your setup?':'GPU Teşhisi: Kurulumunuz nedir?','VIDEO_TDR errors are handled differently by manufacturers.':'VIDEO_TDR hataları üreticilere göre farklı ele alınır.','NVIDIA GPU':'NVIDIA GPU','AMD GPU':'AMD GPU','GeForce series':'GeForce serisi','Radeon series':'Radeon serisi',
    'Context':'Bağlam','When does the VIDEO_TDR_FAILURE happen?':'VIDEO_TDR_FAILURE ne zaman oluşuyor?','Is it load-based or driver-based?':'Yüke mi yoksa sürücüye mi bağlı?','Only during heavy gaming / rendering':'Yalnızca ağır oyun / render sırasında','Under stress':'Yük altında','Started right after a driver update':'Sürücü güncellemesinden hemen sonra başladı','Software conflict':'Yazılım çakışması',
    'Core Settings':'Çekirdek Ayarları','CPU Diagnostics: Are you overclocking?':'CPU Teşhisi: Hız aşırtma yapıyor musunuz?','WHEA errors are fatal hardware architecture flags.':'WHEA hataları kritik donanım mimarisi işaretleridir.','Yes, CPU OC or Undervolting is active':'Evet, CPU OC veya Undervolt aktif','Voltage curves altered':'Voltaj eğrileri değiştirildi','No, completely stock':'Hayır, tamamen standart','Factory defaults':'Fabrika ayarları',
    'BIOS State':'BIOS Durumu','Is your Motherboard BIOS up to date?':'Anakart BIOS’unuz güncel mi?','Crucial for system stability, especially on Ryzen platforms.':'Özellikle Ryzen platformlarında sistem kararlılığı için önemlidir.','No / I don’t know':'Hayır / Bilmiyorum','Likely outdated':'Muhtemelen eski','Yes, it is the latest version':'Evet, en son sürüm','Recently flashed':'Yakın zamanda güncellendi',
    'Unstable RAM Configuration':'Kararsız RAM Yapılandırması','GPU Driver / Hardware Timeout':'GPU Sürücü / Donanım Zaman Aşımı','CPU/Motherboard Hardware Fault':'CPU/Anakart Donanım Arızası','Critical Driver Conflict':'Kritik Sürücü Çakışması','Storage Drive Corruption':'Depolama Sürücüsü Bozulması',
    'FPS Drops & Stuttering':'FPS Düşüşleri ve Takılma','Symptom Analysis':'Belirti Analizi','When do the FPS drops usually happen?':'FPS düşüşleri genellikle ne zaman oluyor?','Timing helps us understand if it is a heat, memory, or hardware capability issue.':'Zamanlama, sorunun ısı, bellek veya donanım kapasitesiyle ilgili olup olmadığını anlamamıza yardımcı olur.',
    'Right from the beginning of the game':'Oyunun en başından itibaren','Low FPS as soon as you load in':'Oyuna girer girmez düşük FPS','After 15-20 minutes of playing':'15-20 dakika oynadıktan sonra','Starts fine, gets worse over time':'İyi başlıyor, zamanla kötüleşiyor','Randomly during fast action or explosions':'Hızlı aksiyon veya patlamalarda rastgele','Sudden stutters and freezes':'Ani takılmalar ve donmalar',
    'Hardware Profile':'Donanım Profili','What type of machine are you playing on?':'Hangi tür cihazda oynuyorsunuz?','Form factor greatly influences cooling capacity and power delivery.':'Cihaz formu soğutma kapasitesini ve güç iletimini büyük ölçüde etkiler.','Desktop PC':'Masaüstü PC','Standard tower case':'Standart kule kasa','Gaming Laptop':'Oyuncu Dizüstü','Portable gaming machine':'Taşınabilir oyuncu cihazı','Device':'Cihaz',
    'System Resources':'Sistem Kaynakları','Are your games installed on an SSD or HDD?':'Oyunlarınız SSD’ye mi HDD’ye mi kurulu?','Drive speed is the #1 cause of map-loading stutters.':'Disk hızı, harita yükleme takılmalarının başlıca nedenidir.','HDD (Mechanical Drive)':'HDD (Mekanik Disk)','Older, slower spinning disks':'Eski, daha yavaş dönen diskler','SSD or NVMe':'SSD veya NVMe','Fast solid-state storage':'Hızlı katı hâl depolama','Storage':'Depolama',
    'Thermal & Overheating Analysis':'Termal ve Aşırı Isınma Analizi','Thermal Behavior':'Termal Davranış','When do your temperatures or fan speeds spike?':'Sıcaklıklarınız veya fan hızlarınız ne zaman yükseliyor?','Helps isolate idle cooling failures from sustained load thermal throttling.':'Boştaki soğutma sorunlarını sürekli yük altındaki termal kısıtlamadan ayırmaya yardımcı olur.',
    'Even while idle on the Windows desktop':'Windows masaüstünde boştayken bile','High temps with no games open':'Oyun açık değilken yüksek sıcaklık','After 15-20 minutes of gaming or rendering':'15-20 dakika oyun veya render sonrasında','Heat builds up and causes FPS drops':'Isı birikir ve FPS düşüşlerine neden olur','Only in specific unoptimized titles':'Yalnızca belirli optimize edilmemiş oyunlarda','High GPU/CPU utilization spikes':'Yüksek GPU/CPU kullanım sıçramaları',
    'What type of chassis are you using?':'Hangi tür kasa kullanıyorsunuz?','Airflow geometry dictates heat dissipation efficiency.':'Hava akışı düzeni, ısı dağıtım verimliliğini belirler.','Compact shared heatpipe cooling':'Kompakt ortak ısı borulu soğutma','Maintenance History':'Bakım Geçmişi','When was the last time fans/vents were cleaned or thermal paste replaced?':'Fanlar/hava kanalları en son ne zaman temizlendi veya termal macun değiştirildi?','Dust buildup and dried thermal paste are the primary causes of overheating.':'Toz birikimi ve kurumuş termal macun, aşırı ısınmanın başlıca nedenleridir.','Over 1 year ago / Never cleaned':'1 yıldan uzun süre önce / Hiç temizlenmedi','Likely dust accumulation or dry paste':'Muhtemel toz birikimi veya kurumuş macun','Maintenance':'Bakım','Cleaned recently (within 6 months)':'Yakın zamanda temizlendi (6 ay içinde)','Heatsinks and fans are clear':'Soğutucular ve fanlar temiz',
    'CPU/GPU Thermal Throttling':'CPU/GPU Termal Kısıtlama','Storage Drive Bottleneck':'Depolama Darboğazı','Hardware Capability Limit':'Donanım Kapasite Sınırı',
    'Boot Duration':'Açılış Süresi','How long does it take for the Windows desktop to appear?':'Windows masaüstünün görünmesi ne kadar sürüyor?','Timing from cold boot helps isolate mechanical delays from software layer bloat.':'Soğuk açılış süresi, mekanik gecikmeleri yazılım katmanı şişkinliğinden ayırmaya yardımcı olur.','Under 15 seconds':'15 saniyenin altında','Excellent speed. Likely healthy NVMe drive configuration.':'Mükemmel hız. NVMe yapılandırması muhtemelen sağlıklı.','15 to 45 seconds':'15 ila 45 saniye','Moderate delay. Usually caused by background startup apps.':'Orta düzey gecikme. Genellikle arka plan başlangıç uygulamalarından kaynaklanır.','45 seconds to 2 minutes':'45 saniye ila 2 dakika','Noticeable lag. Indicates a bottleneck during system file loading.':'Belirgin gecikme. Sistem dosyaları yüklenirken bir darboğaza işaret eder.','More than 2 minutes':'2 dakikadan fazla','Critical slowdown. Highly indicative of mechanical drive bottlenecks or failing health.':'Kritik yavaşlama. Mekanik disk darboğazı veya sağlık sorunu olasılığı yüksektir.',
    'Post-Desktop Usability':'Masaüstü Sonrası Kullanılabilirlik','Can you use the PC immediately after the desktop icons load?':'Masaüstü simgeleri yüklendikten hemen sonra PC’yi kullanabiliyor musunuz?','Identifies if background services are choking your system resources at launch.':'Arka plan hizmetlerinin başlangıçta sistem kaynaklarını tüketip tüketmediğini belirler.','Yes, it responds instantly':'Evet, anında yanıt veriyor','Background load is clear.':'Arka plan yükü temiz.','No, it completely freezes/stutters for a few minutes':'Hayır, birkaç dakika tamamen donuyor/takılıyor','System locks up during background initialization.':'Arka plan başlatma sırasında sistem kilitleniyor.','No, the fans ramp up loudly with immediate high CPU use':'Hayır, fanlar hemen hızlanıyor ve CPU kullanımı yükseliyor','Cries for processing power right at boot.':'Açılışta yoğun işlem gücü ihtiyacına işaret ediyor.',
    'Storage Architecture':'Depolama Mimarisi','What type of drive is Windows installed on?':'Windows hangi tür diske kurulu?','Mechanical drives lack the IOPS capacity required for modern Windows services.':'Mekanik diskler modern Windows hizmetlerinin gerektirdiği IOPS kapasitesinden yoksundur.','SSD (NVMe or SATA)':'SSD (NVMe veya SATA)','Solid state storage technology.':'Katı hâl depolama teknolojisi.','HDD (Mechanical spinning disk)':'HDD (Mekanik döner disk)','Traditional magnetic drive plates.':'Geleneksel manyetik disk plakaları.',
    'Behavioral History':'Davranış Geçmişi','When did this lag/slowdown sequence begin?':'Bu gecikme/yavaşlama ne zaman başladı?','Helps correlate the system decay with a specific trigger event.':'Sistem bozulmasını belirli bir tetikleyici olayla ilişkilendirmeye yardımcı olur.','Immediately after a Windows or Driver update':'Windows veya sürücü güncellemesinden hemen sonra','Corrupt installation layer.':'Bozuk kurulum katmanı.','After installing cracked or unverified software':'Korsan veya doğrulanmamış yazılım kurduktan sonra','Potential payload activation.':'Olası zararlı yük aktivasyonu.','It has been getting gradually worse over several months':'Birkaç aydır giderek kötüleşiyor','Natural decay or clutter build-up.':'Doğal bozulma veya birikmiş gereksiz yük.',
    'Power Management Cycle':'Güç Yönetimi Döngüsü','No, both cycles are consistently sluggish':'Hayır, iki döngü de sürekli yavaş','Persistent state problem.':'Kalıcı durum sorunu.','Memory Capacity':'Bellek Kapasitesi','What is the total physical memory (RAM) capacity of your system?':'Sisteminizin toplam fiziksel bellek (RAM) kapasitesi nedir?','Low memory forcing high reliance on virtual swap file storage page files.':'Düşük bellek, sanal takas/pagefile kullanımını artırır.','8 GB or less':'8 GB veya daha az','Critical for modern multi-threaded operating tasks.':'Modern çok iş parçacıklı görevler için kritik derecede düşük.','16 GB':'16 GB','Optimal base standard.':'İdeal temel standart.','32 GB or higher':'32 GB veya daha fazla','Abundant memory environment.':'Geniş bellek kapasitesi.',
    'Telemetry Data Input':'Telemetri Veri Girişi','Advanced Diagnostics (Optional Inputs)':'Gelişmiş Teşhis (İsteğe Bağlı Girdiler)','Provide values from Task Manager or CrystalDiskInfo for a high-precision report.':'Yüksek hassasiyetli rapor için Görev Yöneticisi veya CrystalDiskInfo değerlerini girin.','Process inputs and compile telemetry metrics':'Girdileri işle ve telemetri metriklerini derle','Finalizes the system lag calculation matrix.':'Sistem gecikmesi hesaplama matrisini tamamlar.',
    'Excessive Startup Apps & Bloatware':'Aşırı Başlangıç Uygulaması ve Gereksiz Yazılım','Background Malware or Crypto-Miner Threat':'Arka Plan Zararlı Yazılımı veya Kripto Madencisi Tehdidi','Storage Drive Sector Degradation or HDD Bottleneck':'Depolama Sektör Bozulması veya HDD Darboğazı','Corrupted Windows System Files':'Bozuk Windows Sistem Dosyaları','Sentry S0/Kernel Driver Conflict':'Sentry S0/Çekirdek Sürücü Çakışması','Low Memory Capacity Paging Trap':'Düşük Bellek Kapasitesi Sayfalama Tuzağı','Windows Hybrid Fast Startup Bug':'Windows Hibrit Hızlı Başlatma Hatası',
    '📁 Upload Minidump (.dmp) File (Experimental)':'📁 Minidump (.dmp) Dosyası Yükle (Deneysel)','Choose File (C:\\Windows\\Minidump)':'Dosya Seç (C:\\Windows\\Minidump)','Startup Apps Count:':'Başlangıç Uygulamaları Sayısı:','Under 5 Enabled Apps':'5’ten Az Etkin Uygulama','5 to 10 Enabled Apps':'5 ila 10 Etkin Uygulama','More than 10 Enabled Apps':'10’dan Fazla Etkin Uygulama','CrystalDiskInfo Drive Health Status:':'CrystalDiskInfo Disk Sağlığı Durumu:','Good (100% / No warnings)':'İyi (%100 / Uyarı yok)','Caution (Yellow warning sectors detected)':'Dikkat (Sarı uyarı sektörleri algılandı)','Bad (Red failing status)':'Kötü (Kırmızı arıza durumu)','I don’t have CrystalDiskInfo installed':'CrystalDiskInfo kurulu değil'
  };

  // Turkish strings already present in the original UI; these must become English by default.
  const ORIGINAL_TR_TO_EN={
    'Syspect — Derin sistem analizi':'Syspect — Deep System Analysis','SİSTEM ÇALIŞMA ALANI':'SYSTEM WORKSPACE','Derin sistem analizi':'Deep System Analysis','Bilgisayarının durumu, kanıtlarıyla birlikte.':'Your computer status, backed by evidence.','Taramayı başlat':'Start Scan','Bağlantı kontrol ediliyor…':'Checking connection…','Yerel tarama servisi aranıyor.':'Looking for local scan service.','Bağlantıyı kontrol et':'Check Connection','Tarama için tarayıcınızda JavaScript açık olmalı.':'JavaScript must be enabled in your browser for scanning.','CİHAZ BİLGİLERİ':'DEVICE INFORMATION','Senin bilgisayarın':'Your computer','Henüz taranmadı':'Not scanned yet','Henüz taranmadı.':'Not scanned yet.','Tarama bekleniyor':'Waiting for scan','İşlemci':'Processor','Bellek':'Memory','Toplam fiziksel kapasite':'Total physical capacity','Ekran kartı':'Graphics','İşletim sistemi':'Operating System','Windows derleme numarası':'Windows build number','Cihazlar:':'Devices:','Problemli cihazlar:':'Problematic devices:','Genel bakış':'Overview','Zaman çizelgesi':'Timeline','Sonuç bekleniyor':'Waiting for results','Son çökme':'Last crash','Modül tespiti tek başına kesin kök neden değildir.':'Module detection alone is not a definitive root cause.','Sürücüler ve bulgular':'Drivers and findings','Motorun saydığı sürücülü cihazlar':'Driver-backed devices counted by engine','Olay geçmişi':'Event history','ÇÖKME ÇEVRESİNDEKİ OLAYLAR':'EVENTS AROUND CRASH','Ayrıntılı motor raporu':'Detailed engine report','Toplanan kanıtlar ve okunamayan verilere ilişkin açıklamalar.':'Collected evidence and explanations for unreadable data.','Sonucu indir':'Download Result','Metin raporunu göster':'Show Text Report','Veriden kanıta. Kanıttan teşhise.':'From data to evidence. From evidence to diagnosis.','Yerel tarama / v0.3':'Local scan / v0.3','Dashboard görünümü':'Dashboard view','Analiz görünümü':'Analysis view','Gerçek motor verisi':'Live engine data','Gerçek motor sonuçları aşağıda. Alınamayan veriler ayrıca belirtilmiştir.':'Live engine results are shown below. Unavailable data is listed separately.','Kısmi sonuç · eksik alanlar belirtilmiştir':'Partial result · missing fields are indicated','Bulgular motor tarafından raporlandı.':'Findings were reported by the engine.','Motor bu taramada bulgu raporlamadı.':'The engine reported no findings in this scan.','Ayrı bulgu verisi alınamadı. Motorun bulguları ve kapsamı aşağıdaki metin raporunda.':'No separate findings data was available. Engine findings and scope are included in the text report below.','Bu sürümde toplanmıyor':'Not collected in this version','Dump analizi tamamlandı':'Dump analysis completed','Dump analizi kısmen tamamlandı':'Dump analysis partially completed','Dump analizi tamamlanamadı':'Dump analysis could not be completed','Dump dosyası bulunamadı':'Dump file not found','Dump klasörüne erişilemedi':'Dump folder could not be accessed','Dump okunamadı veya analiz edilemedi. Diğer sonuçlar hazır; ayrıntılar motor raporunda.':'The dump could not be read or analyzed. Other results are ready; details are in the engine report.','Bağlantı kurulamadı':'Connection failed','Başka bir tarama devam ediyor.':'Another scan is already running.','Başka bir tarama sürüyor. Tamamlanmasını bekleyip yeniden dene.':'Another scan is running. Wait for it to finish and try again.','Bu sayfanın servise erişimine izin verilmedi. Arayüzü Syspect Desktop üzerinden aç.':'This page is not allowed to access the service. Open the interface through Syspect Desktop.','Motorun yanıt biçimi bu arayüzle uyumlu değil.':'The engine response format is not compatible with this interface.','Motorun yanıtı okunamadı. Paketteki motor ve agent sürümlerinin eşleştiğini kontrol et.':'The engine response could not be read. Check that the engine and agent versions in the package match.','Motorun çökme çevresi özeti.':'Engine summary around the crash.','Beklenmeyen kapanma':'Unexpected shutdown','Dosya sistemi (NTFS)':'File system (NTFS)','Ekran sürücüsü':'Display driver','mantıksal işlemci':'logical processors','mimarisi':'architecture','Modül':'Module'
  };


  Object.assign(ORIGINAL_TR_TO_EN,{
    'Tarama motoru bulunamadı. diagnosticengine.exe dosyasının paket klasöründe olduğunu kontrol et.':'Scan engine not found. Check that diagnosticengine.exe is in the package folder.',
    'Tarama motoru başlatılamadı. Servis penceresini kontrol et.':'Scan engine could not be started. Check the service window.',
    'Tarama süre sınırını aştı. Yeniden deneyebilirsin.':'The scan exceeded the time limit. You can try again.',
    'Tarama motoru hata ile kapandı. Servis penceresini kontrol et.':'The scan engine exited with an error. Check the service window.',
    'Tarama raporu boyut sınırını aştı.':'The scan report exceeded the size limit.',
    'İstek zaman aşımına uğradı. Yeniden deneyebilirsin.':'The request timed out. You can try again.',
    'Servise ulaşılamadı. Syspect Desktop uygulamasını yeniden başlat.':'The service could not be reached. Restart the Syspect Desktop application.',
    'Tarama tamamlanamadı. Servis penceresini kontrol edip yeniden dene.':'The scan could not be completed. Check the service window and try again.',
    'Tarama sonucu':'Scan result','Sürücü sürümleri aşağıdaki metin raporunda bulunabilir. Güncellik karşılaştırması yapılmadı.':'Driver versions are available in the text report below. No version freshness comparison was performed.',
    'Veri alınamadı. Zaman çizelgesi henüz ayrı olarak aktarılmıyor. Varsa zaman korelasyonunu aşağıdaki metin raporundan inceleyebilirsin.':'No data was available. The timeline is not yet exported separately. Review any time correlation in the text report below.',
    'Tarama servisi meşgul':'Scan service busy','Tarama servisi hazır':'Scan service ready','Taramayı başlat düğmesiyle bilgisayarını inceleyebilirsin.':'You can inspect your computer with the Start Scan button.',
    'Tarama sürüyor':'Scan in progress','Sistem, sürücüler ve olay kayıtları inceleniyor. Dump analizi birkaç dakika sürebilir.':'System, drivers, and event logs are being inspected. Dump analysis may take a few minutes.',
    'Tarama tamamlandı · kısmi sonuç':'Scan completed · partial result','Tarama tamamlandı':'Scan completed','Tarama tamamlanamadı':'Scan failed','Tarama servisi':'Scan service',
    'Seçili olay türlerinin tutulan geçmişteki sayılarıdır. Olay bulunması, tek başına arızanın nedenini kanıtlamaz.':'Counts of selected event types in retained history. The presence of an event alone does not prove the cause of a fault.',
    'Tarama durumu':'Scan status','Tarama sürüyor':'Scan in progress','Yerel çalışma alanı':'Local workspace','Syspect ana sayfa':'Syspect home'
  });

  // Complete UI copy, including diagnostic evidence and remediation.
  Object.assign(TR,{
  "Download": "İndir",
  "Download Syspect": "Syspect’i İndir",
  "Download Syspect for Windows.": "Windows için Syspect’i indirin.",
  "Download for Windows": "Windows için İndir",
  "Download and extract the ZIP.": "ZIP dosyasını indirip çıkarın.",
  "Install in three steps": "Üç adımda kurulum",
  "Native diagnostics": "Yerel teşhis",
  "Local-only bridge": "Yalnızca yerel bağlantı",
  "App-style window": "Uygulama penceresi",
  "Open": "Açın",
  "Run": "Çalıştırın",
  "and choose": "ve şunu seçin:",
  ". The installer creates Start Menu and Desktop shortcuts.": ". Yükleyici, Başlat menüsü ve masaüstü kısayollarını oluşturur.",
  "01 / LOCAL": "01 / YEREL",
  "02 / PRIVATE": "02 / ÖZEL",
  "03 / DESKTOP": "03 / MASAÜSTÜ",
  "SYSPECT DESKTOP": "SYSPECT MASAÜSTÜ",
  "DESKTOP / 1.0.1": "MASAÜSTÜ / 1.0.1",
  "WORKSPACE / 01": "ÇALIŞMA ALANI / 01",
  "Create Account.": "Hesap Oluştur.",
  "Switch to dark mode": "Koyu temaya geç",
  "Switch to light mode": "Açık temaya geç",
  "Portable package with the native diagnostic engine, desktop host, installer, EN/TR language support and dark/light themes.": "Yerel teşhis motoru, masaüstü uygulaması, yükleyici, İngilizce/Türkçe dil desteği ve koyu/açık temalar içeren taşınabilir paket.",
  "Run the real diagnostic engine locally. Syspect Desktop starts the native engine on your PC, keeps scan data on the machine, and displays the results in the full Syspect workspace.": "Gerçek teşhis motorunu yerel olarak çalıştırın. Syspect Desktop motoru bilgisayarınızda başlatır, tarama verilerini cihazda saklar ve sonuçları Syspect çalışma alanında gösterir.",
  "Runs diagnosticengine.exe directly on the Windows machine and returns structured JSON to the Syspect UI.": "diagnosticengine.exe dosyasını doğrudan Windows bilgisayarında çalıştırır ve yapılandırılmış JSON verilerini Syspect arayüzüne aktarır.",
  "The desktop host listens only on 127.0.0.1. Scan traffic does not need a remote server.": "Masaüstü uygulaması yalnızca 127.0.0.1 adresini dinler. Tarama trafiği için uzak sunucu gerekmez.",
  "The desktop package does not require Python. Microsoft Edge and Windows PowerShell are used by the desktop host.": "Masaüstü paketi Python gerektirmez. Masaüstü uygulaması Microsoft Edge ve Windows PowerShell kullanır.",
  "Syspect opens in Microsoft Edge app mode, with the local engine connected automatically.": "Syspect, yerel motora otomatik bağlanarak Microsoft Edge uygulama modunda açılır.",
  "The intelligent, rule-based diagnostic engine that pinpoints the root cause of your PC performance issues with evidence-backed analysis.": "Bilgisayarınızdaki performans sorunlarının kök nedenini kanıta dayalı analizle belirleyen akıllı, kural tabanlı teşhis motoru.",
  "Select an interactive diagnostic workflow or inspect your deep kernel telemetry report.": "Etkileşimli bir teşhis akışı seçin veya ayrıntılı çekirdek telemetri raporunuzu inceleyin.",
  "Identify thermal throttling, RAM limitations, and storage bottlenecks specifically in your gaming sessions.": "Oyun oturumlarınızdaki termal kısıtlamaları, RAM sınırlamalarını ve depolama darboğazlarını belirleyin.",
  "Analyze crash codes, dump files, and system behavior to find faulty drivers or failing hardware components.": "Hatalı sürücüleri veya arızalanan donanım bileşenlerini bulmak için çökme kodlarını, döküm dosyalarını ve sistem davranışını analiz edin.",
  "Detect bloated startup registries, background malware, and physical drive degradation causing slow boots.": "Yavaş açılışa neden olan aşırı başlangıç kayıtlarını, arka plandaki zararlı yazılımları ve fiziksel disk bozulmasını tespit edin.",
  "Perform a deep system analysis to obtain clearer results. Scans kernel-level processes, PnP timelines, and raw hardware data.": "Daha net sonuçlar için derin sistem analizi yapın. Çekirdek düzeyindeki işlemleri, PnP zaman çizelgelerini ve ham donanım verilerini tarar.",
  "Syspect — BSOD Crash Analysis": "Syspect — Mavi Ekran Çökme Analizi",
  "Syspect — Create Account": "Syspect — Hesap Oluştur",
  "Syspect — Dashboard": "Syspect — Kontrol Paneli",
  "Syspect — Download for Windows": "Syspect — Windows için İndir",
  "Syspect — FPS & Thermal Diagnosis Engine": "Syspect — FPS ve Termal Teşhis Motoru",
  "Syspect — Professional System Diagnostics": "Syspect — Profesyonel Sistem Teşhisi",
  "Syspect — Scan History": "Syspect — Tarama Geçmişi",
  "Syspect — Slow Boot & System Lag": "Syspect — Yavaş Açılış ve Sistem Gecikmesi",
  "Syspect — Welcome Back": "Syspect — Tekrar Hoş Geldiniz",
  "Your system is intentionally reducing its performance to protect the hardware from heat damage.": "Sisteminiz, donanımı ısının vereceği zarardan korumak için performansını bilinçli olarak düşürüyor.",
  "Clean dust from vents, fans, and heatsinks.": "Havalandırma delikleri, fanlar ve soğutuculardaki tozu temizleyin.",
  "Repaste the CPU/GPU if the device is over 2 years old.": "Cihaz iki yıldan eskiyse CPU/GPU termal macununu yenileyin.",
  "If using a laptop, elevate the back or use a cooling pad.": "Dizüstü bilgisayar kullanıyorsanız arkasını yükseltin veya soğutucu stand kullanın.",
  "Set fan profiles to \"Aggressive\" or \"Performance\" in your control center.": "Kontrol merkezinizde fan profilini \"Agresif\" veya \"Performans\" olarak ayarlayın.",
  "Your hard drive cannot read game assets fast enough, causing the game to freeze while waiting for data.": "Sabit diskiniz oyun dosyalarını yeterince hızlı okuyamıyor; oyun veri beklerken donuyor.",
  "Move your frequently played games to an SSD.": "Sık oynadığınız oyunları bir SSD’ye taşıyın.",
  "Defragment your HDD (Note: Never defragment an SSD).": "HDD’nizde disk birleştirme yapın (Not: SSD’de asla disk birleştirme yapmayın).",
  "Upgrade to an NVMe or SATA SSD for a massive performance boost.": "Performansı önemli ölçüde artırmak için NVMe veya SATA SSD’ye geçin.",
  "Your CPU or GPU is consistently hitting 100% usage and struggling to render the game frames.": "CPU veya GPU’nuz sürekli %100 kullanıma ulaşıyor ve oyun karelerini oluşturmakta zorlanıyor.",
  "Lower overall in-game graphics settings.": "Oyun içi genel grafik ayarlarını düşürün.",
  "Enable upscaling technologies like NVIDIA DLSS or AMD FSR if supported.": "Destekleniyorsa NVIDIA DLSS veya AMD FSR gibi görüntü yükseltme teknolojilerini etkinleştirin.",
  "Performance is poor immediately upon launching games.": "Oyun başlatılır başlatılmaz performans düşük.",
  "Performance degrades after extended gameplay (15-20 mins).": "Uzun süreli oyun sonrasında (15–20 dakika) performans düşüyor.",
  "Stutters occur during high-intensity scenes and rapid rendering.": "Yoğun sahnelerde ve hızlı görüntü oluşturma sırasında takılmalar oluyor.",
  "Frame spikes under burst shader workloads.": "Ani gölgelendirici yüklerinde kare süreleri sıçrıyor.",
  "Using a Gaming Laptop, which naturally has restricted airflow.": "Hava akışı yapısı gereği sınırlı olan bir oyun dizüstü bilgisayarı kullanılıyor.",
  "Games are running on a slow mechanical hard drive (HDD).": "Oyunlar yavaş bir mekanik sabit disk (HDD) üzerinde çalışıyor.",
  "Elevated temperatures present even under idle desktop load.": "Masaüstünde boşta çalışırken bile sıcaklıklar yüksek.",
  "Thermal saturation occurs after 15-20 minutes of sustained load.": "15–20 dakikalık sürekli yük sonrasında soğutma kapasitesinin sınırına ulaşılıyor.",
  "Thermal spikes correlate with 100% component utilization.": "Sıcaklık sıçramaları bileşenlerin %100 kullanımıyla örtüşüyor.",
  "Compact laptop chassis restricts thermal dissipation.": "Kompakt dizüstü kasası ısı dağılımını sınırlıyor.",
  "Cooling assembly has not been serviced in over a year.": "Soğutma sistemine bir yıldan uzun süredir bakım yapılmamış.",
  "No explicit analytical anomalies flagged.": "Analizde belirgin bir anormallik işaretlenmedi.",
  "Your system memory (RAM) is either physically failing or running at unstable speeds, corrupting critical Windows processes.": "Sistem belleğiniz (RAM) fiziksel olarak arızalanıyor veya kararsız hızlarda çalışarak kritik Windows işlemlerini bozuyor.",
  "Disable XMP/EXPO/DOCP profiles in your BIOS immediately.": "BIOS’taki XMP/EXPO/DOCP profillerini hemen devre dışı bırakın.",
  "Run MemTest86 via a USB drive to check for physical hardware faults.": "Fiziksel donanım hatalarını denetlemek için USB bellekten MemTest86 çalıştırın.",
  "Reseat your RAM sticks, ensuring they click into the motherboard firmly.": "RAM modüllerini anakarta sıkıca oturduklarından emin olarak yeniden takın.",
  "If using 4 sticks, drop down to 2 sticks in slots 2 and 4 to test memory controller load.": "Dört modül kullanıyorsanız bellek denetleyicisi yükünü test etmek için 2. ve 4. yuvalarda iki modülle deneyin.",
  "Your graphics card stopped responding to Windows (TDR), forcing the OS to crash to protect the system.": "Ekran kartınız Windows’a yanıt vermeyi bıraktı (TDR); işletim sistemi, sistemi korumak için durdu.",
  "Download DDU (Display Driver Uninstaller), boot into Safe Mode, and wipe all GPU drivers completely.": "DDU’yu (Display Driver Uninstaller) indirin, Güvenli Mod’da başlatın ve tüm GPU sürücülerini tamamen kaldırın.",
  "Reinstall the latest stable (not beta) graphics drivers.": "En güncel kararlı (beta olmayan) ekran kartı sürücülerini yeniden yükleyin.",
  "If using AMD, disable FreeSync and Anti-Lag in Adrenalin.": "AMD kullanıyorsanız Adrenalin’de FreeSync ve Anti-Lag özelliklerini kapatın.",
  "Check GPU junction temperatures while gaming to ensure it is not thermal throttling to a halt.": "Isı nedeniyle durma noktasına kadar kısıtlanmadığından emin olmak için oyun sırasında GPU bağlantı noktası sıcaklıklarını kontrol edin.",
  "A fatal hardware error (WHEA) occurred at the processor level, usually due to incorrect voltage or failing VRMs.": "İşlemci düzeyinde, genellikle yanlış voltaj veya arızalı VRM’lerden kaynaklanan kritik bir donanım hatası (WHEA) oluştu.",
  "Reset your motherboard BIOS to \"Optimized Defaults\" to clear any unstable voltage curves.": "Kararsız voltaj eğrilerini temizlemek için anakart BIOS’unu \"Optimize Edilmiş Varsayılanlar\" ayarına sıfırlayın.",
  "Update your BIOS to the latest version to improve CPU stability.": "CPU kararlılığını artırmak için BIOS’u en son sürüme güncelleyin.",
  "Monitor CPU temps; if hitting 95C+, repaste the CPU and check cooler mount.": "CPU sıcaklıklarını izleyin; 95°C ve üstüne çıkıyorsa termal macunu yenileyin ve soğutucu montajını kontrol edin.",
  "If undervolting (Curve Optimizer), reduce the negative offset.": "Voltaj düşürme (Curve Optimizer) kullanıyorsanız negatif ofseti azaltın.",
  "A third-party driver or recent Windows update is trying to access protected kernel memory improperly.": "Üçüncü taraf bir sürücü veya son Windows güncellemesi, korunan çekirdek belleğine uygunsuz şekilde erişmeye çalışıyor.",
  "Boot into Safe Mode and uninstall recently added software or drivers.": "Güvenli Mod’da başlatın ve yakın zamanda eklenen yazılım veya sürücüleri kaldırın.",
  "Run \"sfc /scannow\" and \"DISM /Online /Cleanup-Image /RestoreHealth\" in Command Prompt.": "Komut İstemi’nde \"sfc /scannow\" ve \"DISM /Online /Cleanup-Image /RestoreHealth\" komutlarını çalıştırın.",
  "Check Device Manager for any hardware flagged with a yellow warning triangle.": "Aygıt Yöneticisi’nde sarı uyarı üçgeniyle işaretlenmiş donanımları kontrol edin.",
  "Windows failed to read the pagefile or critical kernel data from your SSD/HDD.": "Windows, SSD/HDD’nizdeki sayfalama dosyasını veya kritik çekirdek verilerini okuyamadı.",
  "Run \"chkdsk /f /r\" via Command Prompt as Administrator.": "Komut İstemi’ni yönetici olarak açıp \"chkdsk /f /r\" komutunu çalıştırın.",
  "Download CrystalDiskInfo to check the health status of your drives.": "Disklerinizin sağlık durumunu kontrol etmek için CrystalDiskInfo’yu indirin.",
  "Ensure your NVMe or SATA cables are firmly seated.": "NVMe bağlantınızın veya SATA kablolarınızın sıkıca takılı olduğundan emin olun.",
  "Stop code falls under Memory/RAM fault category.": "Durma kodu, Bellek/RAM hatası kategorisine giriyor.",
  "Stop code indicates an IRQL or Driver-level kernel panic.": "Durma kodu, IRQL veya sürücü düzeyinde bir çekirdek hatasını gösteriyor.",
  "Stop code is directly tied to GPU Timeout Detection and Recovery (TDR).": "Durma kodu, doğrudan GPU zaman aşımı algılama ve kurtarma (TDR) mekanizmasıyla ilişkili.",
  "Stop code signifies a fatal hardware communication error (WHEA/Watchdog).": "Durma kodu, kritik bir donanım iletişim hatasını (WHEA/Watchdog) gösteriyor.",
  "System failed to read kernel data from the storage pagefile.": "Sistem, depolama birimindeki sayfalama dosyasından çekirdek verilerini okuyamadı.",
  "XMP/Overclock is active, which can cause memory controller instability.": "XMP/hız aşırtma etkin; bu durum bellek denetleyicisinde kararsızlığa yol açabilir.",
  "New RAM was recently installed; potential seating or compatibility issue.": "Yakın zamanda yeni RAM takılmış; oturma veya uyumluluk sorunu olabilir.",
  "Stock RAM settings crashing points to physical degradation of the stick.": "Varsayılan RAM ayarlarında çökme, modülde fiziksel bozulmaya işaret ediyor.",
  "Memory diagnostic tools explicitly confirmed physical sector errors.": "Bellek teşhis araçları fiziksel bellek hatalarını doğruladı.",
  "Memory has not been professionally stress-tested yet.": "Belleğe henüz kapsamlı bir stres testi uygulanmamış.",
  "NVIDIA driver crash detected. Often related to Studio vs Game Ready driver conflicts.": "NVIDIA sürücü çökmesi algılandı. Genellikle Studio ve Game Ready sürücüsü çakışmalarıyla ilişkilidir.",
  "AMD driver crash detected. FreeSync or Adrenalin software timeouts are common culprits.": "AMD sürücü çökmesi algılandı. FreeSync veya Adrenalin yazılımındaki zaman aşımları yaygın nedenlerdir.",
  "Crash occurs specifically under graphical load, indicating power spikes or thermal limits.": "Çökme özellikle grafik yükü altında oluyor; güç sıçramalarına veya termal sınırlara işaret ediyor.",
  "Issue aligns perfectly with a recent driver update, suggesting corrupt software installation.": "Sorun son sürücü güncellemesiyle örtüşüyor; bozuk yazılım kurulumu olasılığını gösteriyor.",
  "Recent update corrupted the display driver registry.": "Son güncelleme ekran sürücüsü kayıtlarını bozmuş.",
  "CPU voltages/clocks have been altered, leading to processing miscalculations.": "CPU voltajları/saat hızları değiştirilmiş ve hesaplama hatalarına yol açmış.",
  "Stock CPU throwing WHEA suggests failing motherboard VRMs or critical thermal event.": "Varsayılan ayarlardaki CPU’nun WHEA hatası vermesi, anakart VRM arızasına veya kritik bir termal olaya işaret ediyor.",
  "Outdated BIOS may have known voltage distribution bugs with current CPU.": "Eski BIOS sürümünde mevcut CPU ile ilgili bilinen voltaj dağıtımı hataları olabilir.",
  "Insufficient telemetry data. Check system logs.": "Telemetri verileri yetersiz. Sistem günlüklerini kontrol edin.",
  "No, haven't tested it yet": "Hayır, henüz test etmedim",
  "No / I don't know": "Hayır / Bilmiyorum",
  "Unknown / I don't know": "Bilinmiyor / Bilmiyorum",
  "Is there a noticeable speed difference between \"Shut Down\" and \"Restart\"?": "\"Kapat\" ve \"Yeniden Başlat\" arasında belirgin bir hız farkı var mı?",
  "Yes, Cold Boot (Shut Down) is very slow, but \"Restart\" is fast": "Evet, kapatıp açmak çok yavaş ancak yeniden başlatmak hızlı",
  "Classic Fast Startup behavior.": "Tipik Hızlı Başlatma davranışı.",
  "Windows handles kernel memory dumps differently across these cycles.": "Windows, bu işlemlerde çekirdek bellek dökümlerini farklı şekilde işler.",
  "Too many high-impact applications (Discord, Steam, RGB/Control Centers) are initialization-locked, launching simultaneously during boot and choking disk/CPU pipelines.": "Çok sayıda yüksek etkili uygulama (Discord, Steam, RGB/kontrol merkezleri) açılışta aynı anda başlayarak disk ve CPU işlemlerini tıkıyor.",
  "Open Task Manager (Ctrl+Shift+Esc), navigate to \"Startup Apps\", and disable unneeded background services.": "Görev Yöneticisi’ni (Ctrl+Shift+Esc) açın, \"Başlangıç Uygulamaları\" bölümüne gidin ve gereksiz arka plan hizmetlerini devre dışı bırakın.",
  "Uninstall competing system monitoring or duplicate vendor control centers.": "Birbiriyle çakışan sistem izleme araçlarını veya yinelenen üretici kontrol merkezlerini kaldırın.",
  "Delay secondary application triggers via task scheduler configurations.": "Görev Zamanlayıcı ayarlarıyla ikincil uygulamaların başlangıcını geciktirin.",
  "Unsolicited software hooks are hijacking your CPU/Disk threads immediately at startup to mine crypto or run telemetry commands, causing permanent lag.": "İstenmeyen yazılımlar, açılışta CPU/disk kaynaklarını kripto madenciliği veya telemetri komutları için kullanarak sürekli gecikmeye yol açıyor.",
  "Initiate a Full Scan utilizing Windows Defender offline mode.": "Windows Defender çevrimdışı modunu kullanarak tam tarama başlatın.",
  "Run a dedicated remediation cycle with Malwarebytes Free to scan root directories.": "Kök dizinleri taramak için Malwarebytes Free ile kapsamlı bir tarama ve temizleme yapın.",
  "Verify unrecognized executable roots inside your Task Manager \"Details\" tab.": "Görev Yöneticisi’nin \"Ayrıntılar\" sekmesinde tanımadığınız çalıştırılabilir dosyaların konumlarını kontrol edin.",
  "Your storage unit is either physically degrading (bad sectors, corrupted NAND controllers) or is a mechanical HDD unable to sustain modern Windows pagefile speeds.": "Depolama biriminiz fiziksel olarak bozuluyor (bozuk sektörler, arızalı NAND denetleyicileri) veya modern Windows sayfalama hızlarına yetişemeyen mekanik bir HDD kullanıyorsunuz.",
  "Launch Command Prompt as Admin and run \"chkdsk /f /r\" to isolate damaged logical sectors.": "Hasarlı mantıksal sektörleri belirlemek için Komut İstemi’ni yönetici olarak açıp \"chkdsk /f /r\" çalıştırın.",
  "Check disk SMART logs with CrystalDiskInfo. If labeled \"Caution/Bad\", clone data immediately.": "CrystalDiskInfo ile disk SMART kayıtlarını kontrol edin. \"Dikkat/Kötü\" görünüyorsa verileri hemen klonlayın.",
  "Upgrade your operating system deployment sector to a dedicated NVMe or SATA SSD.": "İşletim sisteminizi ayrı bir NVMe veya SATA SSD’ye taşıyın.",
  "Core Windows binaries or system registry configurations became corrupted following a interrupted patch lifecycle or forced shut down sequence.": "Temel Windows dosyaları veya sistem kayıt ayarları, yarıda kalan bir güncelleme ya da zorla kapatma sonrasında bozulmuş.",
  "Run \"sfc /scannow\" in an elevated command prompt to repair missing manifest files.": "Eksik bildirim dosyalarını onarmak için yönetici Komut İstemi’nde \"sfc /scannow\" çalıştırın.",
  "Execute \"DISM /Online /Cleanup-Image /RestoreHealth\" to replace broken system image segments.": "Bozuk sistem görüntüsü bölümlerini onarmak için \"DISM /Online /Cleanup-Image /RestoreHealth\" çalıştırın.",
  "Clear the update distribution path cache by resetting the SoftwareDistribution directory.": "SoftwareDistribution dizinini sıfırlayarak güncelleme dağıtım önbelleğini temizleyin.",
  "Critical third-party system drivers (frequently associated with anti-cheat tools, sound drivers, or RGB frameworks) are taking too long to link during the boot interface.": "Kritik üçüncü taraf sistem sürücülerinin (çoğunlukla hile önleme araçları, ses sürücüleri veya RGB yazılımları) açılışta devreye girmesi çok uzun sürüyor.",
  "Perform a clean installation of primary platform drivers (GPU, Chipset) using DDU tools.": "DDU araçlarını kullanarak temel platform sürücülerini (GPU, yonga seti) temiz şekilde yükleyin.",
  "Isolate system services via Clean Boot configurations to pinpoint the failing driver string.": "Sorunlu sürücüyü belirlemek için Temiz Önyükleme ayarlarıyla sistem hizmetlerini tek tek sınayın.",
  "Verify if entering Safe Mode removes the system boot latency entirely.": "Güvenli Mod’a geçmenin açılış gecikmesini tamamen giderip gidermediğini kontrol edin.",
  "Your machine lacks enough physical RAM allocation space, forcing Windows to aggressively dump active kernel workloads into the slower virtual disk Pagefile cache.": "Bilgisayarınızda yeterli fiziksel RAM yok; Windows etkin çekirdek iş yüklerini daha yavaş olan disk üzerindeki sanal sayfalama dosyasına aktarmak zorunda kalıyor.",
  "Limit concurrent background tabs and drop auto-boot overhead scripts.": "Arka planda aynı anda açık olan sekmeleri sınırlayın ve gereksiz otomatik başlangıç betiklerini kapatın.",
  "Verify that your virtual memory allocation settings are handled automatically by Windows.": "Sanal bellek ayarlarınızın Windows tarafından otomatik yönetildiğini doğrulayın.",
  "Consider expanding your dual-channel architecture footprint to 16GB minimum.": "Çift kanallı bellek kapasitenizi en az 16 GB’a yükseltmeyi değerlendirin.",
  "Windows Fast Startup feature is dumping a corrupted hibernation image cache file (hiberfil.sys) onto the disk, looping errors across clean boot sequences.": "Windows Hızlı Başlatma özelliği, bozuk bir hazırda bekletme görüntüsünü (hiberfil.sys) diske yazarak hataların açılışlar boyunca tekrarlanmasına neden oluyor.",
  "Navigate to Control Panel > Power Options > \"Choose what the power buttons do\".": "Denetim Masası > Güç Seçenekleri > \"Güç düğmelerinin yapacaklarını seçin\" yolunu izleyin.",
  "Click \"Change settings that are currently unavailable\", then uncheck \"Turn on fast startup\".": "\"Şu anda kullanılamayan ayarları değiştir\" seçeneğine tıklayın ve \"Hızlı başlatmayı aç\" işaretini kaldırın.",
  "Open Admin CMD and run \"powercfg -h off\" to purge corrupted hibernation system tables entirely.": "Bozuk hazırda bekletme verilerini tamamen temizlemek için yönetici Komut İstemi’nde \"powercfg -h off\" çalıştırın.",
  "Boot duration falls into the 15-45s margin (Potential startup apps pressure).": "Açılış süresi 15–45 saniye aralığında (başlangıç uygulamalarının yükü olası neden).",
  "Boot duration is significantly delayed (45-120s).": "Açılış belirgin biçimde gecikiyor (45–120 saniye).",
  "Prolonged boot indicates potential system registry or file validation delays.": "Uzun açılış süresi, sistem kayıtları veya dosya doğrulama gecikmelerine işaret ediyor.",
  "Extreme boot time (>120s) points directly to mechanical disk latency or hardware sector decay.": "Aşırı uzun açılış süresi (>120 saniye), mekanik disk gecikmesine veya donanım sektörlerinin bozulmasına işaret ediyor.",
  "Desktop environment locks up immediately after login.": "Masaüstü ortamı oturum açıldıktan hemen sonra kilitleniyor.",
  "System lockup post-boot implies severe active disk loading queues.": "Açılış sonrası sistem kilitlenmesi, yoğun disk yükleme kuyruklarına işaret ediyor.",
  "Abrupt idle CPU and fan activity upon boot indicates unauthorized background tasks.": "Açılışta boşta CPU ve fan etkinliğinin aniden artması, izinsiz arka plan görevlerine işaret ediyor.",
  "OS is running on a mechanical HDD, forcing massive queue lengths and 100% disk usage bugs.": "İşletim sistemi mekanik HDD’de çalışıyor; uzun işlem kuyrukları ve %100 disk kullanım sorunları oluşuyor.",
  "System slowdown directly correlates with a recent Windows core/driver patch configuration.": "Sistemin yavaşlaması son Windows çekirdek/sürücü güncellemesiyle doğrudan örtüşüyor.",
  "Potential driver registry or service collision post-update.": "Güncelleme sonrasında sürücü kaydı veya hizmet çakışması olabilir.",
  "Slowdown timeline began after deploying unverified third-party/cracked installations.": "Yavaşlama, doğrulanmamış üçüncü taraf veya korsan yazılım kurulumlarından sonra başlamış.",
  "Gradual operational decay over months points to sequential hardware storage sector wear.": "Aylar boyunca kademeli kötüleşme, depolama sektörlerinin aşınmasına işaret ediyor.",
  "Accumulation of background application services over time.": "Zaman içinde arka plan uygulama hizmetleri birikmiş.",
  "System anomalies clear upon a soft reboot but persist on cold boot (Fast Startup corruption template).": "Sistem sorunları yeniden başlatmada kayboluyor ancak kapatıp açmada sürüyor (Hızlı Başlatma bozulması örüntüsü).",
  "Total system RAM is 8GB or less, triggering constant virtual pagefile cache thrashing on disk.": "Toplam RAM 8 GB veya daha az; diskteki sanal sayfalama dosyasına sürekli veri aktarılıyor.",
  "Task Manager confirms >10 active startup applications enabled.": "Görev Yöneticisi, 10’dan fazla başlangıç uygulamasının etkin olduğunu doğruluyor.",
  "No prominent hardware bottlenecks or anomalous software behaviors flagged.": "Belirgin bir donanım darboğazı veya olağandışı yazılım davranışı işaretlenmedi.",
  "Request failed.": "İstek başarısız oldu.",
  "Data unavailable": "Veri alınamadı",
  "Scanning…": "Taranıyor…",
  "Scan again": "Yeniden tara",
  "Saved scan": "Kayıtlı tarama",
  "Your last engine scan has been loaded. You can start a new scan.": "Son motor taraman yüklendi. Yeni bir tarama başlatabilirsin.",
  "Process": "İşlem",
  "Crash time": "Çökme zamanı",
  "Exception code": "İstisna kodu",
  "Before crash": "Çökme öncesi",
  "After crash": "Çökme sonrası",
  "Restart / report": "Yeniden başlatma / rapor",
  "Driver initialization (PnP)": "Sürücü başlatma (PnP)",
  "WHEA hardware logs": "WHEA donanım kayıtları",
  "Open scan history →": "Tarama geçmişini aç →",
  "System information, crash analysis and events in one workspace. Real local scan results.": "Sistem bilgilerin, çökme analizi ve olayların tek bir çalışma alanında. Gerçek yerel tarama sonuçları.",
  "Konya / Ereğli | Turkey": "Konya / Ereğli | Türkiye",
  "I don't have CrystalDiskInfo installed": "CrystalDiskInfo kurulu değil",
  "Choose File (C:\\\\Windows\\\\Minidump)": "Dosya Seç (C:\\\\Windows\\\\Minidump)"
});
  Object.assign(TR,{'Username':'Kullanıcı adı','Overdue':'Bakımı gecikmiş','Recent':'Yakın zamanda','vgk.sys · Event Correlation':'vgk.sys · Olay Korelasyonu','Raw engine output (original language)':'Ham motor çıktısı (özgün dil)'});
  Object.entries(ORIGINAL_TR_TO_EN).forEach(([tr,en])=>{if(!TR[en])TR[en]=tr;});

  const normalize=s=>String(s).replace(/\s+/g,' ').trim();
  const reverse={}; Object.keys(TR).forEach(k=>reverse[normalize(TR[k])]=k);
  function saved(){try{const v=localStorage.getItem(KEY);return v==='tr'?'tr':'en'}catch(e){return'en'}}
  let lang=saved();
  const textSources=new WeakMap(), attributeSources=new WeakMap();
  const patterns=[
    [/^Step (\d+) \/ (\d+)$/, /^Adım (\d+) \/ (\d+)$/, (a,b)=>'Step '+a+' / '+b, (a,b)=>'Adım '+a+' / '+b],
    [/^Severity: (.+)$/, /^Önem Derecesi: (.+)$/, a=>'Severity: '+translateString(a,'en'), a=>'Önem Derecesi: '+translateString(a,'tr')],
    [/^DIAGNOSIS: (.+)$/, /^TEŞHİS: (.+)$/, a=>'DIAGNOSIS: '+translateString(a,'en').toLocaleUpperCase('en'), a=>'TEŞHİS: '+translateString(a,'tr').toLocaleUpperCase('tr')],
    [/^(\d+) devices detected$/, /^(\d+) cihaz algılandı$/, a=>a+' devices detected', a=>a+' cihaz algılandı'],
    [/^(\d+) problematic devices$/, /^(\d+) problemli cihaz$/, a=>a+' problematic devices', a=>a+' problemli cihaz'],
    [/^Devices: (.+)$/, /^Cihazlar: (.+)$/, a=>'Devices: '+a, a=>'Cihazlar: '+a],
    [/^Problematic devices: (.+)$/, /^Problemli cihazlar: (.+)$/, a=>'Problematic devices: '+a, a=>'Problemli cihazlar: '+a],
    [/^(\d+) logical processors$/, /^(\d+) mantıksal işlemci$/, a=>a+' logical processors', a=>a+' mantıksal işlemci'],
    [/^(.+) architecture$/, /^(.+) mimarisi$/, a=>a+' architecture', a=>a+' mimarisi'],
    [/^Scan in progress · (\d+) s$/, /^Tarama sürüyor · (\d+) sn$/, a=>'Scan in progress · '+a+' s', a=>'Tarama sürüyor · '+a+' sn'],
    [/^Case #(.+)$/, /^Vaka #(.+)$/, a=>'Case #'+a, a=>'Vaka #'+a],
    [/^Minidump Case #(.+)$/, /^Minidump Vakası #(.+)$/, a=>'Minidump Case #'+a, a=>'Minidump Vakası #'+a],
    [/^Latency Report #(.+)$/, /^Gecikme Raporu #(.+)$/, a=>'Latency Report #'+a, a=>'Gecikme Raporu #'+a],
    [/^(\d+) Results$/, /^(\d+) Sonuçlar$/, a=>a+' Results', a=>a+' Sonuçlar'],
    [/^CrystalDiskInfo hard telemetry flags drive health explicitly as \[(CAUTION|BAD)\]\.$/, /^CrystalDiskInfo disk sağlığını \[(DİKKAT|KÖTÜ)\] olarak bildiriyor\.$/, a=>'CrystalDiskInfo hard telemetry flags drive health explicitly as ['+({DİKKAT:'CAUTION',KÖTÜ:'BAD'}[a]||a)+'].', a=>'CrystalDiskInfo disk sağlığını ['+({CAUTION:'DİKKAT',BAD:'KÖTÜ'}[a]||a)+'] olarak bildiriyor.']
  ];
  function translateString(value,target){
    if(!value || !normalize(value)) return value;
    const raw=String(value), lead=raw.match(/^\s*/)[0], trail=raw.match(/\s*$/)[0], valueKey=normalize(raw);
    const extra=window.SyspectAccountTranslations||{};
    const extraKey=Object.keys(extra).find(key=>normalize(extra[key])===valueKey);
    let canonical=Object.hasOwn(TR,valueKey)||Object.hasOwn(extra,valueKey) ? valueKey : (ORIGINAL_TR_TO_EN[valueKey] || extraKey || reverse[valueKey] || valueKey);
    let out=target==='tr' ? (extra[canonical] || TR[canonical]) : canonical;
    if(target==='tr' && out===undefined || target==='en' && out===valueKey){
      // Headings generated with toUpperCase still use the same translation entry.
      const key=Object.keys(TR).find(key=>key.toLocaleUpperCase('en')===valueKey || TR[key].toLocaleUpperCase('tr')===valueKey);
      if(key)out=(target==='tr'?TR[key]:key).toLocaleUpperCase(target);
      else for(const [en,tr,english,turkish] of patterns){
        const match=valueKey.match(en)||valueKey.match(tr);
        if(match){out=(target==='tr'?turkish:english)(...match.slice(1));break;}
      }
    }
    return lead+(out===undefined?valueKey:out)+trail;
  }
  function stableTranslation(owner,key,value){
    const cache=key===null?textSources:attributeSources;
    let entries=cache.get(owner);if(!entries){entries={};cache.set(owner,entries);}
    const id=key||'text';let entry=entries[id];
    if(!entry || entry.output!==value)entry=entries[id]={source:value,output:value};
    entry.output=translateString(entry.source,lang);return entry.output;
  }
  function translateNode(node){
    if(node.nodeType===3){
      if(node.parentElement?.closest('[translate="no"],script,style,code,pre,textarea,.language-toggle'))return;
      const value=stableTranslation(node,null,node.nodeValue);
      if(node.nodeValue!==value)node.nodeValue=value;
      return;
    }
    if(node.nodeType!==1 || node.closest('[translate="no"],script,style,code,pre,textarea,.language-toggle'))return;
    const attributes=['placeholder','title','aria-label','alt'];
    if(node.matches('input[type="submit"],input[type="button"],input[type="reset"]'))attributes.push('value');
    if(node.matches('meta[name="description"]'))attributes.push('content');
    attributes.forEach(a=>{if(node.hasAttribute(a)){
      const value=stableTranslation(node,a,node.getAttribute(a));
      if(node.getAttribute(a)!==value)node.setAttribute(a,value);
    }});
    Array.from(node.childNodes).forEach(translateNode);
  }
  function updateControl(){
    document.documentElement.lang=lang==='tr'?'tr':'en';
    const b=document.querySelector('.language-toggle');
    if(!b) return;
    const label=lang==='tr'?'TR':'EN';
    const title=lang==='tr'?'Switch to English':'Türkçeye geç';
    // Avoid creating childList mutations when the control is already correct.
    if(b.textContent!==label) b.textContent=label;
    if(b.title!==title) b.title=title;
    if(b.getAttribute('aria-label')!==title) b.setAttribute('aria-label',title);
  }
  function apply(next,persist){
    lang=next==='tr'?'tr':'en';
    if(persist)try{localStorage.setItem(KEY,lang)}catch(e){}
    // Suspend mutation tracking while changing the whole page language. This prevents
    // our own DOM updates from being observed and translated again.
    if(typeof obs!=='undefined') obs.disconnect();
    translateNode(document.documentElement);
    updateControl();
    startObserver();
    window.dispatchEvent(new CustomEvent('syspect-language-change',{detail:{language:lang}}));
  }
  function build(){
    const host=document.querySelector('.top-right')||document.querySelector('.topbar'); if(!host||document.querySelector('.language-toggle')){apply(lang,false);return;}
    const b=document.createElement('button'); b.type='button'; b.className='language-toggle'; b.addEventListener('click',()=>apply(lang==='en'?'tr':'en',true));
    const theme=host.querySelector('.theme-toggle'); if(theme && theme.nextSibling) host.insertBefore(b,theme.nextSibling); else host.prepend(b);
    apply(lang,false);
  }
  const obs=new MutationObserver(records=>{
    obs.disconnect();
    try{
      records.forEach(r=>{
        if(r.type==='childList')r.addedNodes.forEach(translateNode);
        else translateNode(r.target);
      });
      updateControl();
    }finally{startObserver();}
  });
  function startObserver(){
    obs.observe(document.documentElement,{childList:true,subtree:true,characterData:true,attributes:true,attributeFilter:['placeholder','title','aria-label','alt','content','value']});
  }
  if(document.readyState==='loading')document.addEventListener('DOMContentLoaded',()=>{build();startObserver()});else{build();startObserver()}
  window.SyspectI18n={getLanguage:()=>lang,setLanguage:l=>apply(l,true),t:s=>translateString(s,lang)};
})();
