#pragma once

#include "terminalscreen.h"

#include <QColor>
#include <QFont>
#include <QList>
#include <QQuickPaintedItem>
#include <QtQml/qqmlregistration.h>

#include <memory>

class PtyProcess;
class QTimer;

// Terminalin bir sekmesi: kabuğu pty'de başlatır, ekranını (TerminalScreen) çizer, klavye ve fareyi
// işler. Renkler temadan QML'de verilir (Main.qml), böylece açık ve koyu temada ayrı olur.
class TerminalView : public QQuickPaintedItem
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QString fontFamily READ fontFamily WRITE setFontFamily NOTIFY fontChanged)
    Q_PROPERTY(int fontSize READ fontSize WRITE setFontSize NOTIFY fontChanged)
    Q_PROPERTY(QColor foreground MEMBER m_foreground NOTIFY colorsChanged)
    Q_PROPERTY(QColor background MEMBER m_background NOTIFY colorsChanged)
    Q_PROPERTY(QColor cursorColor MEMBER m_cursorColor NOTIFY colorsChanged)
    Q_PROPERTY(QColor selectionColor MEMBER m_selectionColor NOTIFY colorsChanged)
    // 16 ANSI rengi: 0-7 normal, 8-15 parlak
    Q_PROPERTY(QList<QColor> ansiColors MEMBER m_palette NOTIFY colorsChanged)
    // Sekmenin başlığı: programın verdiği başlık ya da "kullanıcı@makine: ~" / klasörün adı
    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(bool hasSelection READ hasSelection NOTIFY selectionChanged)
    Q_PROPERTY(int columns READ columns NOTIFY sizeChanged)
    Q_PROPERTY(int rows READ rows NOTIFY sizeChanged)

public:
    explicit TerminalView(QQuickItem *parent = nullptr);
    ~TerminalView() override;

    QString fontFamily() const { return m_font.family(); }
    void setFontFamily(const QString &family);
    int fontSize() const { return m_font.pixelSize(); }
    void setFontSize(int size);
    QString title() const { return m_title; }
    bool hasSelection() const { return m_selectionStart != m_selectionEnd; }
    int columns() const { return m_screen ? m_screen->columns() : 0; }
    int rows() const { return m_screen ? m_screen->rows() : 0; }

    Q_INVOKABLE void copy();
    Q_INVOKABLE void paste();
    // Programın çıktısı gibi ekrana yazar (önizleme ve testler için)
    Q_INVOKABLE void feed(const QString &text);

    void paint(QPainter *painter) override;

signals:
    void fontChanged();
    void colorsChanged();
    void titleChanged();
    void selectionChanged();
    void sizeChanged();
    // Kabuk kapandı (exit), sekme kapanmalı
    void finished();

protected:
    void componentComplete() override;
    void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override;
    bool event(QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    void updateMetrics();
    void updateSize();
    void updateTitle();
    void scrollBy(int lines);
    // Ekrandaki noktaya karşılık gelen hücre, satır kaydırma geçmişinin atılan satırları da
    // sayılarak (seçim geçmiş kısaldığında kaymasın diye)
    TerminalScreen::Position cellAt(QPointF point) const;
    QColor color(const QColor &rgb, int index, bool foreground, bool bold) const;

    std::unique_ptr<TerminalScreen> m_screen;
    PtyProcess *m_pty = nullptr;
    QTimer *m_titleTimer = nullptr;
    QFont m_font;
    QFont m_boldFont;
    qreal m_cellWidth = 8;
    qreal m_cellHeight = 20;
    qreal m_ascent = 14;
    QColor m_foreground = Qt::black;
    QColor m_background = Qt::white;
    QColor m_cursorColor = Qt::black;
    QColor m_selectionColor = QColor(0, 0, 0, 40);
    QList<QColor> m_palette;
    QString m_title;
    // Kaç satır yukarı kaydırıldı (0: en altta)
    int m_scroll = 0;
    TerminalScreen::Position m_selectionStart;
    TerminalScreen::Position m_selectionEnd;
    bool m_selecting = false;
};
