# 0006: Compositor için Qt Wayland Compositor ve eglfs

- **Durum:** Kabul edildi
- **Tarih:** 2026-10-28

## Bağlam

ankaros'un masaüstü bir Wayland compositor'ı gerektiriyor: uygulama pencerelerini alacak, ekrana
çizecek, girdiyi dağıtacak. Kabuk (Ankaros.Shell) QML ile yazılı ve aynı süreçte, aynı sahnede
çalışmalı.

## Değerlendirilen seçenekler

- **wlroots (C) + ayrı bir QML kabuk süreci:** Olgun ve hızlı, ama ekip tek dil olarak C++ seçti
  (0002) ve kabuk ile compositor ayrı süreçler olunca arada bir protokol (layer-shell vb.) gerekir.
- **Hazır bir compositor'ı (KWin, Weston) temel almak:** Hızlı başlangıç, ama "kendi masaüstümüz"
  hedefiyle çelişir ve özelleştirme sınırlı kalır.
- **Qt Wayland Compositor:** Compositor da QML ile yazılabilir, kabuk aynı sahnenin parçası olur.
  Pencereler sıradan Qt Quick öğeleri gibi animasyonlanabilir.

Qt'nin ekrana çizim yolu için: X11 üzerinde çalışmak (gereksiz katman), iç içe Wayland (sadece
geliştirme için) ve **eglfs** (DRM/KMS ile doğrudan ekran kartına, Mesa'nın GBM'si üzerinden).

## Karar

Qt Wayland Compositor (QML API) ve eglfs platform eklentisi (`QT_QPA_PLATFORM=eglfs`, KMS arka ucu).
Girdi libinput ile alınır.

## Sonuçlar

- Kabuk ve compositor tek bir süreç: başlat paneli, pencere animasyonları ve buzlu cam efekti pencere
  içeriklerini de kapsayabilir.
- Compositor yalnızca Linux'ta çalışır. Mac önizlemesi kabuğu compositor olmadan göstermeye devam eder,
  kabuk modülü bu yüzden Wayland türlerini doğrudan import etmez (bkz. mimari.md).
- Compositor çökerse bütün oturum gider. Kabuk kodunda hata yönetimine dikkat edilmeli.
