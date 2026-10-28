// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QCatUiKit

ApplicationWindow {
    id: root
    width: 760
    height: 660
    minimumWidth: 420
    minimumHeight: 520
    visible: true
    title: qsTr("CatGray character playback")
    color: Theme.tokens.background
    Component.onCompleted: {
        Theme.density = ThemePalette.Desktop
        Theme.colorScheme = previewDark ? ThemePalette.Dark : ThemePalette.Light
    }
    required property MascotPetController controller
    property bool previewDark: false
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.tokens.spacing * 2
        spacing: Theme.tokens.spacing
        CatLabel {
            Layout.fillWidth: true
            text: qsTr("Character playback preview — desktop integration pending")
            wrapMode: Text.Wrap
        }
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: root.previewDark ? "#28313b" : "#dce5ef"
            radius: 12
            Flickable {
                id: viewport
                anchors.fill: parent
                clip: true
                contentWidth: Math.max(width, visual.width)
                contentHeight: Math.max(height, visual.height)
                boundsBehavior: Flickable.StopAtBounds
                ScrollBar.vertical: CatScrollBar { }
                ScrollBar.horizontal: CatScrollBar { }
                Loader {
                    id: visual
                    x: (viewport.contentWidth - width) / 2
                    y: (viewport.contentHeight - height) / 2
                    // Development harness for the library's private visual item.
                    Component.onCompleted: setSource("qrc:/qt/qml/QCatUiKit/qml/MascotSprite.qml",
                                                      { controller: root.controller })
                }
            }
        }
        RowLayout {
            Layout.fillWidth: true
            CatButton {
                text: qsTr("CatGray sprite")
                onClicked: root.controller.packageSource = "qrc:/mascot-player/catgray/manifest.json"
            }
            CatButton {
                text: qsTr("CatGray Lottie")
                onClicked: root.controller.packageSource = "qrc:/mascot-player/catgray-lottie/mascot.json"
            }
        }
        RowLayout {
            Layout.fillWidth: true
            CatComboBox {
                id: actions
                Layout.fillWidth: true
                model: root.controller.availableActions
                currentIndex: root.controller.availableActions.indexOf(root.controller.currentAction)
                Accessible.name: qsTr("Action")
                onActivated: root.controller.requestAction(currentText)
            }
            CatButton {
                text: root.controller.paused ? qsTr("Resume") : qsTr("Pause")
                onClicked: root.controller.paused ? root.controller.resumeActions() : root.controller.pauseActions()
            }
            CatButton {
                text: qsTr("Reset")
                onClicked: root.controller.resetToDefault()
            }
        }
        RowLayout {
            Layout.fillWidth: true
            CatLabel { text: qsTr("Scale") }
            CatSlider {
                Layout.fillWidth: true
                from: 0.5
                to: 2
                value: root.controller.petScale
                Accessible.name: qsTr("Character scale")
                onMoved: root.controller.petScale = value
            }
            CatCheckBox {
                text: qsTr("Reduce motion")
                checked: Theme.reducedMotion
                onToggled: Theme.reducedMotion = checked
            }
        }
        CatLabel {
            Layout.fillWidth: true
            text: root.controller.status === MascotPetController.Error
                  ? root.controller.errorString
                  : root.controller.status === MascotPetController.Loading
                    ? qsTr("Loading character…")
                    : qsTr("Action: %1 · Queue: %2").arg(root.controller.currentAction).arg(root.controller.queueLength)
            wrapMode: Text.Wrap
        }
    }
}
