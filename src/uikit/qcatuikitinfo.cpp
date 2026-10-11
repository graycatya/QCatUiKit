// SPDX-License-Identifier: Apache-2.0
#include "qcatuikitinfo_p.h"
#include "qcatuikitversion.h"

namespace QCatUiKit
{
QCatUiKitInfo::QCatUiKitInfo(QObject *parent) : QObject(parent) {}

QString QCatUiKitInfo::version() const
{
    return QCatUiKit::versionString();
}
int QCatUiKitInfo::versionMajor() const
{
    return QCatUiKit::version().majorVersion();
}
int QCatUiKitInfo::versionMinor() const
{
    return QCatUiKit::version().minorVersion();
}
int QCatUiKitInfo::versionPatch() const
{
    return QCatUiKit::version().microVersion();
}
int QCatUiKitInfo::abiVersion() const
{
    return QCatUiKit::abiVersion();
}
QString QCatUiKitInfo::releaseChannel() const
{
    return QCatUiKit::isStableRelease() ? QStringLiteral("stable") : QStringLiteral("experimental");
}
bool QCatUiKitInfo::isStable() const
{
    return QCatUiKit::isStableRelease();
}
bool QCatUiKitInfo::isExperimental() const
{
    return QCatUiKit::isExperimentalRelease();
}
} // namespace QCatUiKit
