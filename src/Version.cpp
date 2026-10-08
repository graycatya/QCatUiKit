// SPDX-License-Identifier: Apache-2.0
#include "VersionData_p.h"
#include <QCatUiKit/Version.h>

namespace QCatUiKit
{
QVersionNumber version()
{
    return QVersionNumber(Build::versionMajor, Build::versionMinor, Build::versionPatch);
}

QString versionString()
{
    return QString::fromLatin1(Build::versionString);
}

ReleaseChannel releaseChannel() noexcept
{
    return Build::experimental ? ReleaseChannel::Experimental : ReleaseChannel::Stable;
}

bool isStableRelease() noexcept
{
    return releaseChannel() == ReleaseChannel::Stable;
}

bool isExperimentalRelease() noexcept
{
    return releaseChannel() == ReleaseChannel::Experimental;
}

int abiVersion() noexcept
{
    return Build::abiVersion;
}
} // namespace QCatUiKit
