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
    property real animationPhase: 0
    SequentialAnimation on animationPhase {
        running: control.indeterminate && control.visible && control.enabled && !(control.theme
                                                                                  || Theme).reducedMotion
        loops: Animation.Infinite
        NumberAnimation {
            from: 0
            to: 1
            duration: 1000
        }
    }
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
            color: control.enabled ? control.tokens.primary : control.tokens.textDisabled
            x: {
                if (!control.indeterminate)
                    return control.mirrored ? parent.width - width : 0;
                if ((control.theme || Theme).reducedMotion || !control.enabled)
                    return (parent.width - width) / 2;
                const offset = -width + (parent.width + width) * control.animationPhase;
                return control.mirrored ? parent.width - width - offset : offset;
            }
        }
    }
}
