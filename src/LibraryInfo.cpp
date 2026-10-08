// SPDX-License-Identifier: Apache-2.0
#include "LibraryInfo.h"
#include <QCatUiKit/Version.h>

namespace QCatUiKit
{
LibraryInfo::LibraryInfo(QObject *parent) : QObject(parent) {}

QString LibraryInfo::version() const
{
    return QCatUiKit::versionString();
}
int LibraryInfo::versionMajor() const
{
    return QCatUiKit::version().majorVersion();
}
int LibraryInfo::versionMinor() const
{
    return QCatUiKit::version().minorVersion();
}
int LibraryInfo::versionPatch() const
{
    return QCatUiKit::version().microVersion();
}
int LibraryInfo::abiVersion() const
{
    return QCatUiKit::abiVersion();
}
QString LibraryInfo::releaseChannel() const
{
    return QCatUiKit::isStableRelease() ? QStringLiteral("stable") : QStringLiteral("experimental");
}
bool LibraryInfo::isStable() const
{
    return QCatUiKit::isStableRelease();
}
bool LibraryInfo::isExperimental() const
{
    return QCatUiKit::isExperimentalRelease();
}
} // namespace QCatUiKit
