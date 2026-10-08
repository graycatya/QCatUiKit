// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QCatUiKit
import QtQuick.Templates as T

T.Button {
    id: control
    property ThemePalette theme: Theme
    readonly property var tokens: (theme || Theme).tokens
    property bool primary: false
    property bool danger: false
    property bool loading: false
    property url iconSource
    Binding {
        target: control
        property: "enabled"
        value: false
        when: control.loading
        restoreMode: Binding.RestoreBindingOrValue
    }
    hoverEnabled: tokens.hoverEnabled
    focusPolicy: Qt.StrongFocus
    padding: tokens.padding
    implicitWidth: Math.max(80, implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(tokens.controlHeight, implicitContentHeight + 8)
    font.pixelSize: tokens.fontSize
    Accessible.name: text
    contentItem: Item {
        implicitWidth: contentRow.implicitWidth
        implicitHeight: contentRow.implicitHeight
        Row {
            id: contentRow
            anchors.centerIn: parent
            spacing: 8
            layoutDirection: control.mirrored ? Qt.RightToLeft : Qt.LeftToRight
            CatBusyIndicator {
                theme: control.theme
                width: 18
                height: 18
                anchors.verticalCenter: parent.verticalCenter
                visible: control.loading
                running: control.loading
            }
            Image {
                source: control.iconSource
                visible: source.toString().length > 0 && !control.loading
                width: 18
                height: 18
                anchors.verticalCenter: parent.verticalCenter
                fillMode: Image.PreserveAspectFit
            }
            Text {
                text: control.text
                font: control.font
                textFormat: Text.PlainText
                anchors.verticalCenter: parent.verticalCenter
                color: !control.enabled ? control.tokens.textDisabled : control.primary ? control.tokens.onPrimary : control.danger ? control.tokens.danger : control.tokens.text
            }
        }
    }
    background: Rectangle {
        radius: control.tokens.radius
        color: !control.enabled ? control.tokens.disabled : control.primary ? (control.down ? control.tokens.primaryPressed : control.hovered ? control.tokens.primaryHover : control.tokens.primary) : control.down || control.checked || control.hovered ? control.tokens.surfaceHover : control.tokens.surface
        border.width: control.visualFocus ? 2 : 1
        border.color: control.visualFocus ? control.tokens.primary : control.danger ? control.tokens.danger : control.tokens.border
        Behavior on color {
            ColorAnimation {
                duration: control.tokens.motionDuration
            }
        }
    }
}
