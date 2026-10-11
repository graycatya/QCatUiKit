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
    property bool autoTruncateOnItemClick: false
    signal itemClicked(int index)
    implicitWidth: path.implicitWidth
    implicitHeight: path.implicitHeight
    contentItem: Flow {
        id: path
        spacing: 4
        layoutDirection: control.mirrored ? Qt.RightToLeft : Qt.LeftToRight
        Repeater {
            model: control.model
            Row {
                required property int index
                required property var modelData
                CatLabel {
                    theme: control.theme
                    text: control.mirrored ? "‹" : "›"
                    visible: parent.index > 0
                    height: control.tokens.controlHeight
                    verticalAlignment: Text.AlignVCenter
                }
                CatButton {
                    theme: control.theme
                    text: String(typeof parent.modelData === "object" ? parent.modelData.text
                                                                        || parent.modelData.title
                                                                        || "" : parent.modelData)
                    flat: true
                    onClicked: {
                        const clickedIndex = parent.index;
                        control.itemClicked(clickedIndex);
                        if (control.autoTruncateOnItemClick)
                            control.model = control.model.slice(0, clickedIndex + 1);
                    }
                }
            }
        }
    }
}
