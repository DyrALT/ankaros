.pragma library

// Simge ve uygulama simgesi verileri. Figma'daki "ankaros: Masaüstü Tasarımı" dosyasının
// Bileşenler sayfasından aktarıldı. Yollar 24x24'lük bir alan için yazılmış SVG yol verisidir,
// Icon.qml bunları Qt Shapes ile çizer (2 px kalınlıkta, yuvarlak uçlu çizgi olarak).

var paths = {
    terminal: "M5 17l6-5-6-5 M13 18h6",
    folder: "M3 7.5A2.5 2.5 0 0 1 5.5 5H9l2 2h7.5A2.5 2.5 0 0 1 21 9.5v7a2.5 2.5 0 0 1-2.5 2.5h-13A2.5 2.5 0 0 1 3 16.5z",
    globe: "M3 12a9 9 0 1 0 18 0a9 9 0 1 0 -18 0Z M3 12h18 M12 3c2.5 2.6 3.6 5.6 3.6 9s-1.1 6.4-3.6 9c-2.5-2.6-3.6-5.6-3.6-9S9.5 5.6 12 3z",
    sliders: "M4 7h9 M17 7h3 M13 7a2 2 0 1 0 4 0a2 2 0 1 0 -4 0Z M4 17h3 M11 17h9 M7 17a2 2 0 1 0 4 0a2 2 0 1 0 -4 0Z",
    pencil: "M4 20h4L19 9l-4-4L4 16z M13 7l4 4",
    music: "M9 18V6l11-2v12 M3 18a3 3 0 1 0 6 0a3 3 0 1 0 -6 0Z M14 16a3 3 0 1 0 6 0a3 3 0 1 0 -6 0Z",
    image: "M6 4h12a3 3 0 0 1 3 3v10a3 3 0 0 1 -3 3h-12a3 3 0 0 1 -3 -3v-10a3 3 0 0 1 3 -3Z M7 10a2 2 0 1 0 4 0a2 2 0 1 0 -4 0Z M21 16l-5-5-9 9",
    calendar: "M6 5h12a3 3 0 0 1 3 3v10a3 3 0 0 1 -3 3h-12a3 3 0 0 1 -3 -3v-10a3 3 0 0 1 3 -3Z M3 10h18 M8 3v4 M16 3v4",
    calculator: "M8 3h8a3 3 0 0 1 3 3v12a3 3 0 0 1 -3 3h-8a3 3 0 0 1 -3 -3v-12a3 3 0 0 1 3 -3Z M9 7.5h6 M9 12h.01 M12 12h.01 M15 12h.01 M9 16h.01 M12 16h.01 M15 16h.01",
    activity: "M3 12h4l3-7 4 14 3-7h4",
    note: "M6 3h8l4 4v14H6z M14 3v4h4 M9 12h6 M9 16h6",
    package: "M12 3l8 4.5v9L12 21l-8-4.5v-9z M4 7.5l8 4.5 8-4.5 M12 12v9",
    logo: "M5.5 19L12 5l6.5 14 M8.3 13.5h7.4",
    wifi: "M2.5 9a14 14 0 0 1 19 0 M5.5 12.5a9.5 9.5 0 0 1 13 0 M8.7 16a5 5 0 0 1 6.6 0 M12 19.5h.01",
    battery: "M4.5 7h13a2.5 2.5 0 0 1 2.5 2.5v5a2.5 2.5 0 0 1 -2.5 2.5h-13a2.5 2.5 0 0 1 -2.5 -2.5v-5a2.5 2.5 0 0 1 2.5 -2.5Z M22.5 10.5v3 M5.5 10.5v3 M9 10.5v3 M12.5 10.5v3",
    volume: "M11 5L6 9H3v6h3l5 4z M15.5 8.5a5 5 0 0 1 0 7 M18.5 5.5a9 9 0 0 1 0 13",
    search: "M4 11a7 7 0 1 0 14 0a7 7 0 1 0 -14 0Z M20 20l-4-4",
    grid: "M5.5 4h3a1.5 1.5 0 0 1 1.5 1.5v3a1.5 1.5 0 0 1 -1.5 1.5h-3a1.5 1.5 0 0 1 -1.5 -1.5v-3a1.5 1.5 0 0 1 1.5 -1.5Z M15.5 4h3a1.5 1.5 0 0 1 1.5 1.5v3a1.5 1.5 0 0 1 -1.5 1.5h-3a1.5 1.5 0 0 1 -1.5 -1.5v-3a1.5 1.5 0 0 1 1.5 -1.5Z M5.5 14h3a1.5 1.5 0 0 1 1.5 1.5v3a1.5 1.5 0 0 1 -1.5 1.5h-3a1.5 1.5 0 0 1 -1.5 -1.5v-3a1.5 1.5 0 0 1 1.5 -1.5Z M15.5 14h3a1.5 1.5 0 0 1 1.5 1.5v3a1.5 1.5 0 0 1 -1.5 1.5h-3a1.5 1.5 0 0 1 -1.5 -1.5v-3a1.5 1.5 0 0 1 1.5 -1.5Z",
    bell: "M6 9a6 6 0 0 1 12 0c0 6 2.5 8 2.5 8h-17S6 15 6 9z M10 20.5a2 2 0 0 0 4 0",
    close: "M7 7l10 10 M17 7L7 17",
    minimize: "M6 12h12",
    maximize: "M7 5h10a2 2 0 0 1 2 2v10a2 2 0 0 1 -2 2h-10a2 2 0 0 1 -2 -2v-10a2 2 0 0 1 2 -2Z",
    chevronRight: "M9 6l6 6-6 6",
    power: "M12 3v8 M6.3 6.6a8 8 0 1 0 11.4 0",
    lock: "M7.5 11h9a2.5 2.5 0 0 1 2.5 2.5v5a2.5 2.5 0 0 1 -2.5 2.5h-9a2.5 2.5 0 0 1 -2.5 -2.5v-5a2.5 2.5 0 0 1 2.5 -2.5Z M8 11V8a4 4 0 0 1 8 0v3",
};

