#pragma once

#include <QObject>
#include <QPointF>
#include <QtQml/qqmlregistration.h>

// Uygulamaya gitmeden önce yakalanması gereken girdiler. QML Shortcut'larının yapamadığı iki şey
// var: tek başına basılıp bırakılan Süper tuşu ve Alt bırakılana kadar süren Alt+Tab geçişi. Basit
// kısayollar (Süper+D, Alt+F4...) Main.qml'de Shortcut olarak duruyor. Fareye basılınca da haber
// verir: tıklanan pencere öne gelsin diye. Uygulamanın yüzeyi tıklamayı aldığında pencere
// çerçevesindeki tutucular bunu her zaman göremiyor.
class GlobalKeys : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    // Giriş ve kilit ekranında kapalı
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    // Alt+Tab ile pencere geçişi sürüyor mu (Alt basılı)
    Q_PROPERTY(bool switching READ switching NOTIFY switchingChanged)

public:
    explicit GlobalKeys(QObject *parent = nullptr);

    bool enabled() const { return m_enabled; }
    void setEnabled(bool enabled);
    bool switching() const { return m_switching; }

signals:
    void enabledChanged();
    void switchingChanged();
    // Süper tuşuna başka bir tuşla birlikte kullanılmadan basılıp bırakıldı
    void superTapped();
    // Alt+Tab (backward: Alt+Shift+Tab). İlk basışta switching true olur.
    void switchRequested(bool backward);
    // Alt bırakıldı: seçili pencere öne gelsin
    void switchCommitted();
    // Geçiş sırasında Esc: hiçbir şey değişmesin
    void switchCancelled();
    // Fareye basıldı (ekran koordinatında), olay pencerelere ulaşmadan önce. Giriş ve kilit
    // ekranında da gönderilir, kullanan karar verir.
    void pointerPressed(QPointF position);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void setSwitching(bool switching);

    bool m_enabled = true;
    bool m_switching = false;
    bool m_superDown = false;
    // Süper basılıyken başka bir tuşa ya da fareye basıldıysa bırakınca başlat paneli açılmaz
    bool m_superUsed = false;
};
