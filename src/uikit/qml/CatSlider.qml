// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Templates as T
import QCatUiKit

T.Slider {
    id: control
    property ThemePalette theme: Theme
    readonly property var tokens: (theme || Theme).tokens
    implicitWidth: horizontal ? 200 : Math.max(tokens.controlHeight, implicitHandleWidth
                                               + leftPadding + rightPadding)
    implicitHeight: horizontal ? Math.max(tokens.controlHeight, implicitHandleHeight + topPadding
                                          + bottomPadding) : 200
    padding: tokens.padding / 2
    hoverEnabled: tokens.hoverEnabled
    focusPolicy: Qt.StrongFocus
    background: Rectangle {
        x: control.horizontal ? control.leftPadding + control.handle.width / 2 : (control.width
                                                                                  - width) / 2
        y: control.horizontal ? (control.height - height) / 2 : control.topPadding
                                + control.handle.height / 2
        width: control.horizontal ? Math.max(0, control.availableWidth - control.handle.width) : 4
        height: control.horizontal ? 4 : Math.max(0, control.availableHeight
                                                  - control.handle.height)
        radius: 2
        color: control.enabled ? control.tokens.border : control.tokens.disabled
        Rectangle {
            x: control.horizontal && control.mirrored ? parent.width * (1 - control.position) : 0
            y: control.horizontal ? 0 : parent.height * control.visualPosition
            width: control.horizontal ? parent.width * control.position : parent.width
            height: control.horizontal ? parent.height : parent.height * (1
                                                                          - control.visualPosition)
            radius: 2
            color: control.enabled ? control.tokens.primary : control.tokens.textDisabled
        }
    }
    handle: Rectangle {
        implicitWidth: 22
        implicitHeight: 22
        x: control.horizontal ? control.leftPadding + control.visualPosition * (
                                    control.availableWidth - width) : (control.width - width) / 2
        y: control.horizontal ? (control.height - height) / 2 : control.topPadding
                                + control.visualPosition * (control.availableHeight - height)
        radius: width / 2
        antialiasing: true
        color: control.enabled ? control.tokens.surface : control.tokens.disabled
        border.width: control.visualFocus ? 2 : 1
        border.color: control.visualFocus ? control.tokens.primary : control.tokens.border
        Rectangle {
            anchors.centerIn: parent
            width: 12
            height: 12
            radius: 6
            antialiasing: true
            scale: control.pressed ? 0.8 : control.hovered ? 1.15 : 1
            color: !control.enabled ? control.tokens.textDisabled : control.pressed ? control.tokens.primaryPressed :
                                                                                      control.hovered
                                                                                      ? control.tokens.primaryHover :
                                                                                        control.tokens.primary
            Behavior on scale {
                NumberAnimation {
                    duration: control.tokens.motionDuration
                }
            }
        }
    }
}
