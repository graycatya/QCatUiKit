// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import QCatUiKit

ColumnLayout {
    id: control
    signal selectComponent(string id)
    signal browse(string category)
    spacing: 28

    CatPane {
        Layout.fillWidth: true
        padding: control.width < 500 ? 24 : 32
        background: Rectangle {
            radius: 14
            gradient: Gradient {
                orientation: Gradient.Horizontal
                GradientStop {
                    position: 0
                    color: Theme.colorScheme === ThemePalette.Dark ? "#182c45" : "#eaf2ff"
                }
                GradientStop {
                    position: 1
                    color: Theme.colorScheme === ThemePalette.Dark ? "#252537" : "#f1edff"
                }
            }
        }
        contentItem: RowLayout {
            spacing: 32
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 18
                CatLabel {
                    text: qsTr("YOUR NEXT INTERFACE STARTS HERE")
                    color: Theme.tokens.primary
                    font.pixelSize: 11
                    font.letterSpacing: 1.2
                    Layout.fillWidth: true
                    wrapMode: Text.Wrap
                }
                CatLabel {
                    text: qsTr("A toolkit that feels at home.")
                    font.pixelSize: control.width < 500 ? 30 : 38
                    font.bold: true
                    wrapMode: Text.Wrap
                    Layout.fillWidth: true
                    lineHeight: 1.15
                }
                CatLabel {
                    text: qsTr("Explore small, thoughtful Qt Quick components. Try them live, make them yours, and take the QML with you.")
                    color: Theme.tokens.textSecondary
                    wrapMode: Text.Wrap
                    Layout.fillWidth: true
                    lineHeight: 1.4
                }
                Flow {
                    Layout.fillWidth: true
                    spacing: 8
                    CatButton {
                        text: qsTr("Explore components")
                        primary: true
                        onClicked: control.browse("all")
                    }
                    CatButton {
                        text: qsTr("Try a button")
                        onClicked: control.selectComponent("button")
                    }
                }
                CatLabel {
                    text: qsTr("%1 examples · Qt 6.8+ · C++ & QML").arg(GalleryCatalog.components.length)
                    color: Theme.tokens.textSecondary
                    font.pixelSize: 11
                    Layout.fillWidth: true
                    wrapMode: Text.Wrap
                }
            }
            Rectangle {
                visible: control.width >= 720
                Layout.preferredWidth: 210
                Layout.preferredHeight: 210
                radius: 14
                rotation: -5
                color: Theme.tokens.surface
                border.color: Theme.tokens.border
                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 22
                    spacing: 14
                    RowLayout {
                        GalleryLogo {
                            objectName: "homeBrandLogo"
                        }
                        CatLabel {
                            text: "QCatUiKit"
                            font.bold: true
                            Layout.fillWidth: true
                        }
                    }
                    CatButton {
                        text: qsTr("Create something")
                        primary: true
                        Layout.fillWidth: true
                        onClicked: control.selectComponent("button")
                    }
                    CatProgressBar {
                        value: 0.72
                        Layout.fillWidth: true
                    }
                    RowLayout {
                        CatBusyIndicator {
                            implicitWidth: 20
                            implicitHeight: 20
                        }
                        CatLabel {
                            text: qsTr("Made to move with you.")
                            font.pixelSize: 11
                            color: Theme.tokens.textSecondary
                            Layout.fillWidth: true
                            wrapMode: Text.Wrap
                        }
                    }
                }
            }
        }
    }
    ColumnLayout {
        spacing: 6
        CatLabel {
            text: qsTr("Featured components")
            font.pixelSize: 23
            font.bold: true
        }
        CatLabel {
            text: qsTr("A few good places to start.")
            color: Theme.tokens.textSecondary
        }
    }
    GridLayout {
        Layout.fillWidth: true
        columns: control.width >= 840 ? 3 : control.width >= 500 ? 2 : 1
        columnSpacing: 16
        rowSpacing: 16
        uniformCellWidths: true
        Repeater {
            model: ["button", "textfield", "busy", "calendar", "table", "chat"]
            delegate: GalleryEntryCard {
                required property string modelData
                entry: GalleryCatalog.componentForId(modelData)
                Layout.fillWidth: true
                onClicked: control.selectComponent(modelData)
            }
        }
    }
    ColumnLayout {
        spacing: 6
        CatLabel {
            text: qsTr("Browse by category")
            font.pixelSize: 23
            font.bold: true
        }
        CatLabel {
            text: qsTr("Find the right piece for your interface.")
            color: Theme.tokens.textSecondary
            wrapMode: Text.Wrap
            Layout.fillWidth: true
        }
    }
    Flow {
        Layout.fillWidth: true
        spacing: 10
        Repeater {
            model: GalleryCatalog.categories
            delegate: CatButton {
                required property var modelData
                text: modelData.title
                onClicked: control.browse(modelData.id)
            }
        }
    }
}
