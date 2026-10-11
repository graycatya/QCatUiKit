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
    spacing: tokens.spacing
    focusPolicy: Qt.StrongFocus
    padding: tokens.padding
    topPadding: 6
    bottomPadding: 6
    implicitWidth: Math.max(80, implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(tokens.controlHeight, implicitContentHeight + topPadding
                             + bottomPadding)
    font.pixelSize: tokens.fontSize
    Accessible.name: text
    contentItem: Item {
        implicitWidth: textLabel.implicitWidth + (control.loading || picture.visible ? 18 + (
                                                                                           control.text.length
                                                                                           ? control.spacing :
                                                                                             0) : 0)
        implicitHeight: contentRow.implicitHeight
        Row {
            id: contentRow
            anchors.centerIn: parent
            spacing: control.spacing
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
                id: picture
                source: control.iconSource.toString().length ? control.iconSource :
                                                               control.icon.source
                visible: source.toString().length > 0 && !control.loading
                width: 18
                height: 18
                anchors.verticalCenter: parent.verticalCenter
                fillMode: Image.PreserveAspectFit
            }
            Text {
                id: textLabel
                width: Math.min(implicitWidth, Math.max(0, control.availableWidth - (
                                                            control.loading
                                                            || control.iconSource.toString().length
                                                            || control.icon.source.toString(
                                                                ).length ? 18 + contentRow.spacing :
                                                                           0)))
                text: control.text
                font: control.font
                textFormat: Text.PlainText
                elide: Text.ElideRight
                anchors.verticalCenter: parent.verticalCenter
                color: !control.enabled ? control.tokens.textDisabled : control.primary
                                          ? control.tokens.onPrimary : control.danger
                                            ? control.tokens.danger : control.tokens.text
            }
        }
    }
    background: Rectangle {
        radius: control.tokens.radius
        color: !control.enabled ? (control.flat ? "transparent" : control.tokens.disabled) :
                                  control.primary ? (control.down ? control.tokens.primaryPressed :
                                                                    control.hovered
                                                                    ? control.tokens.primaryHover :
                                                                      control.tokens.primary) :
                                                    control.down || control.checked
                                                    || control.hovered
                                                    ? control.tokens.surfaceHover : control.flat
                                                      ? "transparent" : control.tokens.surface
        border.width: control.visualFocus ? 2 : 1
        border.color: control.visualFocus || control.checked ? control.tokens.primary :
                                                               control.danger
                                                               ? control.tokens.danger :
                                                                 control.flat ? "transparent" :
                                                                                control.tokens.border
        Behavior on color {
            ColorAnimation {
                duration: control.tokens.motionDuration
            }
        }
    }
}
