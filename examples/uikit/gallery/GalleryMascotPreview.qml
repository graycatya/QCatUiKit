// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import QCatUiKit

Item {
    id: root
    implicitHeight: layout.implicitHeight
    property string lastEvent: ""
    readonly property string exampleCode: 'CatMascotPet {\n    id: pet\n    packageSource: "qrc:/characters/cat/mascot.json"\n    visible: true\n    onBubbleActionTriggered: (id, action) => console.log(action)\n}\n// Call from the host after Ready:\n// pet.requestAction("working")\n// pet.showBubble("Hello", {actions: [{id: "reply", text: "Reply"}]})'
    CatMascotPet {
        id: pet
        objectName: "galleryMascotPet"
        packageSource: "qrc:/gallery-mascot/catgray/manifest.json"
        petScale: 0.5
        onPlatformError: (code, detail) => root.lastEvent = code + ": " + detail
        onMoveEnded: (id, reason) => root.lastEvent = id + ": " + reason
        onBubbleActionTriggered: (id, action) => root.lastEvent = action
        onRegionActivated: (name, point) => root.lastEvent = name
    }
    CatMascotPet {
        id: second
        objectName: "galleryMascotSecond"
        packageSource: pet.packageSource
        petScale: 0.35
    }
    Component.onDestruction: { pet.hide(); second.hide() }
    ColumnLayout {
        id: layout
        width: parent.width
        spacing: Theme.tokens.spacing
        CatLabel {
            Layout.fillWidth: true
            wrapMode: Text.Wrap
            text: qsTr("The host owns these windows. Closing a character hides it; the host can show it again. System startup and application lifecycle belong to the host.")
        }
        Flow {
            Layout.fillWidth: true
            spacing: 8
            CatButton {
                objectName: "mascotShow"
                text: qsTr("Show character")
                onClicked: pet.show()
            }
            CatButton { text: qsTr("Hide character"); onClicked: pet.hide() }
            CatButton { text: qsTr("Second instance"); onClicked: second.visible = !second.visible }
            CatButton {
                text: qsTr("Switch package")
                onClicked: pet.packageSource = pet.packageSource.toString().indexOf("catgray-lottie") >= 0
                    ? "qrc:/gallery-mascot/catgray/manifest.json" : "qrc:/gallery-mascot/catgray-lottie/mascot.json"
            }
        }
        Flow {
            Layout.fillWidth: true
            spacing: 8
            Repeater {
                model: pet.availableActions
                CatButton {
                    required property string modelData
                    text: modelData
                    onClicked: pet.requestAction(modelData)
                }
            }
        }
        Flow {
            Layout.fillWidth: true
            spacing: 8
            CatButton {
                objectName: "mascotBubble"
                text: qsTr("Show message")
                onClicked: {
                    pet.show()
                    pet.showBubble(qsTr("A message from your application. <b>This is plain text.</b>"),
                        {actions: [{id: "reply", text: qsTr("Reply")}], durationMs: 10000})
                }
            }
            CatButton { text: qsTr("Append text"); onClicked: pet.appendBubbleText(pet.bubbleId, qsTr(" More text.")) }
            CatButton { text: qsTr("Toggle input passthrough"); onClicked: pet.inputTransparent = !pet.inputTransparent }
            CatButton { text: qsTr("Move character"); onClicked: pet.moveTo("", Qt.point(120, 120), 500) }
            CatButton { text: qsTr("Cancel move"); onClicked: pet.cancelMove() }
        }
        CatLabel {
            Layout.fillWidth: true
            wrapMode: Text.Wrap
            text: qsTr("Status: %1; action: %2; passthrough: %3").arg(pet.status).arg(pet.currentAction).arg(pet.inputTransparent)
        }
        CatLabel { Layout.fillWidth: true; wrapMode: Text.Wrap; text: pet.errorString || root.lastEvent }
        Repeater {
            model: Object.keys(pet.capabilities)
            CatLabel {
                required property string modelData
                Layout.fillWidth: true
                wrapMode: Text.Wrap
                text: modelData + ": " + pet.capabilities[modelData].status + " — " + pet.capabilities[modelData].reason
            }
        }
    }
}
