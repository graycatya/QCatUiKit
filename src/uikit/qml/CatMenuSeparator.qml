// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Templates as T
import QCatUiKit

T.MenuSeparator {
    id: control
    property ThemePalette theme: Theme
    padding: 4
    implicitWidth: 180
    implicitHeight: 9
    contentItem: Rectangle {
        implicitHeight: 1
        color: (control.theme || Theme).tokens.border
    }
}
