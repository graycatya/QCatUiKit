// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QCatUiKit
import QtQuick.Effects

MultiEffect {
    property ThemePalette theme: Theme
    readonly property var tokens: (theme || Theme).tokens
    shadowEnabled: !(theme || Theme).reducedMotion
    shadowColor: "#40000000"
    shadowBlur: 0.4
    shadowVerticalOffset: 2
}
