// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Templates as T
import QCatUiKit

T.ScrollView {
    id: control
    property ThemePalette theme: Theme
    property var _hoverHandler: null
    clip: true
    // Create the handler after ScrollView has connected its deferred Flickable
    // and scroll bars; reading contentItem earlier can bypass that setup.
    resources: [
        Component {
            id: mouseScroll
            MouseScrollImpl {
                view: control.contentItem as Flickable
            }
        },
        Component {
            id: revealHover
            HoverHandler {}
        }
    ]
    Component.onCompleted: {
        mouseScroll.createObject(control);
        // Track the viewport after ScrollView has created its content item, so
        // overlay scroll bars stay hidden until the pointer enters the view.
        _hoverHandler = revealHover.createObject(control.contentItem);
    }
    readonly property bool _pointerInside: _hoverHandler ? _hoverHandler.hovered : false
    T.ScrollBar.vertical: CatScrollBar {
        theme: control.theme
        parent: control
        x: control.mirrored ? 0 : control.width - width
        y: control.topPadding
        height: control.availableHeight
        revealed: control._pointerInside
    }
    T.ScrollBar.horizontal: CatScrollBar {
        theme: control.theme
        parent: control
        x: control.leftPadding
        y: control.height - height
        width: control.availableWidth
        revealed: control._pointerInside
    }
}
