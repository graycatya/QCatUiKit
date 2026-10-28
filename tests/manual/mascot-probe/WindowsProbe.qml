// SPDX-License-Identifier: Apache-2.0
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: root
    required property var probe
    width: 860
    height: 740
    visible: true
    title: qsTr("Windows mascot prototype - not full platform acceptance")
    onClosing: Qt.quit()
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        Label {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: qsTr("A separate native process counts clicks and keyboard input behind the overlay. Prototype results do not certify multi-screen, bubbles or full desktop integration.")
        }
        Flow {
            Layout.fillWidth: true
            spacing: 8
            Button {
                text: qsTr("Show pet")
                onClicked: root.probe.setPetVisible(true)
            }
            Button {
                text: qsTr("Hide pet")
                onClicked: root.probe.setPetVisible(false)
            }
            Button {
                text: qsTr("Close probe")
                onClicked: Qt.quit()
            }
            Button {
                text: qsTr("Reset position")
                onClicked: root.probe.resetPosition()
            }
            CheckBox {
                text: qsTr("Whole-window passthrough")
                checked: root.probe.passthrough
                onToggled: root.probe.setPassthrough(checked)
            }
            CheckBox {
                text: qsTr("Always on top")
                checked: root.probe.topmost
                onToggled: root.probe.setTopmost(checked)
            }
            Button {
                text: qsTr("Hide bubble")
                onClicked: root.probe.setBubble(false)
            }
            Button {
                text: qsTr("Show bubble")
                onClicked: root.probe.setBubble(true)
            }
        }
        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            TextArea {
                text: root.probe.report
                readOnly: true
                selectByMouse: true
                font.family: "monospace"
                textFormat: TextEdit.PlainText
                Accessible.name: qsTr("Windows prototype evidence")
            }
        }
    }
}
