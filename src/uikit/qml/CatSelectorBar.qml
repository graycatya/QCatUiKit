// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Templates as T
import QCatUiKit

T.Control {
    id: control
    property ThemePalette theme: Theme
    readonly property var tokens: (theme || Theme).tokens
    property var model: []
    property int selectedIndex: 0
    readonly property int itemCount: model.length
    signal selectionRequested(int index)
    implicitWidth: selectors.implicitWidth
    implicitHeight: selectors.implicitHeight
    contentItem: Flow {
        id: selectors
        spacing: control.tokens.spacing
        layoutDirection: control.mirrored ? Qt.RightToLeft : Qt.LeftToRight
        Repeater {
            model: control.model
            CatToggleButton {
                required property int index
                required property var modelData
                theme: control.theme
                text: String(typeof modelData === "object" ? modelData.text || modelData.title
                                                             || "" : modelData)
                checked: control.selectedIndex === index
                onClicked: {
                    control.selectedIndex = index;
                    control.selectionRequested(index);
                }
            }
        }
    }
    onItemCountChanged: selectedIndex = itemCount ? Math.max(0, Math.min(itemCount - 1,
                                                                         selectedIndex)) : -1
}
