// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Shapes
import QtQuick.Templates as T
import QCatUiKit

T.CheckBox {
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
        radius: Math.min(4, control.tokens.radius)
        antialiasing: true
        color: !control.enabled ? control.tokens.disabled : control.checkState !== Qt.Unchecked ? (
                                                                                                      control.down
                                                                                                      ? control.tokens.primaryPressed :
                                                                                                        control.hovered
                                                                                                        ? control.tokens.primaryHover :
                                                                                                          control.tokens.primary) :
                                                                                                  control.down
                                                                                                  || control.hovered
                                                                                                  ? control.tokens.surfaceHover :
                                                                                                    control.tokens.surface
        border.color: control.enabled && control.checkState !== Qt.Unchecked ? color :
                                                                               control.tokens.border

        Shape {
            anchors.fill: parent
            visible: control.checkState === Qt.Checked
            preferredRendererType: Shape.CurveRenderer
            ShapePath {
                strokeColor: control.enabled ? control.tokens.onPrimary :
                                               control.tokens.textDisabled
                strokeWidth: 2
                fillColor: "transparent"
                capStyle: ShapePath.RoundCap
                joinStyle: ShapePath.RoundJoin
                startX: 5
                startY: 10
                PathLine {
                    x: 8.5
                    y: 13.5
                }
                PathLine {
                    x: 15
                    y: 6.5
                }
            }
        }
        Rectangle {
            anchors.centerIn: parent
            width: 10
            height: 2
            radius: 1
            visible: control.checkState === Qt.PartiallyChecked
            color: control.enabled ? control.tokens.onPrimary : control.tokens.textDisabled
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