// Uygulama adı -> simge, degrade renkleri (üst, alt), koyu simge rengi gerekip gerekmediği, kısa ad
var apps = {
    "Terminal": { icon: "terminal", top: "#45454A", bottom: "#1C1C1E", darkGlyph: false, shortName: "Terminal" },
    "Dosyalar": { icon: "folder", top: "#62B2FA", bottom: "#2C78DB", darkGlyph: false, shortName: "Dosyalar" },
    "Tarayıcı": { icon: "globe", top: "#FFA36E", bottom: "#E5583A", darkGlyph: false, shortName: "Tarayıcı" },
    "Ayarlar": { icon: "sliders", top: "#A3A8AE", bottom: "#5E636A", darkGlyph: false, shortName: "Ayarlar" },
    "Metin Düzenleyici": { icon: "pencil", top: "#72D6A3", bottom: "#2A9D68", darkGlyph: false, shortName: "Metin" },
    "Müzik": { icon: "music", top: "#FF7590", bottom: "#DC2D58", darkGlyph: false, shortName: "Müzik" },
    "Fotoğraflar": { icon: "image", top: "#FFD976", bottom: "#F29B1F", darkGlyph: true, shortName: "Fotoğraflar" },
    "Takvim": { icon: "calendar", top: "#FF8080", bottom: "#DD4343", darkGlyph: false, shortName: "Takvim" },
    "Hesap Makinesi": { icon: "calculator", top: "#9583F2", bottom: "#5944D3", darkGlyph: false, shortName: "Hesap M." },
    "Sistem İzleyici": { icon: "activity", top: "#63DAD3", bottom: "#1E9D96", darkGlyph: false, shortName: "İzleyici" },
    "Notlar": { icon: "note", top: "#FFE891", bottom: "#F0C541", darkGlyph: true, shortName: "Notlar" },
    "Paket Merkezi": { icon: "package", top: "#C9A1FF", bottom: "#8A57E3", darkGlyph: false, shortName: "Paketler" },
};

var fallbackApp = { icon: "package", top: "#A3A8AE", bottom: "#5E636A", darkGlyph: false, shortName: "" };
