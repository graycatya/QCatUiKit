// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QCatUiKit
import QtQuick

Rectangle {
    property ThemePalette theme: Theme
    property int orientation: Qt.Horizontal
    property real thickness: 1
    property real leadingInset: 0
    property real trailingInset: 0
    implicitWidth: orientation === Qt.Horizontal ? 200 : Math.max(1, thickness)
    implicitHeight: orientation === Qt.Horizontal ? Math.max(1, thickness) : 200
    color: (theme || Theme).tokens.border
    Accessible.ignored: true
}
