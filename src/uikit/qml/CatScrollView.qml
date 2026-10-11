// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Templates as T
import QCatUiKit

T.ScrollView {
    id: control
    property ThemePalette theme: Theme
    clip: true
    T.ScrollBar.vertical: CatScrollBar {
        theme: control.theme
        parent: control
        x: control.mirrored ? 0 : control.width - width
        y: control.topPadding
        height: control.availableHeight
    }
    T.ScrollBar.horizontal: CatScrollBar {
        theme: control.theme
        parent: control
        x: control.leftPadding
        y: control.height - height
        width: control.availableWidth
    }
}
