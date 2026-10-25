// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <QtCore/qglobal.h>
#if defined(QCATUIKIT_BUILD)
#define QCATUIKIT_EXPORT Q_DECL_EXPORT
#else
#define QCATUIKIT_EXPORT Q_DECL_IMPORT
#endif
