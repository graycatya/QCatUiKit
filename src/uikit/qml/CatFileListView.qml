// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QCatUiKit
import QtQuick

CatListView {
    id: control
    textRole: "name"
    signal fileActivated(string url)
    onActivated: index => {
        if (Array.isArray(model) && model[index] && model[index].url)
            fileActivated(String(model[index].url));
    }
    delegate: CatButton {
        id: entry
        required property int index
        required property var model
        theme: control.theme
        width: control.width
        checked: control.currentIndex === index
        contentItem: Column {
            spacing: 4
            CatLabel {
                theme: control.theme
                width: parent.width
                text: control.labelFor(entry.model)
                elide: Text.ElideRight
            }
            CatLabel {
                theme: control.theme
                width: parent.width
                text: String(entry.model.description || "")
                elide: Text.ElideRight
                color: control.tokens.textSecondary
                visible: text.length > 0
            }
        }
        onClicked: control.select(index)
    }
}
