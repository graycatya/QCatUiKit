// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import QtQml.Models
import QCatUiKit

ColumnLayout {
    id: control
    objectName: "dataPreview"
    required property string componentId
    readonly property alias dataController: demo
    property string savedDraft: ""
    readonly property bool tableMode: componentId === "table"
    readonly property bool composerMode: componentId === "chatinput"
    spacing: 16
    function countText(value: real): string {
        return value.toLocaleString(Qt.locale(GalleryLanguage.localeName), "f", 0);
    }
    function selectTableRow(row: int): void {
        demo.select(row);
        if (demo.selectedIndex < 0) return;
        table.view.selectionModel.setCurrentIndex(demo.tableModel.index(demo.selectedIndex, 0),
                                                  ItemSelectionModel.ClearAndSelect | ItemSelectionModel.Rows);
        table.view.positionViewAtRow(demo.selectedIndex, TableView.Contain);
    }
    onVisibleChanged: if (!visible) demo.stopStress()
    Component.onDestruction: demo.stopStress()
    GalleryDataDemo {
        id: demo
        objectName: "dataDemo"
        mode: control.tableMode ? 0 : control.composerMode ? 2 : 1
        onRunningChanged: {
            if (!control.composerMode) return;
            if (running) control.savedDraft = composer.text;
            else composer.text = control.savedDraft;
        }
        onSubmitRequested: text => {
            composer.text = text;
            composer.submit();
        }
    }
    Connections {
        target: GalleryLanguage
        function onLanguageChanged() { demo.retranslate(); }
    }
    Connections {
        target: control.Window.window
        function onVisibilityChanged() {
            if (control.Window.window.visibility === Window.Hidden ||
                control.Window.window.visibility === Window.Minimized) demo.stopStress();
        }
    }
    CatLabel {
        Layout.fillWidth: true
        wrapMode: Text.Wrap
        text: control.tableMode
            ? qsTr("Edit real model cells, select and locate rows, resize columns, and keep scrolling while data changes.")
            : control.composerMode
              ? qsTr("Try Send or Enter, whitespace rejection and long single-line text. Continuous submissions use this same composer and appear in the history below.")
              : qsTr("Try incoming and outgoing messages, delivery states, long text and attachment placeholders. Turn off tail following to read history during live updates.")
    }
    Flow {
        Layout.fillWidth: true
        spacing: 8
        CatButton {
            objectName: "dataAddButton"
            text: qsTr("Add a batch")
            visible: !control.composerMode
            enabled: !demo.running
            onClicked: demo.addBatch()
        }
        CatButton {
            objectName: "dataUpdateButton"
            text: control.tableMode ? qsTr("Update a batch") : qsTr("Change delivery states")
            visible: !control.composerMode
            enabled: !demo.running && demo.count > 0
            onClicked: demo.updateBatch()
        }
        CatButton {
            objectName: "dataRemoveButton"
            text: qsTr("Remove latest batch")
            enabled: !demo.running && demo.count > 0
            onClicked: demo.removeBatch()
        }
        CatButton {
            objectName: "dataClearButton"
            text: qsTr("Clear data")
            enabled: !demo.running && demo.count > 0
            onClicked: demo.clearData()
        }
        CatButton {
            objectName: "dataRestoreButton"
            text: qsTr("Restore demo")
            onClicked: demo.resetData()
        }
        CatButton {
            objectName: "dataTopStressButton"
            text: demo.running ? qsTr("Stop stress test") : qsTr("Start stress test")
            primary: true
            onClicked: demo.running ? demo.stopStress() : demo.startStress()
        }
    }
    CatLabel {
        objectName: "dataCountSummary"
        Layout.fillWidth: true
        wrapMode: Text.Wrap
        text: control.tableMode ? qsTr("%1 / %2 rows").arg(control.countText(demo.count)).arg(control.countText(demo.capacity))
                               : qsTr("%1 / %2 messages").arg(control.countText(demo.count)).arg(control.countText(demo.capacity))
    }
    CatTableView {
        id: table
        objectName: "demoTableView"
        visible: control.tableMode
        Layout.fillWidth: true
        Layout.preferredHeight: 320
        model: demo.tableModel
        columnWidthProvider: column => {
            const explicit = table.view.explicitColumnWidth(column);
            return explicit > 0 ? explicit : Math.max(120, (table.width - 4) / 4);
        }
        view.selectionBehavior: TableView.SelectRows
        view.selectionMode: TableView.SingleSelection
        view.pointerNavigationEnabled: true
        view.keyNavigationEnabled: true
        emptyState: tableEmptyState
        loadingState: tableLoadingState
    }
    Connections {
        target: table.view
        function onCurrentRowChanged() {
            if (control.tableMode && table.view.currentRow >= 0) demo.select(table.view.currentRow);
        }
    }
    CatLabel {
        Layout.fillWidth: true
        visible: demo.count === 0
        text: control.tableMode ? qsTr("No rows. Add data to begin.") : qsTr("No messages yet. Send one to begin.")
        wrapMode: Text.Wrap
        color: Theme.tokens.textSecondary
    }
    Flow {
        visible: control.tableMode
        Layout.fillWidth: true
        spacing: 8
        CatCheckBox { text: qsTr("Resizable columns"); checked: true; onToggled: table.resizableColumns = checked }
        CatButton { text: qsTr("First row"); enabled: demo.count > 0; onClicked: control.selectTableRow(0) }
        CatButton { text: qsTr("Last row"); enabled: demo.count > 0; onClicked: control.selectTableRow(demo.count - 1) }
    }
    ColumnLayout {
        visible: control.tableMode
        Layout.fillWidth: true
        CatLabel { text: qsTr("Freeze panes"); font.bold: true }
        Flow {
            Layout.fillWidth: true
            spacing: 8
            CatSpinBox {
                id: frozenColumns
                objectName: "tableFrozenColumns"
                width: 160
                from: 0
                to: Math.max(1, table.view.columns)
                value: table.frozenColumnCount
                editable: true
                onValueModified: table.frozenColumnCount = value
                Accessible.name: qsTr("Frozen columns")
            }
            CatSpinBox {
                id: frozenRows
                objectName: "tableFrozenRows"
                width: 160
                from: 0
                to: Math.min(20, Math.max(1, table.view.rows))
                value: table.frozenRowCount
                editable: true
                onValueModified: table.frozenRowCount = value
                Accessible.name: qsTr("Frozen rows")
            }
            CatButton {
                objectName: "tableFreezeFirst"
                text: qsTr("Freeze first column and row")
                onClicked: table.freezeTo(0, 0)
            }
            CatButton {
                objectName: "tableClearFreeze"
                text: qsTr("Clear freeze")
                enabled: table.frozenColumnCount > 0 || table.frozenRowCount > 0
                onClicked: table.clearFreeze()
            }
            CatCheckBox {
                id: rtlToggle
                objectName: "tableRtlToggle"
                text: qsTr("Right-to-left layout")
                checked: table.LayoutMirroring.enabled
                onToggled: table.LayoutMirroring.enabled = checked
                Accessible.name: qsTr("Right-to-left layout")
            }
            CatButton {
                objectName: "tableRtlLargePreset"
                text: qsTr("Load 100,000 rows in RTL")
                onClicked: {
                    if (demo.running)
                        demo.stopStress();
                    demo.clearData();
                    table.LayoutMirroring.enabled = true;
                    rtlToggle.checked = true;
                    demo.configureLargeData();
                    demo.startStress();
                }
            }
        }
        CatLabel {
            Layout.fillWidth: true
            wrapMode: Text.Wrap
            color: Theme.tokens.textSecondary
            text: qsTr("RTL is applied before loading, avoiding a full model reset.")
        }
    }
    ColumnLayout {
        visible: control.tableMode
        Layout.fillWidth: true
        CatLabel { text: qsTr("Table customization"); font.bold: true }
        Flow {
            Layout.fillWidth: true
            spacing: 8
            CatCheckBox {
                id: customHeaderToggle
                objectName: "tableCustomHeader"
                text: qsTr("Custom header")
                onToggled: table.headerDelegate = checked ? tableCustomHeader : null
                Accessible.name: qsTr("Custom header")
            }
            CatCheckBox {
                id: toolbarToggle
                objectName: "tableToolbar"
                text: qsTr("Table toolbar")
                onToggled: table.toolbar = checked ? tableToolbar : null
                Accessible.name: qsTr("Table toolbar")
            }
            CatCheckBox {
                id: loadingToggle
                objectName: "tableLoading"
                text: qsTr("Show loading")
                onToggled: table.loading = checked
                Accessible.name: qsTr("Show loading")
            }
            CatButton {
                objectName: "tableEmptyDemo"
                text: qsTr("Show empty state")
                enabled: !demo.running
                onClicked: {
                    table.loading = false;
                    loadingToggle.checked = false;
                    demo.clearData();
                }
            }
            CatButton {
                objectName: "tableRestoreRows"
                text: qsTr("Restore rows")
                enabled: !demo.running
                onClicked: demo.resetData()
            }
        }
    }
    Connections {
        target: table
        function onFrozenColumnCountChanged() { frozenColumns.value = table.frozenColumnCount }
        function onFrozenRowCountChanged() { frozenRows.value = table.frozenRowCount }
    }
    ColumnLayout {
        visible: control.tableMode
        Layout.fillWidth: true
        CatLabel { text: qsTr("Select a row, then edit a cell"); font.bold: true }
        Flow {
            Layout.fillWidth: true
            spacing: 8
            CatSpinBox {
                objectName: "tableRowSelector"
                width: 160; from: 1; to: Math.max(1, demo.count); value: demo.selectedIndex + 1
                editable: true; enabled: demo.count > 0
                onValueModified: control.selectTableRow(value - 1)
                Accessible.name: qsTr("Row number")
            }
            CatComboBox {
                id: columnPicker
                objectName: "tableColumnPicker"
                width: 160
                model: [qsTr("Name"), qsTr("Status"), qsTr("Value")]
                Accessible.name: qsTr("Column to edit")
            }
            CatButton {
                objectName: "dataRemoveSelected"
                text: qsTr("Delete selected row")
                enabled: demo.count > 0 && !demo.running
                onClicked: demo.removeSelected()
            }
        }
        CatTextField {
            id: cellEditor
            objectName: "tableCellEditor"
            Layout.fillWidth: true
            enabled: demo.count > 0 && !demo.running
            text: String(demo.selectedRecord[["name", "state", "value"][columnPicker.currentIndex]] ?? "")
            Accessible.name: qsTr("Cell content")
            onAccepted: demo.editCell(columnPicker.currentIndex + 1, text)
        }
        CatButton {
            objectName: "tableApplyCell"
            text: qsTr("Apply cell edit")
            enabled: demo.count > 0 && !demo.running
            onClicked: demo.editCell(columnPicker.currentIndex + 1, cellEditor.text)
        }
    }
    ColumnLayout {
        visible: !control.tableMode
        Layout.fillWidth: true
        Flow {
            Layout.fillWidth: true
            spacing: 8
            CatCheckBox {
                objectName: "chatFollowTail"
                text: qsTr("Follow latest messages"); checked: true
                onToggled: conversation.followTail = checked
            }
            CatButton { text: qsTr("First message"); enabled: demo.count > 0; onClicked: conversation.positionViewAtBeginning() }
            CatButton { text: qsTr("Latest message"); enabled: demo.count > 0; onClicked: conversation.positionViewAtEnd() }
        }
        CatChatView {
            id: conversation
            objectName: "demoChatView"
            Layout.fillWidth: true
            Layout.preferredHeight: 320
            model: demo.chatModel
            currentUser: "me"
            reuseItems: true
        }
        Flow {
            Layout.fillWidth: true
            visible: !control.composerMode
            spacing: 8
            CatComboBox {
                id: messageType
                objectName: "messageTypePicker"
                width: 160
                model: [qsTr("Text"), qsTr("Image placeholder"), qsTr("Audio placeholder"), qsTr("Video placeholder")]
                Accessible.name: qsTr("Message type")
            }
            CatButton {
                objectName: "chatReceiveButton"
                text: qsTr("Receive a message"); enabled: !demo.running
                onClicked: demo.addMessage(true, messageType.currentIndex)
            }
            CatButton {
                objectName: "chatSendButton"
                text: qsTr("Send a sample"); enabled: !demo.running
                onClicked: demo.addMessage(false, messageType.currentIndex)
            }
        }
        CatLabel { text: qsTr("Message length"); visible: !control.tableMode }
        CatSpinBox {
            objectName: "dataPayloadSize"
            Layout.fillWidth: true
            from: 16; to: 4096; value: demo.payloadSize; editable: true; enabled: !demo.running
            onValueModified: demo.payloadSize = value
            Accessible.name: qsTr("Generated message length")
        }
        CatChatInput {
            id: composer
            objectName: "demoChatInput"
            Layout.fillWidth: true
            enabled: !control.composerMode || !demo.running
            onSubmitted: text => demo.recordSubmission(text)
        }
        Flow {
            Layout.fillWidth: true
            spacing: 8
            CatButton {
                objectName: "composerLongText"
                text: qsTr("Load long text"); enabled: !demo.running
                onClicked: composer.text = qsTr("Long text with spaces, numbers 123 and punctuation. ").repeat(100).slice(0, 2048)
            }
            CatButton {
                objectName: "composerWhitespace"
                text: qsTr("Try whitespace"); enabled: !demo.running
                onClicked: composer.text = "   "
            }
            CatButton { text: qsTr("Clear draft"); enabled: !demo.running; onClicked: composer.text = "" }
        }
        CatLabel {
            objectName: "composerDraftSummary"
            Layout.fillWidth: true
            wrapMode: Text.Wrap
            color: Theme.tokens.textSecondary
            text: qsTr("Draft: %1 characters · whitespace-only submissions are ignored").arg(control.countText(composer.text.length))
        }
        ColumnLayout {
            visible: !control.composerMode
            Layout.fillWidth: true
            CatLabel { text: qsTr("Inspect a message and change its delivery state"); wrapMode: Text.Wrap; Layout.fillWidth: true; font.bold: true }
            Flow {
                Layout.fillWidth: true
                spacing: 8
                CatSpinBox {
                    objectName: "chatMessageSelector"
                    width: 160; from: 1; to: Math.max(1, demo.count); value: demo.selectedIndex + 1
                    editable: true; enabled: demo.count > 0
                    onValueModified: {
                        demo.select(value - 1);
                        conversation.positionViewAtIndex(demo.selectedIndex, ListView.Center);
                    }
                    Accessible.name: qsTr("Message number")
                }
                CatComboBox {
                    objectName: "chatStatusPicker"
                    width: 160
                    model: [qsTr("Pending"), qsTr("Sent"), qsTr("Failed")]
                    currentIndex: demo.selectedRecord.status ?? 0
                    enabled: demo.count > 0 && !demo.running
                    onActivated: index => demo.setMessageStatus(index)
                    Accessible.name: qsTr("Delivery state")
                }
                CatButton {
                    objectName: "chatRemoveSelected"
                    text: qsTr("Delete selected message"); enabled: demo.count > 0 && !demo.running
                    onClicked: demo.removeSelected()
                }
            }
            CatLabel {
                objectName: "chatSelectedMessage"
                Layout.fillWidth: true
                wrapMode: Text.Wrap
                maximumLineCount: 3; elide: Text.ElideRight
                text: demo.count ? String(demo.selectedRecord.username) + ": " + String(demo.selectedRecord.content) : ""
            }
            CatLabel {
                Layout.fillWidth: true
                wrapMode: Text.Wrap
                color: Theme.tokens.textSecondary
                text: qsTr("Image, audio and video entries demonstrate attachment metadata; this component does not download or play media.")
            }
        }
    }
    GalleryDataStressPanel { demo: control.dataController }

    Component {
        id: tableCustomHeader
        CatTableHeaderCell {
            objectName: "tableCustomHeaderCell"
            property string marker: "custom"
            Rectangle {
                width: 4
                height: parent.height
                anchors.right: parent.right
                color: Theme.tokens.primary
            }
        }
    }

    Component {
        id: tableToolbar
        Rectangle {
            objectName: "tableCustomToolbar"
            height: 40
            implicitHeight: 40
            color: Theme.tokens.surfaceHover
            RowLayout {
                anchors.fill: parent
                anchors.margins: 8
                CatLabel { text: qsTr("Custom toolbar") }
                Item { Layout.fillWidth: true }
                CatButton { text: qsTr("Toolbar action") }
            }
        }
    }

    Component {
        id: tableEmptyState
        Rectangle {
            objectName: "tableEmptyState"
            color: Theme.tokens.surface
            ColumnLayout {
                anchors.centerIn: parent
                CatLabel { text: qsTr("No rows to display."); font.bold: true }
            }
        }
    }

    Component {
        id: tableLoadingState
        Rectangle {
            objectName: "tableLoadingState"
            color: Theme.tokens.surface
            CatLabel {
                anchors.centerIn: parent
                text: qsTr("Loading rows...")
            }
        }
    }
}
