// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QCatUiKit

Image {
    property ThemePalette theme: Theme
    readonly property var tokens: (theme || Theme).tokens
    asynchronous: true
    fillMode: Image.PreserveAspectFit
    CatBusyIndicator {
        theme: parent.theme
        anchors.centerIn: parent
        visible: parent.status === Image.Loading
    }
}
