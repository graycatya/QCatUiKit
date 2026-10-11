// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Templates as T
import QtQuick.Layouts
import QCatUiKit

T.Control {
    id: control
    property ThemePalette theme: Theme
    readonly property var tokens: (theme || Theme).tokens
    property string text
    property url iconSource
    property T.Menu menu
    property bool primary: false
    property alias checkable: actionButton.checkable
    property alias checked: actionButton.checked
    readonly property bool isOpen: menu ? menu.opened : false
    signal clicked
    signal toggled
    implicitWidth: Math.max(160, actionButton.implicitWidth + secondaryButton.implicitWidth + 2)
    implicitHeight: Math.max(tokens.controlHeight, actionButton.implicitHeight)
    contentItem: RowLayout {
        spacing: 2
        layoutDirection: control.mirrored ? Qt.RightToLeft : Qt.LeftToRight
        CatButton {
            id: actionButton
            theme: control.theme
            text: control.text
            iconSource: control.iconSource
            primary: control.primary
            Layout.fillWidth: true
            Layout.fillHeight: true
            onClicked: control.clicked()
            onToggled: control.toggled()
        }
        CatDropDownButton {
            id: secondaryButton
            theme: control.theme
            primary: control.primary
            menu: control.menu
            implicitWidth: control.tokens.controlHeight
            Layout.fillHeight: true
            Accessible.name: qsTr("More options")
        }
    }
}
