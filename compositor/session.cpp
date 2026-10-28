#include "session.h"

#include <QCoreApplication>

QString Session::userName() const
{
    return qEnvironmentVariable("USER", QStringLiteral("root"));
}

bool Session::greeter() const
{
    return QCoreApplication::arguments().contains(QLatin1String("--login"));
}
