// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QCatUiKit
import QtQuick
import QtQuick.Controls.Basic as B

B.SwipeView {
    id: control
    property ThemePalette theme: Theme
    implicitWidth: 320
    implicitHeight: 200
    clip: true
    contentItem: ListView {
        model: control.contentModel
        orientation: control.orientation
        currentIndex: control.currentIndex
        onCurrentIndexChanged: control.setCurrentIndex(currentIndex)
        spacing: control.spacing
        interactive: control.interactive
        snapMode: ListView.SnapOneItem
        boundsBehavior: Flickable.StopAtBounds
        highlightRangeMode: ListView.StrictlyEnforceRange
        highlightMoveDuration: (control.theme || Theme).tokens.motionDuration
        preferredHighlightBegin: 0
        preferredHighlightEnd: 0
        maximumFlickVelocity: 4 * (control.orientation === Qt.Horizontal ? width : height)
    }
}
