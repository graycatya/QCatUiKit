// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QCatUiKit
import QtQuick.Templates as T

T.ToolTip {
    id: control
    property ThemePalette theme: Theme
    property bool hoverInteractionEnabled: true
    readonly property var tokens: (theme || Theme).tokens
    delay: 600
    timeout: 3000
    QtObject {
        id: state
        property bool restarting: false
        property bool dismissed: false
        property bool positionedAtPointer: false
        property point position: Qt.point(0, 0)
    }
    // Temporarily suppress the caller's visibility request, preserving its binding.
    // Restoring it restarts ToolTip's own delay rather than opening a second timer.
    Binding {
        target: control
        property: "visible"
        value: false
        when: control.hoverInteractionEnabled && (state.restarting || state.dismissed)
        restoreMode: Binding.RestoreBindingOrValue
    }
    // Observe the anchor without consuming its hover or click events.
    HoverHandler {
        id: pointer
        parent: control.parent
        enabled: control.hoverInteractionEnabled
        onPointChanged: {
            if (!control.visible && !state.dismissed) {
                state.restarting = true
                Qt.callLater(function() {
                    if (state)
                        state.restarting = false
                })
            }
        }
        onHoveredChanged: {
            if (!hovered) {
                control.close()
                state.dismissed = false
            } else if (control.visible && !state.positionedAtPointer) {
                state.position = point.position
                state.positionedAtPointer = true
            }
        }
    }
    PointHandler {
        parent: control.parent
        enabled: control.hoverInteractionEnabled
        acceptedButtons: Qt.AllButtons
        onActiveChanged: {
            if (active)
                state.dismissed = true
        }
    }
    closePolicy: hoverInteractionEnabled ? T.Popup.CloseOnEscape | T.Popup.CloseOnPressOutside
                                        : T.Popup.NoAutoClose
    Shortcut {
        sequence: "Escape"
        enabled: control.hoverInteractionEnabled && control.visible
        context: Qt.WindowShortcut
        onActivated: {
            state.dismissed = true
            control.close()
        }
    }
    Connections {
        target: control.parent ? control.parent.Window.window : null
        function onActiveChanged() {
            const window = control.parent ? control.parent.Window.window : null
            if (control.hoverInteractionEnabled && window && !window.active) {
                if (pointer && pointer.hovered)
                    state.dismissed = true
                control.close()
            }
        }
    }
    onAboutToShow: {
        state.positionedAtPointer = pointer && pointer.hovered
        if (state.positionedAtPointer)
            state.position = pointer.point.position
    }
    onClosed: {
        if (pointer && pointer.hovered)
            state.dismissed = true
    }
    x: state.positionedAtPointer ? state.position.x + 12 : (parent ? (parent.width - width) / 2 : 0)
    y: {
        if (!state.positionedAtPointer)
            return parent ? parent.height + 16 : 16
        const below = state.position.y + 16
        const window = parent ? parent.Window.window : null
        if (window && parent.mapToItem(window.contentItem, 0, below).y + height > window.height - 8)
            return state.position.y - height - 16
        return below
    }
    margins: 8
    padding: tokens.spacing
    implicitWidth: implicitContentWidth + leftPadding + rightPadding
    implicitHeight: implicitContentHeight + topPadding + bottomPadding
    width: Math.min(implicitWidth, 360)
    contentItem: CatLabel {
        theme: control.theme
        text: control.text
        wrapMode: Text.Wrap
    }
    background: SurfaceImpl {
        theme: control.theme
    }
}
