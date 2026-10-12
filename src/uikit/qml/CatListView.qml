// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Templates as T
import QCatUiKit

ListView {
    id: control
    property ThemePalette theme: Theme
    readonly property var tokens: (theme || Theme).tokens
    property string textRole: ""
    property bool multiSelect: false
    property var selectedIndexes: []
    signal activated(int index)
    implicitWidth: 320
    implicitHeight: 240
    clip: true
    activeFocusOnTab: true
    keyNavigationEnabled: true
    currentIndex: -1
    spacing: 2
    boundsBehavior: Flickable.StopAtBounds
    MouseScrollImpl {
        view: control
    }
    function labelFor(entry: var): string {
        if (textRole.length)
            return String(entry[textRole] === undefined ? "" : entry[textRole]);
        const item = entry.modelData === undefined ? entry.display : entry.modelData;
        if (item === undefined)
            return "";
        return typeof item === "object" ? String(item.title || item.name || item.display || "") : String(
                                              item);
    }
    function select(index: int): void {
        if (!enabled || index < 0 || index >= count)
            return;
        currentIndex = index;
        if (multiSelect) {
            const next = selectedIndexes.slice();
            const present = next.indexOf(index);
            if (present < 0)
                next.push(index);
            else
                next.splice(present, 1);
            selectedIndexes = next;
        } else
            selectedIndexes = [index];
        activated(index);
    }
    Keys.onReturnPressed: event => {
        select(currentIndex);
        event.accepted = true;
    }
    delegate: CatButton {
        required property int index
        required property var model
        theme: control.theme
        width: control.width
        flat: true
        text: control.labelFor(model)
        checked: control.multiSelect ? control.selectedIndexes.includes(index) :
                                       control.currentIndex === index
        onClicked: control.select(index)
    }
    T.ScrollBar.vertical: CatScrollBar {
        theme: control.theme
    }
}
