// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Templates as T
import QCatUiKit

T.Control {
    id: control
    property ThemePalette theme: Theme
    readonly property var tokens: (theme || Theme).tokens
    property string headerText
    property bool expanded: false
    default property alias body: bodyColumn.data
    implicitWidth: Math.max(240, implicitContentWidth)
    implicitHeight: headerButton.height + bodyClip.height
    contentItem: Item {
        implicitWidth: headerButton.implicitWidth
        implicitHeight: control.implicitHeight
        CatToggleButton {
            id: headerButton
            theme: control.theme
            width: parent.width
            text: (control.expanded ? "▾  " : "▸  ") + control.headerText
            checked: control.expanded
            Accessible.name: control.headerText
            onClicked: control.expanded = checked
        }
        Item {
            id: bodyClip
            y: headerButton.height
            width: parent.width
            height: control.expanded ? bodyColumn.implicitHeight + 2 * control.tokens.padding : 0
            clip: true
            visible: height > 0
            enabled: control.expanded
            Behavior on height {
                NumberAnimation {
                    duration: control.tokens.motionDuration
                }
            }
            Column {
                id: bodyColumn
                x: control.tokens.padding
                y: control.tokens.padding
                width: Math.max(0, parent.width - 2 * control.tokens.padding)
                spacing: control.tokens.spacing
            }
        }
    }
    background: SurfaceImpl {
        theme: control.theme
    }
}
