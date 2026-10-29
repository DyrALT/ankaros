// Başlat panelinde aranabilen ayarlar. page, Ayarlar uygulamasının sayfa kimliğidir
// (shell/apps/settings/SettingsPanel.qml). Yeni bir ayar eklenince buraya da eklenir.
.pragma library

const entries = [
    { title: "Açık ve koyu tema", page: "appearance", pageTitle: "Görünüm",
      keywords: "tema açık koyu karanlık gece mod renk görünüm" },
    { title: "Pencere düğmeleri", page: "appearance", pageTitle: "Görünüm",
      keywords: "pencere düğme kapat küçült büyüt sol sağ simge renkli daire" },
    { title: "Duvar kağıdı", page: "wallpaper", pageTitle: "Duvar kağıdı",
      keywords: "duvar kağıdı arka plan resim ankara taşı bozkır kehribar gece" },
    { title: "Kilit ekranında duvar kağıdı", page: "wallpaper", pageTitle: "Duvar kağıdı",
      keywords: "kilit giriş ekranı duvar kağıdı" },
    { title: "Sabitlenmiş uygulamalar", page: "taskbar", pageTitle: "Alt çubuk",
      keywords: "alt çubuk sabitle uygulama sıra görev" },
    { title: "Saat ve tarih", page: "taskbar", pageTitle: "Alt çubuk",
      keywords: "saat tarih 24 12 biçim" },
    { title: "Kablolu ağ", page: "network", pageTitle: "Ağ",
      keywords: "ağ internet kablo ethernet ip adres dns ağ geçidi dhcp bağlantı" },
    { title: "Şifre", page: "user", pageTitle: "Kullanıcı",
      keywords: "şifre parola kullanıcı hesap değiştir" },
    { title: "Otomatik giriş", page: "user", pageTitle: "Kullanıcı",
      keywords: "otomatik giriş oturum açılış kullanıcı" },
    { title: "Klavye düzeni", page: "keyboard", pageTitle: "Klavye ve dil",
      keywords: "klavye düzen türkçe q f ingilizce us dil yazı tuş" },
    { title: "Sistem bilgisi", page: "about", pageTitle: "Hakkında",
      keywords: "hakkında sürüm çekirdek işlemci bellek disk ekran kartı açılış süresi" }
];

// Sorgunun bütün kelimeleri başlıkta, sayfa adında ya da anahtar kelimelerde geçen ayarlar
function search(query) {
    const words = query.toLocaleLowerCase().split(/\s+/).filter(word => word.length > 0);
    if (words.length === 0)
        return [];
    return entries.filter(entry => {
        const text = (entry.title + " " + entry.pageTitle + " " + entry.keywords).toLocaleLowerCase();
        return words.every(word => text.includes(word));
    });
}
