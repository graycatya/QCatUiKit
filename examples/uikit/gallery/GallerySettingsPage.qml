// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Basic as B
import QtQuick.Layouts
import QCatUiKit

ColumnLayout {
    id: control
    spacing: 24
    CatLabel {
        objectName: "galleryPageTitle"
        text: qsTr("Make it yours.")
        font.pixelSize: 32
        font.bold: true
        Layout.fillWidth: true
        wrapMode: Text.Wrap
    }
    CatLabel {
        text: qsTr("One theme, every component. Changes apply throughout the gallery.")
        color: Theme.tokens.textSecondary
        Layout.fillWidth: true
        wrapMode: Text.Wrap
    }
    CatPane {
        Layout.fillWidth: true
        padding: 24
        contentItem: ColumnLayout {
            spacing: 20
            CatLabel {
                text: qsTr("Appearance")
                font.pixelSize: 19
                font.bold: true
            }
            CatLabel {
                text: qsTr("Choose the colors that feel right for your workspace.")
                color: Theme.tokens.textSecondary
                Layout.fillWidth: true
                wrapMode: Text.Wrap
            }
            Flow {
                Layout.fillWidth: true
                spacing: 12
                CatButton {
                    text: qsTr("Light")
                    checked: Theme.colorScheme === ThemePalette.Light
                    onClicked: Theme.colorScheme = ThemePalette.Light
                }
                CatButton {
                    text: qsTr("Dark")
                    checked: Theme.colorScheme === ThemePalette.Dark
                    onClicked: Theme.colorScheme = ThemePalette.Dark
                }
            }
            CatLabel {
                text: qsTr("Accent color")
                font.bold: true
            }
            Flow {
                Layout.fillWidth: true
                spacing: 12
                Repeater {
                    model: ["#1677ff", "#6366f1", "#008577", "#c24178", "#b75b12"]
                    delegate: B.AbstractButton {
                        id: swatch
                        required property string modelData
                        width: 38
                        height: 38
                        Accessible.name: qsTr("Accent %1").arg(modelData)
                        onClicked: Theme.primaryColor = modelData
                        background: Rectangle {
                            radius: 19
                            color: swatch.modelData
                            border.width: swatch.visualFocus ? 3 : 0
                            border.color: Theme.tokens.text
                        }
                        contentItem: CatLabel {
                            text: Theme.primaryColor.toString() === swatch.modelData ? "✓" : ""
                            color: "white"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                    }
                }
            }
        }
    }
    CatPane {
        Layout.fillWidth: true
        padding: 24
        contentItem: ColumnLayout {
            spacing: 16
            CatLabel {
                text: qsTr("Interaction density")
                font.pixelSize: 19
                font.bold: true
            }
            CatLabel {
                text: qsTr("Adapt control sizes for a mouse, a touch screen or a mobile device.")
                color: Theme.tokens.textSecondary
                Layout.fillWidth: true
                wrapMode: Text.Wrap
            }
            Flow {
                Layout.fillWidth: true
                spacing: 12
                CatButton {
                    text: qsTr("Desktop")
                    checked: Theme.density === ThemePalette.Desktop
                    onClicked: Theme.density = ThemePalette.Desktop
                }
                CatButton {
                    text: qsTr("Touch")
                    checked: Theme.density === ThemePalette.Touch
                    onClicked: Theme.density = ThemePalette.Touch
                }
                CatButton {
                    text: qsTr("Mobile")
                    checked: Theme.density === ThemePalette.Mobile
                    onClicked: Theme.density = ThemePalette.Mobile
                }
            }
            CatSwitch {
                text: Theme.reducedMotion ? qsTr("Reduced motion: on") : qsTr("Reduced motion: off")
                checked: Theme.reducedMotion
                onToggled: Theme.reducedMotion = checked
            }
        }
    }
    CatPane {
        Layout.fillWidth: true
        padding: 24
        contentItem: ColumnLayout {
            spacing: 16
            CatLabel {
                text: qsTr("Language")
                font.pixelSize: 19
                font.bold: true
            }
            CatLabel {
                text: qsTr("Switch languages without leaving your current example.")
                color: Theme.tokens.textSecondary
                Layout.fillWidth: true
                wrapMode: Text.Wrap
            }
            GalleryComboBox {
                model: ["English", "中文"]
                currentIndex: GalleryLanguage.language === "zh_CN" ? 1 : 0
                Accessible.name: qsTr("Language")
                onActivated: index => GalleryLanguage.language = index === 1 ? "zh_CN" : "en"
            }
        }
    }
}
