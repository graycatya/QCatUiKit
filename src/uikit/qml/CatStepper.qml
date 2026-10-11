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
    property int currentIndex: -1
    property int orientation: Qt.Horizontal
    signal stepRequested(int index)
    implicitWidth: orientation === Qt.Horizontal ? 400 : 240
    implicitHeight: steps.implicitHeight
    contentItem: Grid {
        id: steps
        columns: control.orientation === Qt.Vertical ? 1 : Math.max(1, Math.floor(control.width
                                                                                  / 140))
        spacing: control.tokens.spacing
        Repeater {
            model: control.model
            CatButton {
                required property int index
                required property var modelData
                width: Math.min(140, control.width)
                theme: control.theme
                text: (typeof modelData === "object" && modelData.state === "completed" ? "✓" :
                                                                                          String(index
                                                                                                 + 1)) + "  "
                      + String(typeof modelData === "object" ? modelData.text || modelData.title
                                                               || "" : modelData)
                checked: control.currentIndex === index
                enabled: typeof modelData !== "object" || modelData.enabled !== false
                danger: typeof modelData === "object" && modelData.state === "error"
                onClicked: control.stepRequested(index)
            }
        }
    }
}
