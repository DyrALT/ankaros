#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

#include <functional>

// ankaros servislerinin denetim soketlerine (system/common/protocol.h) tek seferlik istek.
// Cevap olay döngüsünde gelir. QtNetwork gerekmesin diye POSIX soket ve QSocketNotifier kullanılır.
namespace SocketRequest {

using Reply = std::function<void(const QStringList &lines, const QString &error)>;

void send(QObject *context, const char *socketPath, const QStringList &command, Reply reply = {});

// Cevabı satırlarına ayırır: veri satırları lines'a, hata mesajı döner (başarılıysa boş)
QString parse(const QByteArray &response, QStringList &lines);

} // namespace SocketRequest
