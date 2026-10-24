// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <QFontDatabase>
#include <QGuiApplication>

inline void initializeTestFonts()
{
    const auto families = QFontDatabase::families();
    if (families.isEmpty())
        qFatal("No fonts available: configure QCATUIKIT_TEST_FONT_DIR for CTest or "
               "QT_QPA_FONTDIR when running offscreen tests directly");

    QFont font = QGuiApplication::font();
    // Use the Windows UI font when available, including with the offscreen QPA.
    const QString family = families.contains(QStringLiteral("Segoe UI"))
                               ? QStringLiteral("Segoe UI")
                               : font.defaultFamily();
    if (!family.isEmpty())
        font.setFamilies({family});
    QGuiApplication::setFont(font);
}
