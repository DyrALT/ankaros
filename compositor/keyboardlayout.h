#pragma once

#include <QObject>
#include <QString>
#include <QtQml/qqmlregistration.h>

struct xkb_context;
struct xkb_keymap;
struct xkb_state;

// Klavye düzeninin compositor'ın kendi arayüzüne (başlat panelinde arama, kilit ekranında şifre)
// anında uygulanması. Uygulamalar düzeni Wayland klavye haritasından alır (Main.qml, seat keymap),
// ama compositor'ın kendisi tuşları açılışta XKB_DEFAULT_LAYOUT'la çevirir ve Qt bunu çalışırken
// değiştirmenin yolunu vermez. Seçilen düzen açılıştakinden farklıysa, odak compositor'ın kendi
// arayüzündeyken tuşlar xkbcommon ile seçilen düzene göre yeniden çevrilir.
class KeyboardLayout : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    // "tr", "tr-f" ya da "us" (Appearance.keyboardLayout)
    Q_PROPERTY(QString layout READ layout WRITE setLayout NOTIFY layoutChanged)
    // Wayland klavye haritası için düzen ve değişke (ör. "tr" ve "f")
    Q_PROPERTY(QString xkbLayout READ xkbLayout NOTIFY layoutChanged)
    Q_PROPERTY(QString xkbVariant READ xkbVariant NOTIFY layoutChanged)

public:
    explicit KeyboardLayout(QObject *parent = nullptr);
    ~KeyboardLayout() override;

    QString layout() const { return m_layout; }
    void setLayout(const QString &layout);
    QString xkbLayout() const;
    QString xkbVariant() const;

signals:
    void layoutChanged();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void freeKeymap();

    QString m_layout;
    // Compositor açılırken kullanılan düzen ("tr" ya da "tr-f"), ortam değişkenlerinden
    QString m_startupLayout;
    xkb_context *m_context = nullptr;
    xkb_keymap *m_keymap = nullptr;
    xkb_state *m_state = nullptr;
    // Çevrilmiş olayı gönderirken süzgeç kendine takılmasın
    bool m_sending = false;
};
