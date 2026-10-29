// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    required property var probe
    id: root
    width: 800
    height: 650
    visible: true
    title: qsTr("macOS mascot window prototype")
    onClosing: Qt.quit()
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        Label {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: qsTr("Native body and bubble panels; a separate process counts input behind them. Full platform acceptance remains Pending. Close this window to clean up the probe.")
        }
        Flow {
            Layout.fillWidth: true
            spacing: 8
            Repeater {
                model: ["show", "hide", "reset", "passthrough", "topmost", "bubble", "menu", "save"]
                Button {
                    required property string modelData
                    text: modelData
                    onClicked: root.probe.command(modelData)
                }
            }
        }
        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            TextArea {
                text: root.probe.report
                readOnly: true
                selectByMouse: true
                textFormat: TextEdit.PlainText
                font.family: "Menlo"
                Accessible.name: qsTr("macOS prototype evidence")
            }
        }
    }
}
