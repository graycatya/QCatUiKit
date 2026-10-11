// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Basic as B
import QtQuick.Layouts
import QCatUiKit

ColumnLayout {
    id: control
    property string pageId: "home"
    property string categoryId: "all"
    signal navigate(string page, string category)
    spacing: 4

    CatLabel {
        text: qsTr("WORKSPACE")
        color: Theme.tokens.textSecondary
        font.pixelSize: 11
        font.letterSpacing: 1.5
        Layout.leftMargin: 14
        Layout.bottomMargin: 8
    }
    Repeater {
        model: [
            {
                id: "home",
                title: qsTr("Overview"),
                icon: "home"
            },
            {
                id: "components",
                title: qsTr("All components"),
                icon: "grid"
            }
        ]
        delegate: B.ItemDelegate {
            id: rootLink
            required property var modelData
            objectName: "navigation_" + modelData.id
            Layout.fillWidth: true
            text: modelData.title
            implicitHeight: 40
            highlighted: control.pageId === modelData.id && (modelData.id !== "components" || control.categoryId === "all")
            Accessible.name: text
            onClicked: control.navigate(modelData.id, "all")
            contentItem: RowLayout {
                spacing: 12
                GalleryIcon {
                    name: rootLink.modelData.icon
                    color: rootLink.highlighted ? Theme.tokens.primary : Theme.tokens.textSecondary
                }
                CatLabel {
                    text: rootLink.text
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                    color: rootLink.highlighted ? Theme.tokens.primary : Theme.tokens.text
                }
            }
            background: Rectangle {
                radius: 6
                color: rootLink.highlighted || rootLink.hovered ? Theme.tokens.surfaceHover : "transparent"
                border.width: rootLink.visualFocus ? 1 : 0
                border.color: Theme.tokens.primary
            }
        }
    }
    CatLabel {
        text: qsTr("COMPONENTS")
        color: Theme.tokens.textSecondary
        font.pixelSize: 11
        font.letterSpacing: 1.5
        Layout.leftMargin: 14
        Layout.topMargin: 24
        Layout.bottomMargin: 8
    }
    Repeater {
        model: GalleryCatalog.categories
        delegate: B.ItemDelegate {
            id: categoryLink
            required property var modelData
            Layout.fillWidth: true
            text: modelData.title
            implicitHeight: 40
            highlighted: control.pageId === "components" && control.categoryId === modelData.id
            Accessible.name: text
            onClicked: control.navigate("components", modelData.id)
            contentItem: RowLayout {
                spacing: 12
                GalleryIcon {
                    name: categoryLink.modelData.icon
                    color: categoryLink.highlighted ? Theme.tokens.primary : Theme.tokens.textSecondary
                }
                CatLabel {
                    text: categoryLink.text
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                    color: categoryLink.highlighted ? Theme.tokens.primary : Theme.tokens.text
                }
                CatLabel {
                    text: GalleryCatalog.components.filter(item => item.category === categoryLink.modelData.id).length
                    color: Theme.tokens.textSecondary
                    font.pixelSize: 11
                }
            }
            background: Rectangle {
                radius: 6
                color: categoryLink.highlighted || categoryLink.hovered ? Theme.tokens.surfaceHover : "transparent"
                border.width: categoryLink.visualFocus ? 1 : 0
                border.color: Theme.tokens.primary
            }
        }
    }
    Item {
        Layout.fillHeight: true
        Layout.minimumHeight: 28
    }
    B.ItemDelegate {
        id: settingsLink
        objectName: "navigation_settings"
        Layout.fillWidth: true
        text: qsTr("Settings")
        implicitHeight: 40
        Accessible.name: text
        onClicked: control.navigate("settings", "all")
        contentItem: RowLayout {
            spacing: 12
            GalleryIcon {
                name: "settings"
            }
            CatLabel {
                text: settingsLink.text
                Layout.fillWidth: true
            }
        }
        background: Rectangle {
            radius: 6
            color: settingsLink.hovered || control.pageId === "settings" ? Theme.tokens.surfaceHover : "transparent"
        }
    }
    CatLabel {
        text: "QCatUiKit " + QCatUiKit.version
        color: Theme.tokens.textSecondary
        font.pixelSize: 11
        Layout.leftMargin: 14
        Layout.topMargin: 8
        Layout.bottomMargin: 6
    }
}
