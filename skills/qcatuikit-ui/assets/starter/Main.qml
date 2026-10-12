// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Basic as B
import QCatUiKit

CatWindow {
    id: window
    width: 1040
    height: 760
    minimumWidth: 360
    minimumHeight: 540
    visible: true
    title: qsTr("Records")
    readonly property bool compact: width < 760
    property bool attempted: false

    function addRecord(): void {
        attempted = true;
        const name = nameField.text.trim();
        if (!name.length)
            return;
        records.append({name: name, quantity: quantity.value, state: qsTr("Ready")});
        nameField.clear();
        attempted = false;
    }

    TableModel {
        id: records
        objectName: "records"
        columns: ["name", "quantity", "state"]
        headers: [qsTr("Name"), qsTr("Quantity"), qsTr("State")]
    }

    CatScrollView {
        id: page
        anchors.fill: parent
        anchors.margins: Theme.tokens.padding
        contentWidth: availableWidth
        rightPadding: Theme.tokens.spacing * 2
        B.ScrollBar.horizontal.visible: false
        B.ScrollBar.vertical.visible: B.ScrollBar.vertical.size < 1

        ColumnLayout {
            width: page.availableWidth
            spacing: Theme.tokens.spacing * 2

            CatLabel {
                Layout.fillWidth: true
                text: qsTr("Records")
                font.pixelSize: Theme.tokens.fontSize * 2
                font.bold: true
            }
            CatLabel {
                Layout.fillWidth: true
                text: qsTr("Add a named record, review the table, and adjust the display to your device.")
                wrapMode: Text.Wrap
                color: Theme.tokens.textSecondary
            }
            RowLayout {
                Layout.fillWidth: true
                spacing: Theme.tokens.spacing
                CatSwitch {
                    text: qsTr("Dark")
                    checked: Theme.colorScheme === ThemePalette.Dark
                    onToggled: Theme.colorScheme = checked ? ThemePalette.Dark : ThemePalette.Light
                }
                CatComboBox {
                    objectName: "densityPicker"
                    Layout.fillWidth: true
                    Layout.maximumWidth: 220
                    model: [qsTr("Desktop"), qsTr("Touch"), qsTr("Mobile")]
                    currentIndex: Theme.density
                    onActivated: index => Theme.density = index
                    Accessible.name: qsTr("Interaction density")
                }
            }

            GridLayout {
                Layout.fillWidth: true
                columns: window.compact ? 1 : 2
                columnSpacing: Theme.tokens.spacing * 2
                rowSpacing: Theme.tokens.spacing * 2

                CatPane {
                    Layout.fillWidth: true
                    Layout.preferredWidth: 300
                    Layout.alignment: Qt.AlignTop
                    contentItem: ColumnLayout {
                        spacing: Theme.tokens.spacing
                        CatLabel { text: qsTr("New record"); font.bold: true }
                        CatLabel { text: qsTr("Name") }
                        CatTextField {
                            id: nameField
                            objectName: "recordName"
                            Layout.fillWidth: true
                            placeholderText: qsTr("Enter a name")
                            invalid: window.attempted && !text.trim().length
                            Accessible.name: qsTr("Record name")
                            onAccepted: window.addRecord()
                        }
                        CatLabel {
                            Layout.fillWidth: true
                            visible: nameField.invalid
                            text: qsTr("Enter a name before adding the record.")
                            wrapMode: Text.Wrap
                            color: Theme.tokens.danger
                        }
                        CatLabel { text: qsTr("Quantity") }
                        CatSpinBox {
                            id: quantity
                            Layout.fillWidth: true
                            from: 1; to: 999; value: 1
                            editable: true
                            Accessible.name: qsTr("Quantity")
                        }
                        CatButton {
                            objectName: "addRecord"
                            Layout.fillWidth: true
                            text: qsTr("Add record")
                            primary: true
                            onClicked: window.addRecord()
                        }
                    }
                }

                CatPane {
                    Layout.fillWidth: true
                    Layout.preferredWidth: 600
                    Layout.alignment: Qt.AlignTop
                    contentItem: ColumnLayout {
                        spacing: Theme.tokens.spacing
                        CatLabel {
                            Layout.fillWidth: true
                            text: qsTr("Records: %1").arg(records.count)
                            font.bold: true
                        }
                        CatLabel {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 100
                            visible: records.count === 0
                            text: qsTr("No records yet. Add one using the form.")
                            wrapMode: Text.Wrap
                            color: Theme.tokens.textSecondary
                        }
                        CatTableView {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 300
                            visible: records.count > 0
                            model: records
                            resizableColumns: true
                        }
                        Flow {
                            Layout.fillWidth: true
                            spacing: Theme.tokens.spacing
                            CatButton {
                                objectName: "deleteLatest"
                                text: qsTr("Delete latest")
                                enabled: records.count > 0
                                onClicked: records.remove(records.count - 1)
                            }
                            CatButton {
                                text: qsTr("Clear")
                                enabled: records.count > 0
                                onClicked: records.clear()
                            }
                        }
                    }
                }
            }
        }
    }
}
