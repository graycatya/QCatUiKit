// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <QCatUiKit/qcatuikitglobal.h>
#include <QString>
#include <QVersionNumber>

namespace QCatUiKit
{
enum class ReleaseChannel
{
    Stable = 0,
    Experimental = 1
};

// All queries describe the loaded shared library, not the client's headers.
QCATUIKIT_EXPORT QVersionNumber version();
QCATUIKIT_EXPORT QString versionString();
QCATUIKIT_EXPORT ReleaseChannel releaseChannel() noexcept;
QCATUIKIT_EXPORT bool isStableRelease() noexcept;
QCATUIKIT_EXPORT bool isExperimentalRelease() noexcept;
QCATUIKIT_EXPORT int abiVersion() noexcept;
} // namespace QCatUiKit
