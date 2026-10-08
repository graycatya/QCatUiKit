// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QCatUiKit
import QtQuick.Layouts

CatPane {
    id: control
    property string text
    property int severity: 0 // 0 info, 1 success, 2 warning, 3 error
    property bool closable: false
    signal closed
    implicitWidth: 360
    contentItem: RowLayout {
        spacing: control.tokens.spacing
        Rectangle {
            Layout.preferredWidth: 4
            Layout.fillHeight: true
            radius: 2
            color: control.severity === 1 ? control.tokens.success : control.severity === 2 ? control.tokens.warning : control.severity === 3 ? control.tokens.danger : control.tokens.primary
        }
        CatLabel {
            theme: control.theme
            text: control.text
            wrapMode: Text.Wrap
            Layout.fillWidth: true
        }
        CatButton {
            theme: control.theme
            text: "×"
            visible: control.closable
            implicitWidth: control.tokens.controlHeight
            Accessible.name: qsTr("Close")
            onClicked: {
                control.visible = false;
                control.closed();
            }
        }
    }
}
