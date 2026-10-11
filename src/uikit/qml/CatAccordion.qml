// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQml.Models
import QtQuick.Templates as T
import QtQuick.Controls.Basic as B
import QtQuick.Layouts
import QCatUiKit

Column {
    id: control
    property ThemePalette theme: Theme
    property bool singleExpanded: true
    spacing: (theme || Theme).tokens.spacing
    readonly property int count: children.filter(item => item instanceof CatExpander).length
    Instantiator {
        model: control.children.filter(item => item instanceof CatExpander)
        delegate: Connections {
            required property var modelData
            target: modelData
            function onExpandedChanged() {
                if (!control.singleExpanded || !modelData.expanded)
                    return;
                for (const item of control.children)
                    if (item !== modelData && item instanceof CatExpander)
                        item.expanded = false;
            }
        }
    }
}
