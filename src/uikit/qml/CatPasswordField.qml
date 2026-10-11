// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QCatUiKit
import QtQuick

CatTextField {
    id: control
    property alias password: control.text
    property bool revealed: false
    property bool revealEnabled: true
    echoMode: revealed ? TextInput.Normal : TextInput.Password
    rightPadding: tokens.padding + (revealEnabled ? tokens.controlHeight : 0)
    CatToggleButton {
        theme: control.theme
        visible: control.revealEnabled
        checked: control.revealed
        flat: true
        padding: 6
        text: checked ? "◉" : "◎"
        width: control.tokens.controlHeight
        implicitWidth: width
        height: parent.height
        anchors.right: parent.right
        Accessible.name: checked ? qsTr("Hide password") : qsTr("Show password")
        onToggled: control.revealed = checked
    }
}
