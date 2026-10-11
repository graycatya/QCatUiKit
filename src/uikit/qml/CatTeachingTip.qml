// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick.Layouts
import QtQuick
import QCatUiKit

CatFlyout {
    id: control
    property string title
    property string text
    property string actionText
    signal actionTriggered
    width: 300
    contentItem: ColumnLayout {
        spacing: 12
        CatLabel {
            theme: control.theme
            text: control.title
            font.bold: true
            Layout.fillWidth: true
            wrapMode: Text.Wrap
        }
        CatLabel {
            theme: control.theme
            text: control.text
            Layout.fillWidth: true
            wrapMode: Text.Wrap
        }
        RowLayout {
            Layout.fillWidth: true
            CatButton {
                theme: control.theme
                text: control.actionText
                visible: text.length > 0
                primary: true
                onClicked: control.actionTriggered()
            }
            Item {
                Layout.fillWidth: true
            }
            CatButton {
                theme: control.theme
                text: qsTr("Close")
                onClicked: control.close()
            }
        }
    }
}
