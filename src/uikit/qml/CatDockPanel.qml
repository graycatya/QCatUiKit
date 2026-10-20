// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Templates as T
import QCatUiKit

T.Control {
    id: control
    // Set before adding to a workspace; immutable while registered.
    required property string dockId
    property string title: dockId
    property ThemePalette theme: Theme
    property real minimumDockWidth: 120
    property real minimumDockHeight: 80
    property bool closable: true
    property bool movable: true
    property bool floatable: true
    property var _dockWorkspace: null
    Component.onDestruction: if (_dockWorkspace)
        _dockWorkspace.panelDestroyed(dockId)
    // The application accepts by calling workspace.controller.closePanel(dockId).
    signal closeRequested
    default property alias content: body.data
    clip: true
    implicitWidth: 240
    implicitHeight: 180
    padding: (theme || Theme).tokens.spacing
    background: Rectangle {
        color: (control.theme || Theme).tokens.surface
    }
    contentItem: Item {
        id: body
    }
}
