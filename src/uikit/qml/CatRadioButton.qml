// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Templates as T
import QCatUiKit

T.RadioButton {
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
        implicitWidth: 20
        implicitHeight: 20
        x: control.text.length ? (control.mirrored ? control.width - width - control.rightPadding :
                                                     control.leftPadding) : (control.width - width)
                                 / 2
        y: (control.height - height) / 2
        radius: width / 2
        antialiasing: true
        color: !control.enabled ? control.tokens.disabled : control.down || control.hovered
                                  ? control.tokens.surfaceHover : control.tokens.surface
        border.width: control.checked ? 2 : 1
        border.color: control.enabled && control.checked ? control.tokens.primary :
                                                           control.tokens.border

        Rectangle {
            anchors.centerIn: parent
            width: 10
            height: 10
            radius: 5
            antialiasing: true
            scale: control.checked ? 1 : 0
            color: !control.enabled ? control.tokens.textDisabled : control.down ? control.tokens.primaryPressed :
                                                                                   control.hovered
                                                                                   ? control.tokens.primaryHover :
                                                                                     control.tokens.primary
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
