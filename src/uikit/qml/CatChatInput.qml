// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QCatUiKit
import QtQuick.Layouts

Item {
    id: control
    property ThemePalette theme: Theme
    readonly property var tokens: (theme || Theme).tokens
    property alias text: field.text
    property alias placeholderText: field.placeholderText
    signal submitted(string text)
    implicitWidth: 360
    implicitHeight: row.implicitHeight
    function submit(): void {
        const message = field.text.trim();
        if (message.length) {
            submitted(message);
            field.clear();
        }
    }
    RowLayout {
        id: row
        anchors.fill: parent
        spacing: control.tokens.spacing
        CatTextField {
            id: field
            theme: control.theme
            Layout.fillWidth: true
            placeholderText: qsTr("Write a message")
            onAccepted: control.submit()
        }
        CatButton {
            theme: control.theme
            text: qsTr("Send")
            primary: true
            enabled: field.text.trim().length > 0
            onClicked: control.submit()
        }
    }
}
