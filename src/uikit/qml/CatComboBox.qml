// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Window
import QtQuick.Shapes
import QtQuick.Templates as T
import QCatUiKit

T.ComboBox {
    id: control
    property ThemePalette theme: Theme
    readonly property var tokens: (theme || Theme).tokens
    readonly property real windowHeight: Window.height
    implicitWidth: 180
    implicitHeight: Math.max(tokens.controlHeight, implicitContentHeight + topPadding
                             + bottomPadding)
    padding: tokens.padding
    topPadding: 6
    bottomPadding: 6
    spacing: tokens.spacing
    leftPadding: padding + (mirrored ? indicator.width + spacing : 0)
    rightPadding: padding + (mirrored ? 0 : indicator.width + spacing)
    font.pixelSize: tokens.fontSize
    hoverEnabled: tokens.hoverEnabled
    focusPolicy: Qt.StrongFocus
    Accessible.name: displayText
    indicator: Shape {
        width: 16
        height: 16
        x: control.mirrored ? control.padding : control.width - width - control.padding
        y: (control.height - height) / 2
        preferredRendererType: Shape.CurveRenderer
        ShapePath {
            strokeColor: control.enabled ? control.tokens.textSecondary :
                                           control.tokens.textDisabled
            strokeWidth: 1.5
            fillColor: "transparent"
            capStyle: ShapePath.RoundCap
            joinStyle: ShapePath.RoundJoin
            startX: 4
            startY: 6
            PathLine {
                x: 8
                y: 10
            }
            PathLine {
                x: 12
                y: 6
            }
        }
    }
    contentItem: CatTextField {
        theme: control.theme
        text: control.editable ? control.editText : control.displayText
        font: control.font
        enabled: control.editable
        color: control.enabled ? control.tokens.text : control.tokens.textDisabled
        autoScroll: control.editable
        readOnly: control.down
        inputMethodHints: control.inputMethodHints
        validator: control.validator
        selectByMouse: control.selectTextByMouse
        verticalAlignment: TextInput.AlignVCenter
        horizontalAlignment: control.mirrored ? TextInput.AlignRight : TextInput.AlignLeft
        padding: 0
        leftPadding: 0
        rightPadding: 0
        topPadding: 0
        bottomPadding: 0
        implicitHeight: contentHeight
        background: null
    }
    background: Rectangle {
        radius: control.tokens.radius
        color: !control.enabled ? control.tokens.disabled : control.down || control.hovered
                                  ? control.tokens.surfaceHover : control.tokens.surface
        border.width: control.visualFocus || (control.editable && control.activeFocus) ? 2 : 1
        border.color: control.visualFocus || (control.editable && control.activeFocus)
                      ? control.tokens.primary : control.tokens.border
        Behavior on color {
            ColorAnimation {
                duration: control.tokens.motionDuration
            }
        }
    }
    delegate: T.ItemDelegate {
        id: entry
        required property int index
        width: list.width
        implicitHeight: Math.max(control.tokens.controlHeight, implicitContentHeight + topPadding
                                 + bottomPadding)
        padding: control.tokens.padding
        text: control.textAt(index)
        highlighted: control.highlightedIndex === index
        hoverEnabled: control.hoverEnabled
        font: control.font
        Accessible.name: text
        contentItem: Text {
            text: entry.text
            font: entry.font
            color: control.tokens.text
            textFormat: Text.PlainText
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: control.mirrored ? Text.AlignRight : Text.AlignLeft
            elide: Text.ElideRight
        }
        background: Rectangle {
            radius: Math.min(4, control.tokens.radius)
            color: entry.highlighted || entry.hovered ? control.tokens.surfaceHover :
                                                        control.tokens.surface
            Rectangle {
                width: 3
                height: Math.max(0, parent.height - 16)
                x: control.mirrored ? parent.width - width : 0
                anchors.verticalCenter: parent.verticalCenter
                radius: 1.5
                color: control.tokens.primary
                visible: entry.index === control.currentIndex
            }
        }
    }
    popup: CatPopup {
        theme: control.theme
        y: control.height + 4
        width: control.width
        height: Math.min(320, list.contentHeight + topPadding + bottomPadding, Math.max(0,
                                                                                        control.windowHeight
                                                                                        - topMargin
                                                                                        - bottomMargin))
        topMargin: 8
        bottomMargin: 8
        padding: 4
        modal: false
        focus: false
        contentItem: ListView {
            id: list
            clip: true
            model: control.delegateModel
            currentIndex: control.highlightedIndex
            highlightMoveDuration: 0
            boundsBehavior: Flickable.StopAtBounds
            T.ScrollIndicator.vertical: T.ScrollIndicator {
                id: scrollIndicator
                implicitWidth: 3
                implicitHeight: 20
                contentItem: Rectangle {
                    implicitWidth: 3
                    implicitHeight: 20
                    radius: 1.5
                    color: control.tokens.textSecondary
                    opacity: scrollIndicator.active ? 0.6 : 0
                }
            }
        }
    }
}
