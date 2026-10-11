// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Templates as T
import QCatUiKit

GridView {
    id: control
    property ThemePalette theme: Theme
    readonly property var tokens: (theme || Theme).tokens
    property string textRole: ""
    signal activated(int index)
    implicitWidth: 320
    implicitHeight: 240
    cellWidth: 140
    cellHeight: 80
    clip: true
    activeFocusOnTab: true
    keyNavigationEnabled: true
    currentIndex: -1
    boundsBehavior: Flickable.StopAtBounds
    delegate: CatButton {
        required property int index
        required property var model
        theme: control.theme
        width: Math.max(0, control.cellWidth - control.tokens.spacing)
        height: Math.max(0, control.cellHeight - control.tokens.spacing)
        text: String(control.textRole.length ? model[control.textRole] : model.modelData
                                               === undefined ? model.display : model.modelData)
        checked: control.currentIndex === index
        onClicked: {
            control.currentIndex = index;
            control.activated(index);
        }
    }
    Keys.onReturnPressed: event => {
        if (currentIndex >= 0)
            activated(currentIndex);
        event.accepted = true;
    }
    T.ScrollBar.vertical: CatScrollBar {
        theme: control.theme
    }
}
