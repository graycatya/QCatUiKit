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
    title: qsTr("Linux mascot prototype — platform acceptance Pending")
    function syncDiagnosticsVisibility(): void {
        root.probe.setDiagnosticsVisible(root.visible && root.visibility !== Window.Minimized)
    }
    Component.onCompleted: syncDiagnosticsVisibility()
    onVisibilityChanged: syncDiagnosticsVisibility()
    onClosing: Qt.quit()
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        Label {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: qsTr("Independent process input counts are evidence; Qt flags alone are not. Native Wayland positioning and topmost need a compositor companion. Close probe cleans up the target and tray.")
        }
        Flow {
            Layout.fillWidth: true
            spacing: 8
            Button { text: qsTr("Show"); onClicked: root.probe.setPetVisible(true) }
            Button { text: qsTr("Hide"); onClicked: root.probe.setPetVisible(false) }
            Button { text: qsTr("Reset position"); onClicked: root.probe.resetPosition() }
            Button { text: qsTr("Show bubble"); onClicked: root.probe.setBubble(true) }
            Button { text: qsTr("Hide bubble"); onClicked: root.probe.setBubble(false) }
            CheckBox {
                text: qsTr("Passthrough")
                checked: root.probe.passthrough
                onToggled: root.probe.setPassthrough(checked)
            }
            CheckBox {
                text: qsTr("Always on top")
                checked: root.probe.topmost
                onToggled: root.probe.setTopmost(checked)
            }
            Button { text: qsTr("Save evidence"); onClicked: root.probe.save() }
            Button { text: qsTr("Native snapshot"); onClicked: root.probe.snapshotCompanion() }
            Button { text: qsTr("Close probe"); onClicked: Qt.quit() }
        }
        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            TextArea {
                text: root.probe.report
                textFormat: TextEdit.PlainText
                readOnly: true
                selectByMouse: true
                font.family: "monospace"
                Accessible.name: qsTr("Linux prototype evidence")
            }
        }
    }
}
