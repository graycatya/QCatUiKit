// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QCatUiKit

Item {
    id: control
    property ThemePalette theme: Theme
    readonly property var tokens: (theme || Theme).tokens
    property string text
    property bool running: true
    property int duration: 6000
    property alias font: label.font
    clip: true
    implicitWidth: 200
    implicitHeight: label.implicitHeight
    CatLabel {
        id: label
        theme: control.theme
        text: control.text
    }
    SequentialAnimation {
        running: control.running && control.visible && label.width > control.width && !(control.theme || Theme).reducedMotion
        loops: Animation.Infinite
        PauseAnimation {
            duration: 800
        }
        NumberAnimation {
            target: label
            property: "x"
            from: 0
            to: Math.min(0, control.width - label.width)
            duration: Math.max(1, control.duration)
        }
        PauseAnimation {
            duration: 800
        }
        NumberAnimation {
            target: label
            property: "x"
            to: 0
            duration: 0
        }
        onRunningChanged: if (!running)
            label.x = 0
    }
}
