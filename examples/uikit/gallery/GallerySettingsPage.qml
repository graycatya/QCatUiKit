// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Basic as B
import QtQuick.Layouts
import QCatUiKit

ColumnLayout {
    id: control
    spacing: 24
    // A palette defined entirely in the application; it never modifies the library.
    readonly property var solarizedPalette: ({
        background: "#002b36",
        surface: "#073642",
        surfaceHover: "#0a4453",
        text: "#93a1a1",
        textSecondary: "#839496",
        textDisabled: "#586e75",
        border: "#586e75",
        disabled: "#0a2b33",
        success: "#859900",
        warning: "#b58900",
        danger: "#dc322f"
    })
    readonly property int schemeIndex: Object.keys(Theme.colorPalette).length > 0 ? 2 : (Theme.colorScheme === ThemePalette.Dark ? 1 : 0)
    ThemePalette {
        id: solarizedTheme
        colorScheme: ThemePalette.Dark
        radius: 10
        colorPalette: control.solarizedPalette
    }
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
                    objectName: "gallerySchemeLight"
                    text: qsTr("Light")
                    checked: control.schemeIndex === 0
                    onClicked: {
                        Theme.colorPalette = ({});
                        Theme.colorScheme = ThemePalette.Light;
                    }
                }
                CatButton {
                    objectName: "gallerySchemeDark"
                    text: qsTr("Dark")
                    checked: control.schemeIndex === 1
                    onClicked: {
                        Theme.colorPalette = ({});
                        Theme.colorScheme = ThemePalette.Dark;
                    }
                }
                CatButton {
                    objectName: "gallerySchemeCustom"
                    text: qsTr("Custom palette")
                    checked: control.schemeIndex === 2
                    onClicked: {
                        Theme.colorScheme = ThemePalette.Dark;
                        Theme.colorPalette = control.solarizedPalette;
                    }
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
                text: qsTr("Two themes side by side")
                font.pixelSize: 19
                font.bold: true
            }
            CatLabel {
                text: qsTr("The right pane receives a local ThemePalette, so one window can present two visual styles. Both panes use public API only.")
                color: Theme.tokens.textSecondary
                Layout.fillWidth: true
                wrapMode: Text.Wrap
            }
            Flow {
                Layout.fillWidth: true
                spacing: 16
                CatPane {
                    objectName: "galleryThemeDefaultPane"
                    width: 240
                    padding: 16
                    contentItem: ColumnLayout {
                        spacing: 10
                        CatLabel {
                            text: qsTr("Default theme")
                            font.bold: true
                        }
                        CatButton {
                            text: qsTr("Primary action")
                            primary: true
                            Layout.fillWidth: true
                        }
                        CatLabel {
                            text: qsTr("Secondary information")
                            color: Theme.tokens.textSecondary
                            Layout.fillWidth: true
                            wrapMode: Text.Wrap
                        }
                    }
                }
                CatPane {
                    objectName: "galleryThemeCustomPane"
                    theme: solarizedTheme
                    width: 240
                    padding: 16
                    contentItem: ColumnLayout {
                        spacing: 10
                        CatLabel {
                            theme: solarizedTheme
                            text: qsTr("Custom palette theme")
                            font.bold: true
                        }
                        CatButton {
                            theme: solarizedTheme
                            text: qsTr("Primary action")
                            primary: true
                            Layout.fillWidth: true
                        }
                        CatLabel {
                            theme: solarizedTheme
                            text: qsTr("Secondary information")
                            color: solarizedTheme.tokens.textSecondary
                            Layout.fillWidth: true
                            wrapMode: Text.Wrap
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
