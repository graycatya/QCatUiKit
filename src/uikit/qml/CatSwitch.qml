// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Templates as T
import QCatUiKit

T.Switch {
    id: control
    property ThemePalette theme: Theme
    readonly property var tokens: (theme || Theme).tokens
    implicitWidth: Math.max(implicitIndicatorWidth, implicitContentWidth) + leftPadding
                   + rightPadding
    implicitHeight: Math.max(tokens.controlHeight, implicitContentHeight + topPadding
                             + bottomPadding, implicitIndicatorHeight + topPadding + bottomPadding)
    padding: tokens.padding / 2
    spacing: tokens.spacing
    font.pixelSize: tokens.fontSize
    hoverEnabled: tokens.hoverEnabled
    focusPolicy: Qt.StrongFocus
    Accessible.name: text
    indicator: Rectangle {
        implicitWidth: 42
        implicitHeight: 22
        x: control.text.length ? (control.mirrored ? control.width - width - control.rightPadding :
                                                     control.leftPadding) : (control.width - width)
                                 / 2
        y: (control.height - height) / 2
        radius: height / 2
        antialiasing: true
        color: !control.enabled ? control.tokens.disabled : control.checked ? (control.down
                                                                               ? control.tokens.primaryPressed :
                                                                                 control.hovered
                                                                                 ? control.tokens.primaryHover :
                                                                                   control.tokens.primary) :
                                                                              control.down
                                                                              || control.hovered
                                                                              ? control.tokens.surfaceHover :
                                                                                control.tokens.surface
        border.color: control.enabled && control.checked ? color : control.tokens.border
        Rectangle {
            width: 16
            height: 16
            radius: 8
            antialiasing: true
            x: 3 + control.visualPosition * (parent.width - width - 6)
            y: (parent.height - height) / 2
            color: !control.enabled ? control.tokens.textDisabled : control.checked ? control.tokens.onPrimary :
                                                                                      control.tokens.textSecondary
            scale: control.down ? 0.85 : control.hovered ? 1.08 : 1
            Behavior on x {
                enabled: !control.down
                NumberAnimation {
                    duration: control.tokens.motionDuration
                }
            }
            Behavior on scale {
                NumberAnimation {
                    duration: control.tokens.motionDuration
                }
            }
        }
        Behavior on color {
            ColorAnimation {
                duration: control.tokens.motionDuration
            }
        }
    }
    contentItem: Text {
        text: control.text
        font: control.font
        textFormat: Text.PlainText
        color: control.enabled ? control.tokens.text : control.tokens.textDisabled
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: control.mirrored ? Text.AlignRight : Text.AlignLeft
        leftPadding: control.text.length && !control.mirrored ? control.indicator.width + control.spacing :
                                                                0
        rightPadding: control.text.length && control.mirrored ? control.indicator.width + control.spacing :
                                                                0
        elide: Text.ElideRight
    }
    background: Rectangle {
        radius: control.tokens.radius
        color: "transparent"
        border.width: control.visualFocus ? 2 : 0
        border.color: control.tokens.primary
    }
}
