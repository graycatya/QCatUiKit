// SPDX-License-Identifier: Apache-2.0
#include <QCatUiKit/Version.h>
#include <QCoreApplication>
#include <QDebug>

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    const QString channel =
        QCatUiKit::isStableRelease() ? QStringLiteral("stable") : QStringLiteral("experimental");
    qInfo().noquote() << QCatUiKit::versionString() << channel << "ABI" << QCatUiKit::abiVersion();
    if (argc != 3)
    {
        qCritical() << "Usage: version-consumer EXPECTED_VERSION EXPECTED_CHANNEL";
        return 2;
    }
    return QCatUiKit::versionString() == QString::fromLocal8Bit(argv[1]) &&
                   channel == QString::fromLocal8Bit(argv[2])
               ? 0
               : 1;
}
