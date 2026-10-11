// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QCatUiKit
import QtQuick
import QtQml.Models
import QtQuick.Templates as T

T.Control {
    id: control
    property ThemePalette theme: Theme
    readonly property var tokens: (theme || Theme).tokens
    property var primaryActions: []
    property var secondaryActions: []
    padding: 4
    implicitWidth: Math.max(240, commands.implicitWidth + leftPadding + rightPadding)
    implicitHeight: commands.implicitHeight + topPadding + bottomPadding
    contentItem: Flow {
        id: commands
        spacing: 4
        layoutDirection: control.mirrored ? Qt.RightToLeft : Qt.LeftToRight
        Repeater {
            model: control.primaryActions
            CatButton {
                required property var modelData
                theme: control.theme
                action: modelData
                flat: true
            }
        }
        CatDropDownButton {
            theme: control.theme
            text: "⋯"
            implicitWidth: control.tokens.controlHeight
            visible: control.secondaryActions.length > 0
            Accessible.name: qsTr("More commands")
            menu: CatMenu {
                id: overflow
                theme: control.theme
                Instantiator {
                    model: control.secondaryActions
                    CatMenuItem {
                        required property var modelData
                        theme: control.theme
                        action: modelData
                    }
                    onObjectAdded: (index, object) => overflow.insertItem(index, object)
                    onObjectRemoved: (index, object) => overflow.removeItem(object)
                }
            }
        }
    }
    background: SurfaceImpl {
        theme: control.theme
    }
}
