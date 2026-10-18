// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QCatUiKit

Rectangle {
    id: thumb
    property ThemePalette theme: Theme
    readonly property var tokens: (theme || Theme).tokens
    property real diameter: 22
    property bool down: false
    property bool highlighted: false
    property bool focused: activeFocus
    property bool tipVisible: false
    property string tipText
    implicitWidth: Math.max(12, diameter)
    implicitHeight: implicitWidth
    radius: width / 2
    antialiasing: true
    color: enabled ? tokens.surface : tokens.disabled
    border.width: focused ? 2 : 1
    border.color: focused ? tokens.primary : tokens.border
    Rectangle {
        anchors.centerIn: parent
        width: thumb.width * 6 / 11
        height: width
        radius: width / 2
        antialiasing: true
        scale: thumb.down ? 0.8 : thumb.highlighted ? 1.15 : 1
        color: !thumb.enabled ? thumb.tokens.textDisabled : thumb.down ? thumb.tokens.primaryPressed
            : thumb.highlighted ? thumb.tokens.primaryHover : thumb.tokens.primary
        Behavior on scale { NumberAnimation { duration: thumb.tokens.motionDuration } }
    }
    CatToolTip {
        theme: thumb.theme
        parent: thumb
        visible: thumb.enabled && thumb.tipVisible
        text: thumb.tipText
        delay: 0
        timeout: -1
        x: (thumb.width - width) / 2
        y: -height - thumb.tokens.spacing / 2
    }
}
