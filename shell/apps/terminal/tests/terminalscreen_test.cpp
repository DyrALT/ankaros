// TerminalScreen'in birim testleri: kabuk çıktısının ekrana yazılması, kaydırma geçmişi, seçim
// metni, renkler, başlık ve tuşların kabuğa giden baytları.
#include "../terminalscreen.h"

#include <QTest>

class TerminalScreenTest : public QObject
{
    Q_OBJECT

private slots:
    void writesText()
    {
        TerminalScreen screen(4, 20);
        screen.write("merhaba\r\ndünya");
        QCOMPARE(screen.lineText(0), QStringLiteral("merhaba"));
        QCOMPARE(screen.lineText(1), QStringLiteral("dünya"));
        QCOMPARE(screen.cursorRow(), 1);
        QCOMPARE(screen.cursorColumn(), 5);
    }

    void keepsScrollback()
    {
        TerminalScreen screen(3, 10, 4);
        for (int i = 1; i <= 8; ++i)
            screen.write(QStringLiteral("satır %1\r\n").arg(i).toUtf8());
        // 3 satırlık ekranda son satır boş, üstte 7 ve 8; 1-6 geçmişte ama sınır 4
        QCOMPARE(screen.scrollbackSize(), 4);
        QCOMPARE(screen.droppedLines(), 2);
        QCOMPARE(screen.lineText(0), QStringLiteral("satır 3"));
        QCOMPARE(screen.lineText(4), QStringLiteral("satır 7"));
        QCOMPARE(screen.lineText(5), QStringLiteral("satır 8"));
    }

    void restoresScrollbackOnGrow()
    {
        TerminalScreen screen(2, 10);
        screen.write("a\r\nb\r\nc");
        QCOMPARE(screen.scrollbackSize(), 1);
        screen.resize(3, 10);
        QCOMPARE(screen.scrollbackSize(), 0);
        QCOMPARE(screen.lineText(0), QStringLiteral("a"));
        QCOMPARE(screen.lineText(2), QStringLiteral("c"));
    }

    void selectsTextAcrossLines()
    {
        TerminalScreen screen(3, 20);
        screen.write("ilk satır   \r\nikinci satır");
        QCOMPARE(screen.text({0, 4}, {1, 6}), QStringLiteral("satır\nikinci"));
        // Ters yönde seçim de aynı metni verir
        QCOMPARE(screen.text({1, 6}, {0, 4}), QStringLiteral("satır\nikinci"));
    }

    void readsColors()
    {
        TerminalScreen screen(2, 20);
        screen.write("\x1b[1;31mK\x1b[0m\x1b[38;2;10;20;30mR\x1b[7mT");
        const TerminalScreen::Line line = screen.line(0);
        QCOMPARE(line[0].foregroundIndex, 1);
        QVERIFY(line[0].bold);
        QCOMPARE(line[1].foregroundIndex, -1);
        QCOMPARE(line[1].foreground, QColor(10, 20, 30));
        QVERIFY(line[2].reverse);
    }

    void wideCharacters()
    {
        TerminalScreen screen(2, 10);
        screen.write("漢x");
        const TerminalScreen::Line line = screen.line(0);
        QCOMPARE(line[0].width, 2);
        QCOMPARE(line[2].text, QStringLiteral("x"));
        QCOMPARE(screen.lineText(0), QStringLiteral("漢x"));
    }

    void title()
    {
        TerminalScreen screen(2, 10);
        int changes = 0;
        screen.titleChanged = [&] { ++changes; };
        screen.write("\x1b]2;derleme\x07");
        QCOMPARE(screen.title(), QStringLiteral("derleme"));
        QCOMPARE(changes, 1);
    }

    void alternateScreen()
    {
        TerminalScreen screen(2, 10);
        screen.write("\x1b[?1049h");
        QVERIFY(screen.alternateScreen());
        screen.write("\x1b[?1049l");
        QVERIFY(!screen.alternateScreen());
    }

    void keysBecomeBytes()
    {
        TerminalScreen screen(2, 10);
        QByteArray sent;
        screen.output = [&](const char *data, size_t size) { sent.append(data, qsizetype(size)); };
        screen.key(Qt::Key_C, Qt::ControlModifier, QStringLiteral("\x03"));
        screen.key(Qt::Key_Return, Qt::NoModifier, QStringLiteral("\r"));
        screen.key(Qt::Key_Up, Qt::NoModifier, {});
        screen.key(Qt::Key_unknown, Qt::NoModifier, QStringLiteral("ş"));
        QCOMPARE(sent, QByteArray("\x03\r\x1b[A") + QStringLiteral("ş").toUtf8());
    }

    void bracketedPaste()
    {
        TerminalScreen screen(2, 10);
        QByteArray sent;
        screen.output = [&](const char *data, size_t size) { sent.append(data, qsizetype(size)); };
        screen.paste(QStringLiteral("a\nb"));
        QCOMPARE(sent, QByteArray("a\rb"));
        sent.clear();
        // Kabuk "bracketed paste" istediyse yapıştırılan metin işaretlenir
        screen.write("\x1b[?2004h");
        screen.paste(QStringLiteral("ls"));
        QCOMPARE(sent, QByteArray("\x1b[200~ls\x1b[201~"));
    }
};

QTEST_GUILESS_MAIN(TerminalScreenTest)
#include "terminalscreen_test.moc"
