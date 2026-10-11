// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QCatUiKit
import QtQuick

Rectangle {
    id: control
    property ThemePalette theme: Theme
    readonly property var tokens: (theme || Theme).tokens
    property int value: -1
    property string iconGlyph
    property int severity: 0
    property bool showZero: true
    implicitWidth: value < 0 && !iconGlyph.length ? 8 : Math.max(20, badgeText.implicitWidth + 12)
    implicitHeight: value < 0 && !iconGlyph.length ? 8 : 20
    visible: showZero || value !== 0
    radius: height / 2
    color: severity === 1 ? tokens.success : severity === 2 ? tokens.warning : severity === 3
                                                              ? tokens.danger : tokens.primary
    Accessible.name: badgeText.text
    Accessible.role: Accessible.Indicator
    Text {
        id: badgeText
        anchors.centerIn: parent
        text: control.iconGlyph || (control.value < 0 ? "" : control.value > 99 ? "99+" : String(
                                                                                      control.value))
        color: control.tokens.onPrimary
        font.pixelSize: 12
        textFormat: Text.PlainText
    }
}
