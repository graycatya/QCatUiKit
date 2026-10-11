// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Templates as T
import QCatUiKit

T.SpinBox {
    id: control
    property ThemePalette theme: Theme
    readonly property var tokens: (theme || Theme).tokens
    implicitWidth: 180
    implicitHeight: tokens.controlHeight
    padding: 4
    leftPadding: tokens.controlHeight + 4
    rightPadding: tokens.controlHeight + 4
    font.pixelSize: tokens.fontSize
    hoverEnabled: tokens.hoverEnabled
    validator: IntValidator {
        bottom: Math.min(control.from, control.to)
        top: Math.max(control.from, control.to)
    }
    inputMethodHints: Qt.ImhFormattedNumbersOnly
    contentItem: CatTextField {
        theme: control.theme
        text: control.textFromValue(control.value, control.locale)
        font: control.font
        readOnly: !control.editable
        validator: control.validator
        inputMethodHints: control.inputMethodHints
        padding: 0
        background: null
        horizontalAlignment: TextInput.AlignHCenter
        verticalAlignment: TextInput.AlignVCenter
    }
    up.indicator: Rectangle {
        implicitWidth: control.tokens.controlHeight
        implicitHeight: control.tokens.controlHeight
        x: control.mirrored ? 0 : control.width - width
        height: control.height
        radius: control.tokens.radius
        color: control.up.pressed || control.up.hovered ? control.tokens.surfaceHover :
                                                          "transparent"

        Text {
            anchors.centerIn: parent
            text: "+"
            font.pixelSize: 20
            color: control.up.indicator.enabled ? control.tokens.text : control.tokens.textDisabled
        }
    }
    down.indicator: Rectangle {
        implicitWidth: control.tokens.controlHeight
        implicitHeight: control.tokens.controlHeight
        x: control.mirrored ? control.width - width : 0
        height: control.height
        radius: control.tokens.radius
        color: control.down.pressed || control.down.hovered ? control.tokens.surfaceHover :
                                                              "transparent"

        Text {
            anchors.centerIn: parent
            text: "−"
            font.pixelSize: 20
            color: control.down.indicator.enabled ? control.tokens.text :
                                                    control.tokens.textDisabled
        }
    }
    background: SurfaceImpl {
        theme: control.theme
        focused: control.activeFocus
        hovered: control.hovered
        opacity: control.enabled ? 1 : 0.6
    }
}
