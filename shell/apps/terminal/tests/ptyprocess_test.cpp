// PtyProcess'in birim testleri: kabuğun sözde terminalde başlatılması, girdi ve çıktı, çalışma
// klasörünün bulunması (sekme başlığı) ve kabuk kapanınca finished.
#include "../ptyprocess.h"

#include <QDir>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>

class PtyProcessTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase()
    {
        // Kullanıcının kabuk ayarları (istem, renkler) testi etkilemesin
        qputenv("SHELL", "/bin/sh");
        qputenv("ENV", "");
    }

    void runsCommands()
    {
        PtyProcess pty;
        QByteArray output;
        connect(&pty, &PtyProcess::received, this, [&](const QByteArray &data) { output += data; });
        QVERIFY(pty.start(24, 80));
        pty.write("echo ankaros-$((40+2))\n", 23);
        QTRY_VERIFY_WITH_TIMEOUT(output.contains("ankaros-42"), 5000);
    }

    void findsWorkingDirectory()
    {
        QTemporaryDir directory;
        PtyProcess pty;
        QByteArray output;
        connect(&pty, &PtyProcess::received, this, [&](const QByteArray &data) { output += data; });
        QVERIFY(pty.start(24, 80));
        const QByteArray command = "cd " + directory.path().toLocal8Bit() + " && echo hazir\n";
        pty.write(command.constData(), size_t(command.size()));
        QTRY_VERIFY_WITH_TIMEOUT(output.contains("hazir\r\n"), 5000);
        QTRY_COMPARE_WITH_TIMEOUT(QDir(pty.workingDirectory()).canonicalPath(),
                                  QDir(directory.path()).canonicalPath(), 5000);
    }

    void finishesWhenShellExits()
    {
        PtyProcess pty;
        QSignalSpy finished(&pty, &PtyProcess::finished);
        QVERIFY(pty.start(24, 80));
        pty.write("exit\n", 5);
        QVERIFY(finished.wait(5000));
    }
};

QTEST_GUILESS_MAIN(PtyProcessTest)
#include "ptyprocess_test.moc"
