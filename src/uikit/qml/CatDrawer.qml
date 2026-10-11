// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QCatUiKit
import QtQuick.Templates as T

T.Drawer {
    id: control
    property ThemePalette theme: Theme
    readonly property var tokens: (theme || Theme).tokens
    padding: tokens.padding
    parent: T.Overlay.overlay
    edge: Qt.LeftEdge
    focus: true
    modal: true
    implicitWidth: 280
    implicitHeight: implicitContentHeight + topPadding + bottomPadding
    // Qt Drawer advances position through its enter/exit transition actions.
    enter: Transition {
        NumberAnimation {
            property: "position"
            duration: control.tokens.motionDuration
            easing.type: Easing.OutCubic
        }
    }
    exit: Transition {
        NumberAnimation {
            property: "position"
            duration: control.tokens.motionDuration
            easing.type: Easing.InCubic
        }
    }
    background: SurfaceImpl {
        theme: control.theme
    }
}
