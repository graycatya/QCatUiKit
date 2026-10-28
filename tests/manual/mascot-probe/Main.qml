// SPDX-License-Identifier: Apache-2.0
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: root
    width: 840
    height: 720
    visible: true
    title: qsTr("Mascot desktop capability probe")
    property string renderedReport: probeReport
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        Label {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: qsTr("Platform acceptance: Pending. This diagnostic does not certify desktop behavior. Graphics API is the QSGRendererInterface enum after scene graph initialization.")
        }
        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            TextArea {
                text: root.renderedReport
                readOnly: true
                selectByMouse: true
                font.family: "monospace"
                textFormat: TextEdit.PlainText
                Accessible.name: qsTr("Platform diagnostic report")
            }
        }
    }
}
