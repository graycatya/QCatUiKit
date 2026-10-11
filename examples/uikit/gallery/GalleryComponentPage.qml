// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Basic as B
import QtQuick.Layouts
import QCatUiKit

ColumnLayout {
    id: control
    required property var entry
    signal back
    spacing: 20
    CatButton {
        text: qsTr("Back to components")
        onClicked: control.back()
    }
    ColumnLayout {
        Layout.fillWidth: true
        spacing: 8
        CatLabel {
            objectName: "galleryPageTitle"
            text: control.entry.title
            font.pixelSize: 32
            font.bold: true
            wrapMode: Text.Wrap
            Layout.fillWidth: true
        }
        CatLabel {
            text: control.entry.description
            color: Theme.tokens.textSecondary
            wrapMode: Text.Wrap
            Layout.fillWidth: true
        }
        CatLabel {
            text: control.entry.name + "  ·  " + GalleryCatalog.categoryTitle(control.entry.category)
            font.pixelSize: 12
            color: Theme.tokens.primary
            Layout.fillWidth: true
            wrapMode: Text.Wrap
        }
    }
    Rectangle {
        Layout.fillWidth: true
        implicitHeight: previewLayout.implicitHeight + 40
        radius: 12
        color: Theme.tokens.surface
        border.color: Theme.tokens.border
        ColumnLayout {
            id: previewLayout
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.margins: 20
            spacing: 12
            RowLayout {
                Layout.fillWidth: true
                CatLabel {
                    text: qsTr("Live preview")
                    font.bold: true
                    Layout.fillWidth: true
                }
                CatButton {
                    text: qsTr("Reset example")
                    font.pixelSize: 12
                    onClicked: {
                        previewLoader.active = false;
                        previewLoader.active = true;
                    }
                }
            }
            Loader {
                id: previewLoader
                objectName: "componentPreview"
                Layout.fillWidth: true
                sourceComponent: Component {
                    GalleryPreview {
                        componentId: control.entry.id
                    }
                }
            }
        }
    }
    Rectangle {
        Layout.fillWidth: true
        implicitHeight: codeLayout.implicitHeight + 40
        radius: 12
        color: Theme.tokens.surface
        border.color: Theme.tokens.border
        ColumnLayout {
            id: codeLayout
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.margins: 20
            spacing: 16
            RowLayout {
                Layout.fillWidth: true
                GalleryIcon {
                    name: "code"
                    color: Theme.tokens.primary
                }
                CatLabel {
                    text: qsTr("QML example")
                    font.bold: true
                    Layout.fillWidth: true
                }
                CatButton {
                    id: copyButton
                    property bool copied: false
                    text: copied ? qsTr("Copied") : qsTr("Copy")
                    onClicked: {
                        codeText.selectAll();
                        codeText.copy();
                        codeText.deselect();
                        copied = true;
                        copyTimer.restart();
                    }
                    Timer {
                        id: copyTimer
                        interval: 1800
                        onTriggered: copyButton.copied = false
                    }
                }
            }
            B.TextArea {
                id: codeText
                objectName: "codeSample"
                Layout.fillWidth: true
                text: "import QtQuick\nimport QCatUiKit\n\n" + control.entry.code
                readOnly: true
                selectByMouse: true
                wrapMode: TextEdit.Wrap
                color: Theme.tokens.text
                selectionColor: Theme.tokens.primary
                font.family: Qt.platform.os === "windows" ? "Consolas" : "monospace"
                font.pixelSize: 13
                padding: 16
                background: Rectangle {
                    color: Theme.tokens.background
                    radius: 6
                }
                Accessible.name: qsTr("QML example")
            }
        }
    }
}
