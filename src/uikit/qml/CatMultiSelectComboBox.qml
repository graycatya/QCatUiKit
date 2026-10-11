// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Basic as B
import QtQuick.Layouts
import QCatUiKit

CatButton {
    id: control
    property var model: []
    property string textRole: ""
    property var selectedIndexes: []
    property string placeholderText: qsTr("Select options")
    property bool searchEnabled: false
    readonly property int selectedCount: selectedIndexes.length
    readonly property var selectedTexts: selectedIndexes.filter(index => index >= 0 && index
                                                                         < model.length).map(index
                                                                                             => labelAt(
                                                                                                    index))
    signal selectionEdited(var indexes)
    text: selectedCount ? selectedTexts.join(", ") : placeholderText
    rightPadding: tokens.padding + 20
    Text {
        text: "▾"
        x: control.width - width - 8
        anchors.verticalCenter: parent.verticalCenter
        color: control.tokens.textSecondary
    }
    function labelAt(index: int): string {
        const entry = model[index];
        return String(textRole ? entry[textRole] : entry);
    }
    function toggleIndex(index: int): void {
        if (index < 0 || index >= model.length)
            return;
        const next = selectedIndexes.slice();
        const present = next.indexOf(index);
        if (present < 0)
            next.push(index);
        else
            next.splice(present, 1);
        selectedIndexes = next.sort((a, b) => a - b);
        selectionEdited(selectedIndexes);
    }
    onClicked: selectionPopup.open()
    CatPopup {
        id: selectionPopup
        theme: control.theme
        y: control.height + 4
        width: Math.max(200, control.width)
        modal: false
        contentItem: ColumnLayout {
            CatTextField {
                id: filter
                theme: control.theme
                visible: control.searchEnabled
                Layout.fillWidth: true
                placeholderText: qsTr("Search options")
            }
            B.ScrollView {
                Layout.fillWidth: true
                Layout.preferredHeight: Math.min(240, options.implicitHeight)
                contentWidth: availableWidth
                Column {
                    id: options
                    width: parent.width
                    Repeater {
                        model: control.model
                        CatCheckBox {
                            required property int index
                            theme: control.theme
                            width: options.width
                            text: control.labelAt(index)
                            visible: !filter.text.length || text.toLocaleLowerCase().includes(
                                         filter.text.toLocaleLowerCase())
                            checked: control.selectedIndexes.includes(index)
                            onClicked: control.toggleIndex(index)
                        }
                    }
                }
            }
        }
    }
}
