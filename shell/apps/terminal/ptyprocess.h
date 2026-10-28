#pragma once

#include <QByteArray>
#include <QObject>
#include <QString>

class QSocketNotifier;

// Sözde terminal (pty) üzerinde çalışan kabuk. Kullanıcının kabuğunu giriş kabuğu olarak başlatır,
// çıktısını received ile verir, girdiyi write ile yazar. Kabuk kapanınca finished yayınlanır.
class PtyProcess : public QObject
{
    Q_OBJECT

public:
    explicit PtyProcess(QObject *parent = nullptr);
    ~PtyProcess() override;

    bool start(int rows, int columns);
    void write(const char *data, size_t size);
    void resize(int rows, int columns);
    // Ön plandaki sürecin çalışma klasörü (sekme başlığı için), bilinmiyorsa boş
    QString workingDirectory() const;

signals:
    void received(const QByteArray &data);
    void finished();

private:
    void read();

    int m_fd = -1;
    int m_pid = -1;
    QSocketNotifier *m_notifier = nullptr;
};
