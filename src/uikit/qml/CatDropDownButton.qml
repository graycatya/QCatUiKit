// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Templates as T
import QCatUiKit

CatButton {
    id: control
    property T.Menu menu
    readonly property bool isOpen: menu ? menu.opened : false
    rightPadding: tokens.padding + 18
    Text {
        text: "▾"
        color: control.enabled ? (control.primary ? control.tokens.onPrimary :
                                                    control.tokens.textSecondary) :
                                 control.tokens.textDisabled
        x: control.mirrored ? 8 : control.width - width - 8
        anchors.verticalCenter: parent.verticalCenter
        font.pixelSize: 16
    }
    onClicked: if (menu) {
                   menu.parent = control;
                   menu.x = 0;
                   menu.y = height + 4;
                   menu.open();
               }
}
