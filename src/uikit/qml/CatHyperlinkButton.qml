// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QCatUiKit
import QtQuick

CatButton {
    id: control
    property url url
    property bool showUnderline: false
    flat: true
    Accessible.role: Accessible.Link
    font.underline: showUnderline || hovered
    contentItem: Text {
        text: control.text
        font: control.font
        color: !control.enabled ? control.tokens.textDisabled : control.down
                                  ? control.tokens.primaryPressed : control.tokens.primary
        textFormat: Text.PlainText
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }
    onClicked: if (url.toString().length)
                   Qt.openUrlExternally(url)
}
