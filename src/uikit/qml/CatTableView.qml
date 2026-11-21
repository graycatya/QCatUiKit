// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QCatUiKit
import QtQml.Models
import QtQuick.Controls.Basic as B

Item {
    id: control
    property ThemePalette theme: Theme
    readonly property var tokens: (theme || Theme).tokens
    property var model: null
    property Component delegate: defaultCell
    property Component editDelegate: defaultEditDelegate
    property Component headerDelegate: null
    property Component toolbar: null
    property Component emptyState: null
    property Component loadingState: null
    property bool loading: false
    property alias columnWidthProvider: plainView.columnWidthProvider
    property alias rowHeightProvider: plainView.rowHeightProvider
    property alias resizableColumns: plainView.resizableColumns
    readonly property TableView view: __frozen ? frozenMainView : plainView
    property int frozenColumnCount: 0
    property int frozenRowCount: 0
    readonly property bool __rtl: control.LayoutMirroring.enabled
    readonly property var __viewModel: __rtl ? rtlProxy : control.model
    function mapToSource(index) {
        return __rtl && rtlProxy ? rtlProxy.mapToSource(index) : index;
    }
    function mapFromSource(index) {
        return __rtl && rtlProxy ? rtlProxy.mapFromSource(index) : index;
    }
    implicitWidth: 480
    implicitHeight: 280

    readonly property int __columns: plainView.columns
    readonly property int __rows: plainView.rows
    readonly property int __frozenColumns: Math.max(0, Math.min(frozenColumnCount, __columns))
    readonly property int __frozenRows: Math.max(0, Math.min(frozenRowCount, __rows))
    readonly property bool __frozen: __frozenColumns > 0 || __frozenRows > 0
    readonly property real __defaultColumnWidth: 140
    readonly property real __cellSpacing: 1
    readonly property real __frozenX: __rtl ? width - __frozenWidth : 0
    readonly property real __mainX: __rtl ? 0 : __frozenWidth
    property var __columnWidths: ({})
    property real __implicitRowHeight: 0
    property bool __wasFrozen: false
    property bool __sizesCaptured: false
    property int __captureAttempts: 0
    property int __rtlAnchorAttempts: 0
    property bool __editing: false
    property var __editIndex: null

    ColumnReverseProxyModel {
        id: rtlProxy
        sourceModel: control.__rtl ? control.model : null
    }

    Timer {
        interval: 16
        repeat: true
        running: control.__rtl && control.__rtlAnchorAttempts < 60
        onTriggered: {
            control.__rtlAnchorAttempts += 1;
            control.__anchorRtlScroll();
        }
    }
    Component.onCompleted: {
        control.__applyHeaderDelegate();
        Qt.callLater(control.__anchorRtlScroll);
    }

    function __handleKey(event) {
        if (!control.__frozen || control.__editing
                || control.__columns <= 0 || control.__rows <= 0)
            return;
        const current = sharedSelection.currentIndex;
        let row = current && current.row >= 0 ? current.row : 0;
        let column = current && current.column >= 0 ? current.column : 0;
        switch (event.key) {
        case Qt.Key_Left:
            column = Math.max(0, column - 1);
            break;
        case Qt.Key_Right:
            column = Math.min(control.__columns - 1, column + 1);
            break;
        case Qt.Key_Up:
            row = Math.max(0, row - 1);
            break;
        case Qt.Key_Down:
            row = Math.min(control.__rows - 1, row + 1);
            break;
        case Qt.Key_Home:
            column = control.__rtl ? control.__columns - 1 : 0;
            break;
        case Qt.Key_End:
            column = control.__rtl ? 0 : control.__columns - 1;
            break;
        default:
            return;
        }
        event.accepted = true;
        const next = control.__viewModel.index(row, column);
        if (next.row >= 0) {
            sharedSelection.setCurrentIndex(
                        next, ItemSelectionModel.ClearAndSelect | ItemSelectionModel.Current);
            control.__ensureCellVisible(row, column);
        }
    }

    Connections {
        target: control.__frozen ? frozenMainView : plainView
        function onContentWidthChanged() { control.__anchorRtlScroll(); }
    }

    onModelChanged: {
        control.__columnWidths = ({});
        control.__implicitRowHeight = 0;
        control.__sizesCaptured = false;
        control.__captureAttempts = 0;
        control.__rtlAnchorAttempts = 0;
    }

    function __columnWidth(column: int): real {
        const stored = __columnWidths[column];
        if (typeof stored === "number" && stored >= 0)
            return stored;
        const provider = columnWidthProvider;
        if (typeof provider === "function") {
            const value = provider(column);
            if (typeof value === "number" && value >= 0)
                return value;
        }
        return __defaultColumnWidth;
    }
    function __setColumnWidth(column: int, value: real) {
        const width = Math.max(40, Math.round(value));
        const next = Object.assign({}, __columnWidths);
        next[column] = width;
        __columnWidths = next;
        __relayout();
    }
    function __seedWidths() {
        if (typeof columnWidthProvider !== "function") {
            const next = Object.assign({}, __columnWidths);
            for (let column = 0; column < __columns; ++column) {
                if (typeof next[column] === "number")
                    continue;
                const explicit = plainView.explicitColumnWidth(column);
                if (explicit >= 0) {
                    next[column] = explicit;
                    continue;
                }
                const implicit = plainView.implicitColumnWidth(column);
                if (implicit > 0)
                    next[column] = implicit;
            }
            __columnWidths = next;
        }
        if (typeof rowHeightProvider !== "function"
                && __implicitRowHeight <= 0 && plainView.topRow >= 0) {
            const height = plainView.implicitRowHeight(plainView.topRow);
            if (height > 0)
                __implicitRowHeight = height;
        }
    }
    function __applyWidths() {
        for (const key in __columnWidths) {
            const column = Number(key);
            const width = __columnWidths[key];
            if (column >= 0 && column < __columns && typeof width === "number")
                plainView.setColumnWidth(column, width);
        }
        __columnWidths = ({});
        plainView.forceLayout();
    }
    function __handleFrozenChange() {
        if (__frozen && !__wasFrozen)
            __seedWidths();
        else if (!__frozen && __wasFrozen)
            __applyWidths();
        if (__frozen) {
            __sizesCaptured = false;
            __captureAttempts = 0;
        }
        __rtlAnchorAttempts = 0;
        __wasFrozen = __frozen;
        __relayout();
        Qt.callLater(__anchorRtlScroll);
    }
    function __captureImplicitSizes() {
        __captureAttempts += 1;
        let captured = false;
        if (typeof columnWidthProvider !== "function") {
            const next = Object.assign({}, __columnWidths);
            let changed = false;
            for (let column = 0; column < __columns; ++column) {
                if (typeof next[column] === "number")
                    continue;
                const pane = column < __frozenColumns
                        ? (frozenBottomLeftView.visible ? frozenBottomLeftView
                           : frozenTopLeftView.visible ? frozenTopLeftView : null)
                        : (frozenMainView.visible ? frozenMainView
                           : frozenTopRightView.visible ? frozenTopRightView : null);
                if (!pane)
                    continue;
                const width = pane.implicitColumnWidth(column);
                if (width > 0) {
                    next[column] = width;
                    changed = true;
                }
            }
            if (changed) {
                __columnWidths = next;
                captured = true;
            }
        }
        if (typeof rowHeightProvider !== "function" && __implicitRowHeight <= 0) {
            const pane = frozenMainView.visible ? frozenMainView
                    : frozenBottomLeftView.visible ? frozenBottomLeftView
                    : frozenTopRightView.visible ? frozenTopRightView
                    : frozenTopLeftView.visible ? frozenTopLeftView : null;
            if (pane && pane.topRow >= 0) {
                const height = pane.implicitRowHeight(pane.topRow);
                if (height > 0) {
                    __implicitRowHeight = height;
                    captured = true;
                }
            }
        }
        if (captured) {
            __sizesCaptured = true;
            __relayout();
        }
    }
    function freezeTo(row: int, column: int) {
        frozenRowCount = Math.max(0, row + 1);
        frozenColumnCount = Math.max(0, column + 1);
    }
    function clearFreeze() {
        frozenRowCount = 0;
        frozenColumnCount = 0;
    }
    function __anchorRtlScroll() {
        if (!__rtl)
            return;
        const view = __frozen ? frozenMainView : plainView;
        if (view && view.columns > 0 && view.contentWidth > view.width)
            view.contentX = view.contentWidth - view.width;
    }
    function sourceColumn(visualColumn: int): int {
        if (visualColumn < 0 || visualColumn >= __columns)
            return visualColumn;
        return __rtl ? __columns - 1 - visualColumn : visualColumn;
    }
    function __paneAt(row: int, column: int) {
        if (!__frozen)
            return plainView;
        const frozenColumn = __rtl ? column >= __columns - __frozenColumns
                                   : column < __frozenColumns;
        const frozenRow = row < __frozenRows;
        if (frozenColumn && frozenRow)
            return frozenTopLeftView;
        if (frozenColumn)
            return frozenBottomLeftView;
        if (frozenRow)
            return frozenTopRightView;
        return frozenMainView;
    }
    function __closeEditors() {
        plainView.closeEditor();
        frozenTopLeftView.closeEditor();
        frozenTopRightView.closeEditor();
        frozenBottomLeftView.closeEditor();
        frozenMainView.closeEditor();
        __editing = false;
    }
    function edit(row: int, column: int): bool {
        if (row < 0 || column < 0 || row >= __rows || column >= __columns)
            return false;
        __closeEditors();
        const index = __viewModel.index(row, column);
        if (!index || index.row < 0)
            return false;
        __editIndex = index;
        const pane = __paneAt(row, column);
        if (!pane)
            return false;
        void pane.edit(index);
        __editing = true;
        return true;
    }
    function commitEdit(value): bool {
        const index = __editIndex && __editIndex.row >= 0
                ? __editIndex : sharedSelection.currentIndex;
        if (!index || index.row < 0)
            return false;
        const accepted = __viewModel.setData(index, value, Qt.EditRole);
        if (accepted) {
            __closeEditors();
            __editIndex = null;
        }
        return accepted;
    }
    function cancelEdit() {
        __closeEditors();
        __editIndex = null;
    }
    function __ensureCellVisible(row: int, column: int) {
        if (!__frozen || !frozenMainView.visible)
            return;
        const frozenColumn = __rtl ? column >= __columns - __frozenColumns
                                   : column < __frozenColumns;
        const frozenRow = row < __frozenRows;
        const rowOut = !frozenRow
                && (row < frozenMainView.topRow || row > frozenMainView.bottomRow);
        const columnOut = !frozenColumn
                && (column < frozenMainView.leftColumn || column > frozenMainView.rightColumn);
        if (rowOut || columnOut)
            frozenMainView.positionViewAtCell(Qt.point(row, column), TableView.Contain);
    }
    function __rowHeight(row: int): real {
        const provider = rowHeightProvider;
        if (typeof provider === "function") {
            const value = provider(row);
            if (typeof value === "number" && value >= 0)
                return value;
        }
        return __implicitRowHeight > 0 ? __implicitRowHeight : tokens.controlHeight;
    }
    function __paneColumnWidth(frozenPane: bool, column: int): real {
        const frozen = __rtl ? column >= __columns - __frozenColumns
                             : column < __frozenColumns;
        if (frozenPane !== frozen)
            return 0;
        return __columnWidth(__rtl ? __columns - 1 - column : column);
    }
    function __paneRowHeight(frozenPane: bool, row: int): real {
        const frozen = row < __frozenRows;
        if (frozenPane !== frozen)
            return 0;
        return __rowHeight(row);
    }
    function __relayout() {
        if (frozenTopLeftView.visible)
            frozenTopLeftView.forceLayout();
        if (frozenTopRightView.visible)
            frozenTopRightView.forceLayout();
        if (frozenBottomLeftView.visible)
            frozenBottomLeftView.forceLayout();
        if (frozenMainView.visible)
            frozenMainView.forceLayout();
    }
    function __applyHeaderDelegate() {
        if (!plainHeader)
            return;
        plainHeader.delegate = headerDelegate ? headerDelegate : defaultHeaderCell;
        frozenHeader.delegate = headerDelegate ? headerDelegate : frozenHeaderCell;
        frozenMainHeader.delegate = headerDelegate ? headerDelegate : frozenHeaderCell;
        plainHeader.forceLayout();
        frozenHeader.forceLayout();
        frozenMainHeader.forceLayout();
    }
    onHeaderDelegateChanged: __applyHeaderDelegate()
    readonly property real __frozenWidth: {
        let total = 0;
        for (let column = 0; column < __frozenColumns; ++column)
            total += __columnWidth(column) + (column > 0 ? __cellSpacing : 0);
        return total;
    }
    readonly property real __frozenHeight: {
        let total = 0;
        for (let row = 0; row < __frozenRows; ++row)
            total += __rowHeight(row) + (row > 0 ? __cellSpacing : 0);
        return total;
    }

    Timer {
        interval: 100
        repeat: true
        running: control.__frozen && !control.__sizesCaptured
                 && control.__captureAttempts < 5
        onTriggered: control.__captureImplicitSizes()
    }

    Connections {
        target: control
        function onFrozenColumnCountChanged() { control.__handleFrozenChange(); }
        function onFrozenRowCountChanged() { control.__handleFrozenChange(); }
        function onColumnWidthProviderChanged() {
            control.__columnWidths = ({});
            control.__relayout();
        }
        function onRowHeightProviderChanged() {
            control.__implicitRowHeight = 0;
            control.__relayout();
        }
    }

    ItemSelectionModel {
        id: sharedSelection
        model: control.__viewModel
    }

    Component {
        id: defaultCell
        CatTableCell {
            theme: control.theme
            editDelegate: control.editDelegate
        }
    }

    Component {
        id: defaultEditDelegate
        TextInput {
            objectName: "tableEditField"
            required property var display
            required property int row
            required property int column
            anchors.fill: parent
            anchors.margins: 8
            text: display === undefined ? "" : String(display)
            font.pixelSize: control.tokens.fontSize
            color: control.tokens.text
            selectionColor: control.tokens.primary
            selectByMouse: true
            clip: true
            verticalAlignment: TextInput.AlignVCenter
            function commitText() {
                if (!control.__editing)
                    return;
                const index = control.__viewModel.index(row, column);
                if (index.row >= 0)
                    control.__viewModel.setData(index, text, Qt.EditRole);
                if (TableView.view)
                    TableView.view.closeEditor();
                control.__editing = false;
                control.__editIndex = null;
            }
            Component.onCompleted: {
                control.__editing = true;
                forceActiveFocus();
                selectAll();
            }
            Component.onDestruction: control.__editing = false
            TableView.onCommit: commitText()
            Keys.onReturnPressed: commitText()
            Keys.onEnterPressed: commitText()
            Keys.onEscapePressed: {
                if (TableView.view)
                    TableView.view.closeEditor();
                control.__editing = false;
                control.__editIndex = null;
            }
        }
    }

    Component {
        id: defaultHeaderCell
        CatTableHeaderCell {
            theme: control.theme
        }
    }

    Component {
        id: frozenHeaderCell
        CatTableHeaderCell {
            id: headerCell
            theme: control.theme
            Rectangle {
                id: resizeHandle
                width: 6
                anchors {
                    top: parent.top
                    bottom: parent.bottom
                    left: control.__rtl ? parent.left : undefined
                    right: control.__rtl ? undefined : parent.right
                }
                color: handleHover.hovered || resizeDrag.active ? control.tokens.primary : "transparent"
                HoverHandler {
                    id: handleHover
                    enabled: control.resizableColumns
                    cursorShape: Qt.SplitHCursor
                }
                DragHandler {
                    id: resizeDrag
                    enabled: control.resizableColumns
                    target: null
                    xAxis.enabled: true
                    yAxis.enabled: false
                    grabPermissions: PointerHandler.CanTakeOverFromItems
                    property real startWidth: 0
                    property real startSceneX: 0
                    onActiveChanged: {
                        if (active) {
                            const sourceColumn = control.__rtl
                                    ? control.__columns - 1 - headerCell.column
                                    : headerCell.column;
                            startWidth = control.__columnWidth(sourceColumn);
                            startSceneX = centroid.scenePosition.x;
                        }
                    }
                    onTranslationChanged: {
                        if (active) {
                            const sourceColumn = control.__rtl
                                    ? control.__columns - 1 - headerCell.column
                                    : headerCell.column;
                            const dx = control.__rtl
                                    ? centroid.scenePosition.x - startSceneX
                                    : translation.x;
                            const delta = control.__rtl ? -dx : dx;
                            control.__setColumnWidth(sourceColumn, startWidth + delta);
                        }
                    }
                }
            }
        }
    }

    Item {
        id: toolbarHost
        objectName: "tableToolbarHost"
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        height: toolbarLoader.childrenRect.height
        visible: height > 0
        Loader {
            id: toolbarLoader
            anchors.fill: parent
            sourceComponent: control.toolbar
        }
    }

    Item {
        id: plainArea
        anchors {
            top: toolbarHost.bottom
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
        visible: !control.__frozen

        B.HorizontalHeaderView {
            id: plainHeader
            objectName: "plainHeader"
            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
            }
            clip: true
            columnSpacing: control.__cellSpacing
            syncView: plainView
            delegate: control.headerDelegate ? control.headerDelegate : defaultHeaderCell
        }
        TableView {
            id: plainView
            anchors {
                top: plainHeader.bottom
                left: parent.left
                right: parent.right
                bottom: parent.bottom
            }
            clip: true
            columnSpacing: control.__cellSpacing
            rowSpacing: control.__cellSpacing
            resizableColumns: true
            boundsBehavior: Flickable.StopAtBounds
            model: control.__viewModel
            editTriggers: TableView.DoubleTapped | TableView.EditKeyPressed
            delegate: control.delegate
            selectionModel: sharedSelection
            MouseScrollImpl {
                view: plainView
            }
            B.ScrollBar.vertical: B.ScrollBar {
                LayoutMirroring.enabled: control.__rtl
            }
            B.ScrollBar.horizontal: B.ScrollBar {
                LayoutMirroring.enabled: control.__rtl
            }
        }
    }

    Item {
        id: frozenArea
        objectName: "frozenArea"
        anchors {
            top: toolbarHost.bottom
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
        visible: control.__frozen

        Item {
            id: frozenHeaderArea
            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
            }
            height: control.tokens.controlHeight
            clip: true

            B.HorizontalHeaderView {
                id: frozenHeader
                objectName: "frozenHeader"
                x: control.__frozenX
                width: control.__frozenWidth
                height: parent.height
                visible: control.__frozenColumns > 0
                clip: true
                columnSpacing: control.__cellSpacing
                syncView: frozenBottomLeftView
                resizableColumns: false
                delegate: control.headerDelegate ? control.headerDelegate : frozenHeaderCell
            }
            B.HorizontalHeaderView {
                id: frozenMainHeader
                objectName: "frozenMainHeader"
                x: control.__mainX
                width: Math.max(0, parent.width - control.__frozenWidth)
                height: parent.height
                clip: true
                columnSpacing: control.__cellSpacing
                syncView: frozenMainView
                resizableColumns: false
                delegate: control.headerDelegate ? control.headerDelegate : frozenHeaderCell
            }
        }

        Item {
            id: frozenContentArea
            objectName: "frozenContentArea"
            anchors {
                top: frozenHeaderArea.bottom
                left: parent.left
                right: parent.right
                bottom: parent.bottom
            }
            clip: true

            TableView {
                id: frozenTopLeftView
                objectName: "frozenTopLeftView"
                x: control.__frozenX
                y: 0
                width: control.__frozenWidth
                height: control.__frozenHeight
                visible: control.__frozenColumns > 0 && control.__frozenRows > 0
                         && width > 0 && height > 0
                clip: true
                columnSpacing: control.__cellSpacing
                rowSpacing: control.__cellSpacing
                interactive: false
                boundsBehavior: Flickable.StopAtBounds
                model: control.__viewModel
                    editTriggers: TableView.DoubleTapped | TableView.EditKeyPressed
                delegate: control.delegate
                selectionModel: sharedSelection
                columnWidthProvider: function(column) { return control.__paneColumnWidth(true, column); }
                rowHeightProvider: function(row) { return control.__paneRowHeight(true, row); }
                resizableColumns: false
                keyNavigationEnabled: true
                Keys.priority: Keys.BeforeItem
                Keys.onPressed: (event) => control.__handleKey(event)
            }
            TableView {
                id: frozenTopRightView
                objectName: "frozenTopRightView"
                x: control.__mainX
                y: 0
                width: Math.max(0, frozenContentArea.width - control.__frozenWidth)
                height: control.__frozenHeight
                visible: control.__frozenRows > 0 && width > 0 && height > 0
                clip: true
                columnSpacing: control.__cellSpacing
                rowSpacing: control.__cellSpacing
                interactive: false
                boundsBehavior: Flickable.StopAtBounds
                syncView: frozenMainView
                syncDirection: Qt.Horizontal
                model: control.__viewModel
                    editTriggers: TableView.DoubleTapped | TableView.EditKeyPressed
                delegate: control.delegate
                selectionModel: sharedSelection
                columnWidthProvider: function(column) { return control.__paneColumnWidth(false, column); }
                rowHeightProvider: function(row) { return control.__paneRowHeight(true, row); }
                resizableColumns: false
                keyNavigationEnabled: true
                Keys.priority: Keys.BeforeItem
                Keys.onPressed: (event) => control.__handleKey(event)
            }
            TableView {
                id: frozenBottomLeftView
                objectName: "frozenBottomLeftView"
                x: control.__frozenX
                y: control.__frozenHeight
                width: control.__frozenWidth
                height: Math.max(0, frozenContentArea.height - control.__frozenHeight)
                visible: control.__frozenColumns > 0 && width > 0 && height > 0
                clip: true
                columnSpacing: control.__cellSpacing
                rowSpacing: control.__cellSpacing
                interactive: false
                boundsBehavior: Flickable.StopAtBounds
                syncView: frozenMainView
                syncDirection: Qt.Vertical
                model: control.__viewModel
                    editTriggers: TableView.DoubleTapped | TableView.EditKeyPressed
                delegate: control.delegate
                selectionModel: sharedSelection
                columnWidthProvider: function(column) { return control.__paneColumnWidth(true, column); }
                rowHeightProvider: function(row) { return control.__paneRowHeight(false, row); }
                resizableColumns: false
                keyNavigationEnabled: true
                Keys.priority: Keys.BeforeItem
                Keys.onPressed: (event) => control.__handleKey(event)
            }
            TableView {
                id: frozenMainView
                objectName: "frozenMainView"
                x: control.__mainX
                y: control.__frozenHeight
                width: Math.max(0, frozenContentArea.width - control.__frozenWidth)
                height: Math.max(0, frozenContentArea.height - control.__frozenHeight)
                visible: control.__frozen && width > 0 && height > 0
                clip: true
                columnSpacing: control.__cellSpacing
                rowSpacing: control.__cellSpacing
                boundsBehavior: Flickable.StopAtBounds
                model: control.__viewModel
                    editTriggers: TableView.DoubleTapped | TableView.EditKeyPressed
                delegate: control.delegate
                selectionModel: sharedSelection
                columnWidthProvider: function(column) { return control.__paneColumnWidth(false, column); }
                rowHeightProvider: function(row) { return control.__paneRowHeight(false, row); }
                resizableColumns: false
                keyNavigationEnabled: true
                Keys.priority: Keys.BeforeItem
                Keys.onPressed: (event) => control.__handleKey(event)
                MouseScrollImpl {
                    view: frozenMainView
                }
                B.ScrollBar.horizontal: B.ScrollBar {
                    LayoutMirroring.enabled: control.__rtl
                    policy: control.__columns > 0 && control.__frozenColumns >= control.__columns
                            ? B.ScrollBar.AlwaysOff : B.ScrollBar.AsNeeded
                }
            }
            B.ScrollBar {
                id: frozenVerticalScrollBar
                objectName: "frozenVerticalScrollBar"
                orientation: Qt.Vertical
                anchors {
                    top: parent.top
                    bottom: parent.bottom
                }
                x: control.__rtl ? 0 : parent.width - width
                z: 20
                policy: B.ScrollBar.AsNeeded
                visible: control.__rows > 0 && control.__frozenRows < control.__rows
                size: frozenMainView.visibleArea.heightRatio
                active: frozenMainView.moving || pressed
                Binding {
                    target: frozenVerticalScrollBar
                    property: "position"
                    value: frozenMainView.visibleArea.yPosition
                    when: !frozenVerticalScrollBar.pressed
                }
                onPositionChanged: if (pressed)
                    frozenMainView.contentY = position * frozenMainView.contentHeight
            }
        }
    }

    Loader {
        id: stateOverlay
        objectName: "tableStateOverlay"
        anchors {
            top: toolbarHost.bottom
            topMargin: control.tokens.controlHeight
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
        z: 30
        visible: item !== null
        sourceComponent: control.loading && control.loadingState
                         ? control.loadingState
                         : (!control.loading && control.__rows === 0
                            ? control.emptyState : null)
    }
}
