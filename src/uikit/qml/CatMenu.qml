// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Templates as T
import QtQuick.Controls.Basic as B
import QCatUiKit

T.Menu {
    id: control
    property ThemePalette theme: Theme
    readonly property var tokens: (theme || Theme).tokens
    padding: 4
    implicitWidth: 220
    implicitHeight: Math.min(320, implicitContentHeight + topPadding + bottomPadding)
    delegate: CatMenuItem {
        theme: control.theme
    }
    contentItem: ListView {
        implicitHeight: contentHeight
        model: control.contentModel
        currentIndex: control.currentIndex
        clip: true
        interactive: contentHeight > height
        boundsBehavior: Flickable.StopAtBounds
        T.ScrollIndicator.vertical: B.ScrollIndicator {}
    }
    background: SurfaceImpl {
        theme: control.theme
    }
    enter: Transition {
        NumberAnimation {
            property: "opacity"
            from: 0
            to: 1
            duration: control.tokens.motionDuration
        }
    }
    exit: Transition {
        NumberAnimation {
            property: "opacity"
            from: 1
            to: 0
            duration: control.tokens.motionDuration
        }
    }
}
