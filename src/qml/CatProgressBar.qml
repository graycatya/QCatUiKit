// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QCatUiKit
import QtQuick.Templates as T

T.ProgressBar {
    id: control
    property ThemePalette theme: Theme
    readonly property var tokens: (theme || Theme).tokens
    implicitWidth: 200
    implicitHeight: 8
    Accessible.name: qsTr("Progress")
    background: Rectangle {
        color: control.tokens.disabled
        radius: height / 2
    }
    contentItem: Item {
        clip: true
        Rectangle {
            id: bar
            width: control.indeterminate ? parent.width / 3 : parent.width * control.position
            height: parent.height
            radius: height / 2
            color: control.tokens.primary
            x: control.mirrored && !control.indeterminate ? parent.width - width : 0
            SequentialAnimation on x {
                running: control.indeterminate && control.visible && !(control.theme || Theme).reducedMotion
                loops: Animation.Infinite
                NumberAnimation {
                    from: -bar.width
                    to: control.availableWidth
                    duration: 1000
                }
            }
        }
    }
}
