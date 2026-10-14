// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Basic as B
import QtQuick.Dialogs as Dialogs
import QtQuick.Layouts
import QCatUiKit

Item {
    id: control
    required property string componentId
    implicitHeight: Math.max(240, sceneLoader.implicitHeight + 48)
    Loader {
        id: sceneLoader
        anchors.centerIn: parent
        width: Math.max(0, control.width - 8)
        sourceComponent: {
            switch (control.componentId) {
            case "accordion":
                return scene0;
            case "annotated-scrollbar":
                return scene1;
            case "auto-suggest-box":
                return scene2;
            case "avatar":
                return scene3;
            case "breadcrumb":
                return scene4;
            case "calendar-date-picker":
                return scene5;
            case "flip-view":
                return scene6;
            case "coach-mark":
                return scene7;
            case "color-picker":
                return scene8;
            case "command-bar":
                return scene9;
            case "command-bar-flyout":
                return scene10;
            case "compound-button":
                return scene11;
            case "content-dialog":
                return scene12;
            case "date-picker":
                return scene13;
            case "dialog":
                return scene14;
            case "divider":
                return scene15;
            case "dropdown-button":
                return scene16;
            case "expander":
                return scene17;
            case "field":
                return scene18;
            case "file-drop-zone":
                return scene19;
            case "file-list-view":
                return scene20;
            case "flow-view":
                return scene21;
            case "flyout":
                return scene22;
            case "font-icon":
                return scene23;
            case "grid-view":
                return scene24;
            case "hyperlink-button":
                return scene25;
            case "info-badge":
                return scene26;
            case "list-view":
                return scene27;
            case "menu":
                return scene28;
            case "menu-bar":
                return scene29;
            case "menubaritem":
                return scene30;
            case "menuitem":
                return scene31;
            case "menuseparator":
                return scene32;
            case "multi-select-combobox":
                return scene33;
            case "navigation-view":
                return scene34;
            case "number-box":
                return scene35;
            case "particle-backdrop":
                return scene36;
            case "password-box":
                return scene37;
            case "pips-pager":
                return scene38;
            case "pivot":
                return scene39;
            case "progress-ring":
                return scene40;
            case "rating-control":
                return scene41;
            case "repeat-button":
                return scene42;
            case "scrollbar":
                return scene43;
            case "scroll-view":
                return scene44;
            case "selector-bar":
                return scene45;
            case "shimmer":
                return scene46;
            case "spatial-item":
                return scene47;
            case "spatial-view":
                return scene48;
            case "spinbox":
                return scene49;
            case "splash-screen":
                return scene50;
            case "split-button":
                return scene51;
            case "split-view":
                return scene52;
            case "stack-view":
                return scene53;
            case "stepper":
                return scene54;
            case "tab-view":
                return scene55;
            case "teaching-tip":
                return scene56;
            case "text-edit":
                return scene57;
            case "time-picker":
                return scene58;
            case "timeline":
                return scene59;
            case "title-bar":
                return scene60;
            case "toast":
                return scene61;
            case "toggle-button":
                return scene62;
            case "toggle-split-button":
                return scene63;
            case "tree-view":
                return scene64;
            case "window":
                return scene65;
            default:
                return null;
            }
        }
    }
    Component {
        id: scene0
        CatAccordion {
            CatExpander {
                width: parent.width
                headerText: qsTr("Workspace")
                expanded: true
                CatLabel {
                    width: parent.width
                    text: qsTr("Your local files and settings.")
                    wrapMode: Text.Wrap
                }
            }
            CatExpander {
                width: parent.width
                headerText: qsTr("Notifications")
                CatCheckBox {
                    text: qsTr("Enable notifications")
                }
            }
        }
    }
    Component {
        id: scene1
        RowLayout {
            CatAnnotatedScrollBar {
                id: bar
                value: 20
                annotations: [
                    {
                        label: qsTr("Introduction"),
                        position: 0
                    },
                    {
                        label: qsTr("Details"),
                        position: 0.5
                    },
                    {
                        label: qsTr("Conclusion"),
                        position: 1
                    }
                ]
                Layout.preferredWidth: 140
                Layout.preferredHeight: 200
            }
            CatLabel {
                text: qsTr("Position: %1%").arg(Math.round(bar.value))
                Layout.fillWidth: true
                wrapMode: Text.Wrap
            }
        }
    }
    Component {
        id: scene2
        ColumnLayout {
            spacing: 16
            CatAutoSuggestBox {
                id: search
                suggestions: [qsTr("Documents"), qsTr("Images"), qsTr("Audio"), qsTr("Settings")]
                placeholderText: qsTr("Type to find a suggestion…")
                Layout.fillWidth: true
            }
            CatLabel {
                text: qsTr("Use arrow keys and Enter to choose.")
                Layout.fillWidth: true
                wrapMode: Text.Wrap
            }
        }
    }
    Component {
        id: scene3
        Flow {
            spacing: 16
            CatAvatar {
                name: "QCat UiKit"
                presence: "online"
            }
            CatAvatar {
                name: qsTr("Guest")
                presence: "busy"
            }
            CatAvatar {
                name: "QCatUiKit"
                source: "qrc:/qt/qml/QCatUiKit/Gallery/assets/brand/png/qcat-uikit-128.png"
            }
        }
    }
    Component {
        id: scene4
        ColumnLayout {
            id: scene
            property int chosen: -1
            CatBreadcrumb {
                model: [qsTr("Home"), qsTr("Workspace"), qsTr("Documents")]
                Layout.fillWidth: true
                onItemClicked: index => scene.chosen = index
            }
            CatLabel {
                text: qsTr("Selected item: %1").arg(scene.chosen + 1)
            }
        }
    }
    Component {
        id: scene5
        ColumnLayout {
            spacing: 16
            CatCalendarDatePicker {
                Layout.fillWidth: true
            }
            CatLabel {
                text: qsTr("Open the picker and confirm your selection.")
                Layout.fillWidth: true
                wrapMode: Text.Wrap
            }
        }
    }
    Component {
        id: scene6
        ColumnLayout {
            spacing: 12
            CatCarousel {
                id: carousel
                Layout.fillWidth: true
                Layout.preferredHeight: 180
                CatPane {
                    contentItem: CatLabel {
                        text: qsTr("Swipe to explore")
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
                CatPane {
                    contentItem: CatLabel {
                        text: qsTr("Another perspective")
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }
            CatPipsPager {
                numberOfPages: carousel.count
                selectedPageIndex: carousel.currentIndex
                onPageRequested: index => carousel.setCurrentIndex(index)
                Layout.alignment: Qt.AlignHCenter
            }
        }
    }
    Component {
        id: scene7
        ColumnLayout {
            CatButton {
                id: anchor
                text: qsTr("Show guidance")
                onClicked: tip.open()
            }
            CatCoachMark {
                id: tip
                target: anchor
                placement: Qt.BottomEdge
                title: qsTr("A useful detail")
                text: qsTr("This action opens your workspace without leaving the current page.")
                actionText: qsTr("Got it")
                onActionTriggered: tip.close()
            }
        }
    }
    Component {
        id: scene8
        ColumnLayout {
            spacing: 16
            CatColorPicker {
                id: picker
                alphaEnabled: true
                Layout.fillWidth: true
            }
            Rectangle {
                color: picker.selectedColor
                radius: 8
                Layout.fillWidth: true
                Layout.preferredHeight: 80
            }
        }
    }
    Component {
        id: scene9
        ColumnLayout {
            id: scene
            property string status: qsTr("Choose an action")
            B.Action {
                id: openAction
                text: qsTr("Open")
                onTriggered: scene.status = text
            }
            B.Action {
                id: saveAction
                text: qsTr("Save")
                onTriggered: scene.status = text
            }
            B.Action {
                id: exportAction
                text: qsTr("Export")
                onTriggered: scene.status = text
            }
            CatCommandBar {
                primaryActions: [openAction, saveAction]
                secondaryActions: [exportAction]
                Layout.fillWidth: true
            }
            CatLabel {
                text: scene.status
            }
        }
    }
    Component {
        id: scene10
        ColumnLayout {
            id: scene
            property string status: qsTr("Choose an action")
            B.Action {
                id: openAction
                text: qsTr("Open")
                onTriggered: scene.status = text
            }
            B.Action {
                id: saveAction
                text: qsTr("Save")
                onTriggered: scene.status = text
            }
            B.Action {
                id: exportAction
                text: qsTr("Export")
                onTriggered: scene.status = text
            }
            CatButton {
                id: anchor
                text: qsTr("Open commands")
                onClicked: commands.open()
            }
            CatCommandBarFlyout {
                id: commands
                target: anchor
                actions: [openAction, saveAction, exportAction]
            }
            CatLabel {
                text: scene.status
            }
        }
    }
    Component {
        id: scene11
        ColumnLayout {
            spacing: 16
            CatCompoundButton {
                text: qsTr("Open workspace")
                secondaryText: qsTr("Continue with your local files.")
                Layout.fillWidth: true
            }
            CatCompoundButton {
                text: qsTr("Create workspace")
                secondaryText: qsTr("Start a new project.")
                primary: true
                Layout.fillWidth: true
            }
        }
    }
    Component {
        id: scene12
        Item {
            implicitHeight: scene.implicitHeight
            property CatContentDialog dialogObject: CatContentDialog {
                id: dialog
                title: qsTr("Save changes?")
                primaryButtonText: qsTr("Save")
                closeButtonText: qsTr("Cancel")
                width: Math.min(360, parent ? parent.width : 360)
                contentItem: CatLabel {
                    text: qsTr("Your workspace contains changes. Choose whether to save them.")
                    wrapMode: Text.Wrap
                }
                onAccepted: scene.status = qsTr("Saved")
                onRejected: scene.status = qsTr("Cancelled")
            }
            ColumnLayout {
                id: scene
                width: parent.width
                property string status: qsTr("No changes yet")
                CatButton {
                    text: qsTr("Open dialog")
                    onClicked: dialog.open()
                }
                CatLabel {
                    text: scene.status
                }
            }
        }
    }
    Component {
        id: scene13
        ColumnLayout {
            spacing: 16
            CatDatePicker {
                Layout.fillWidth: true
            }
            CatLabel {
                text: qsTr("Open the picker and confirm your selection.")
                Layout.fillWidth: true
                wrapMode: Text.Wrap
            }
        }
    }
    Component {
        id: scene14
        Item {
            implicitHeight: scene.implicitHeight
            property CatDialog dialogObject: CatDialog {
                id: dialog
                title: qsTr("Save changes?")
                standardButtons: CatDialog.Ok | CatDialog.Cancel
                width: Math.min(360, parent ? parent.width : 360)
                contentItem: CatLabel {
                    text: qsTr("Your workspace contains changes. Choose whether to save them.")
                    wrapMode: Text.Wrap
                }
                onAccepted: scene.status = qsTr("Saved")
                onRejected: scene.status = qsTr("Cancelled")
            }
            ColumnLayout {
                id: scene
                width: parent.width
                property string status: qsTr("No changes yet")
                CatButton {
                    text: qsTr("Open dialog")
                    onClicked: dialog.open()
                }
                CatLabel {
                    text: scene.status
                }
            }
        }
    }
    Component {
        id: scene15
        ColumnLayout {
            spacing: 20
            CatLabel {
                text: qsTr("Workspace")
            }
            CatDivider {
                Layout.fillWidth: true
            }
            CatLabel {
                text: qsTr("Keep related sections easy to scan.")
                Layout.fillWidth: true
                wrapMode: Text.Wrap
            }
        }
    }
    Component {
        id: scene16
        ColumnLayout {
            id: scene
            property string status: qsTr("Choose an action")
            CatDropDownButton {
                text: qsTr("Save")
                menu: commands
                onClicked: scene.status = qsTr("Saved")
            }
            CatMenu {
                id: commands
                CatMenuItem {
                    text: qsTr("Save a copy")
                    onTriggered: scene.status = qsTr("Copy saved")
                }
                CatMenuItem {
                    text: qsTr("Export")
                    onTriggered: scene.status = qsTr("Export requested")
                }
            }
            CatLabel {
                text: scene.status
                Layout.fillWidth: true
                wrapMode: Text.Wrap
            }
        }
    }
    Component {
        id: scene17
        CatExpander {
            headerText: qsTr("Workspace details")
            expanded: true
            CatLabel {
                width: parent.width
                text: qsTr("Additional settings stay close without crowding the main view.")
                wrapMode: Text.Wrap
            }
            CatCheckBox {
                text: qsTr("Enable notifications")
            }
        }
    }
    Component {
        id: scene18
        CatField {
            labelText: qsTr("Workspace name")
            required: true
            helperText: qsTr("Use a short, descriptive name.")
            editor: CatTextField {
                placeholderText: qsTr("My workspace")
            }
        }
    }
    Component {
        id: scene19
        ColumnLayout {
            id: scene
            property string status: qsTr(
                                        "Accepts local files. Browsing is handled by the application.")
            CatFileDropZone {
                title: qsTr("Drop files here")
                description: qsTr("Choose documents to add to the workspace.")
                acceptedExtensions: [".txt", ".md", ".pdf"]
                Layout.fillWidth: true
                onFilesDropped: urls => scene.status = qsTr("%1 files selected").arg(urls.length)
                onBrowseRequested: chooser.open()
            }
            CatLabel {
                text: scene.status
                Layout.fillWidth: true
                wrapMode: Text.Wrap
            }
            Dialogs.FileDialog {
                id: chooser
                fileMode: Dialogs.FileDialog.OpenFiles
                nameFilters: [qsTr("Documents (*.txt *.md *.pdf)")]
                onAccepted: scene.status = qsTr("%1 files selected").arg(selectedFiles.length)
            }
        }
    }
    Component {
        id: scene20
        CatFileListView {
            model: [
                {
                    name: qsTr("Project notes"),
                    description: qsTr("Text document · Ready"),
                    url: "file:///notes.txt"
                },
                {
                    name: qsTr("Preview image"),
                    description: qsTr("Image · Uploading"),
                    url: "file:///preview.png"
                }
            ]
        }
    }
    Component {
        id: scene21
        ColumnLayout {
            CatFlowView {
                id: collection
                model: [qsTr("Documents"), qsTr("Images"), qsTr("Audio"), qsTr("Settings")]
                Layout.fillWidth: true
                Layout.preferredHeight: 200
            }
            CatLabel {
                text: qsTr("Selected item: %1").arg(collection.currentIndex + 1)
            }
        }
    }
    Component {
        id: scene22
        ColumnLayout {
            CatButton {
                id: anchor
                text: qsTr("Show guidance")
                onClicked: tip.open()
            }
            CatFlyout {
                id: tip
                target: anchor
                placement: Qt.BottomEdge
                contentItem: ColumnLayout {
                    CatLabel {
                        text: qsTr("More details for the current action.")
                        Layout.fillWidth: true
                        wrapMode: Text.Wrap
                    }
                    CatButton {
                        text: qsTr("Close")
                        onClicked: tip.close()
                    }
                }
            }
        }
    }
    Component {
        id: scene23
        Flow {
            spacing: 24
            CatFontIcon {
                glyph: "◇"
                iconSize: 32
            }
            CatFontIcon {
                glyph: "☆"
                iconSize: 32
            }
            CatFontIcon {
                glyph: "→"
                iconSize: 32
            }
        }
    }
    Component {
        id: scene24
        ColumnLayout {
            CatGridView {
                id: collection
                model: [qsTr("Documents"), qsTr("Images"), qsTr("Audio"), qsTr("Settings")]
                Layout.fillWidth: true
                Layout.preferredHeight: 200
            }
            CatLabel {
                text: qsTr("Selected item: %1").arg(collection.currentIndex + 1)
            }
        }
    }
    Component {
        id: scene25
        ColumnLayout {
            id: scene
            property int activations: 0
            CatHyperlinkButton {
                text: qsTr("Run action")
                showUnderline: true
                onClicked: scene.activations++
            }
            CatLabel {
                text: qsTr("Activated %1 times").arg(scene.activations)
            }
        }
    }
    Component {
        id: scene26
        Flow {
            spacing: 16
            CatInfoBadge {
                value: 8
            }
            CatInfoBadge {
                value: 108
            }
            CatInfoBadge {
                severity: 1
                iconGlyph: "✓"
            }
            CatInfoBadge {
                severity: 3
            }
        }
    }
    Component {
        id: scene27
        ColumnLayout {
            CatListView {
                id: collection
                model: [qsTr("Documents"), qsTr("Images"), qsTr("Audio"), qsTr("Settings")]
                Layout.fillWidth: true
                Layout.preferredHeight: 200
            }
            CatLabel {
                text: qsTr("Selected item: %1").arg(collection.currentIndex + 1)
            }
        }
    }
    Component {
        id: scene28
        ColumnLayout {
            id: scene
            property string status: qsTr("Choose an action")
            CatDropDownButton {
                text: qsTr("Open menu")
                menu: actionsMenu
            }
            CatMenu {
                id: actionsMenu
                CatMenuItem {
                    text: qsTr("Open workspace")
                    onTriggered: scene.status = text
                }
                CatMenuItem {
                    text: qsTr("Show details")
                    checkable: true
                }
                CatMenuSeparator {}
                CatMenuItem {
                    text: qsTr("Disabled")
                    enabled: false
                }
                CatMenu {
                    title: qsTr("More options")
                    CatMenuItem {
                        text: qsTr("Export")
                        onTriggered: scene.status = text
                    }
                }
            }
            CatLabel {
                text: scene.status
            }
        }
    }
    Component {
        id: scene29
        ColumnLayout {
            CatMenuBar {
                Layout.fillWidth: true
                delegate: CatMenuBarItem {}
                CatMenu {
                    title: qsTr("File")
                    CatMenuItem {
                        text: qsTr("Open workspace")
                    }
                    CatMenuItem {
                        text: qsTr("Save")
                    }
                }
                CatMenu {
                    title: qsTr("Edit")
                    CatMenuItem {
                        text: qsTr("Copy")
                    }
                    CatMenuItem {
                        text: qsTr("Paste")
                    }
                }
            }
            CatLabel {
                text: qsTr("Use Alt to reach the menu, then navigate with arrow keys.")
                Layout.fillWidth: true
                wrapMode: Text.Wrap
            }
        }
    }
    Component {
        id: scene30
        ColumnLayout {
            CatMenuBar {
                Layout.fillWidth: true
                delegate: CatMenuBarItem {}
                CatMenu {
                    title: qsTr("File")
                    CatMenuItem {
                        text: qsTr("Open workspace")
                    }
                    CatMenuItem {
                        text: qsTr("Save")
                    }
                }
                CatMenu {
                    title: qsTr("Edit")
                    CatMenuItem {
                        text: qsTr("Copy")
                    }
                    CatMenuItem {
                        text: qsTr("Paste")
                    }
                }
            }
            CatLabel {
                text: qsTr("Use Alt to reach the menu, then navigate with arrow keys.")
                Layout.fillWidth: true
                wrapMode: Text.Wrap
            }
        }
    }
    Component {
        id: scene31
        ColumnLayout {
            id: scene
            property string status: qsTr("Choose an action")
            CatDropDownButton {
                text: qsTr("Open menu")
                menu: actionsMenu
            }
            CatMenu {
                id: actionsMenu
                CatMenuItem {
                    text: qsTr("Open workspace")
                    onTriggered: scene.status = text
                }
                CatMenuItem {
                    text: qsTr("Show details")
                    checkable: true
                }
                CatMenuSeparator {}
                CatMenuItem {
                    text: qsTr("Disabled")
                    enabled: false
                }
                CatMenu {
                    title: qsTr("More options")
                    CatMenuItem {
                        text: qsTr("Export")
                        onTriggered: scene.status = text
                    }
                }
            }
            CatLabel {
                text: scene.status
            }
        }
    }
    Component {
        id: scene32
        ColumnLayout {
            id: scene
            property string status: qsTr("Choose an action")
            CatDropDownButton {
                text: qsTr("Open menu")
                menu: actionsMenu
            }
            CatMenu {
                id: actionsMenu
                CatMenuItem {
                    text: qsTr("Open workspace")
                    onTriggered: scene.status = text
                }
                CatMenuItem {
                    text: qsTr("Show details")
                    checkable: true
                }
                CatMenuSeparator {}
                CatMenuItem {
                    text: qsTr("Disabled")
                    enabled: false
                }
                CatMenu {
                    title: qsTr("More options")
                    CatMenuItem {
                        text: qsTr("Export")
                        onTriggered: scene.status = text
                    }
                }
            }
            CatLabel {
                text: scene.status
            }
        }
    }
    Component {
        id: scene33
        ColumnLayout {
            spacing: 16
            CatMultiSelectComboBox {
                id: selection
                model: [qsTr("Documents"), qsTr("Images"), qsTr("Audio")]
                searchEnabled: true
                selectedIndexes: [0]
                Layout.fillWidth: true
            }
            CatLabel {
                text: qsTr("%1 selected").arg(selection.selectedCount)
            }
        }
    }
    Component {
        id: scene34
        CatNavigationView {
            id: navigation
            model: [qsTr("Overview"), qsTr("Documents"), qsTr("Settings")]
            selectedIndex: 0
            implicitHeight: 240
            page: Component {
                CatPane {
                    contentItem: CatLabel {
                        text: qsTr("Current section: %1").arg(navigation.selectedIndex + 1)
                        wrapMode: Text.Wrap
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }
        }
    }
    Component {
        id: scene35
        ColumnLayout {
            spacing: 16
            CatNumberBox {
                id: number
                value: 12.5
                minimum: 0
                maximum: 100
                smallChange: 0.5
                acceptsExpression: true
                Layout.fillWidth: true
                Accessible.name: qsTr("Quantity")
            }
            CatLabel {
                text: qsTr("Enter a number or an expression such as (2 + 3) * 4. Press Enter or leave the field to apply. Value: %1").arg(number.value)
                Layout.fillWidth: true
                wrapMode: Text.Wrap
            }
        }
    }
    Component {
        id: scene36
        ColumnLayout {
            CatParticleBackdrop {
                id: particles
                Layout.fillWidth: true
                Layout.preferredHeight: 180
                particleCount: 30
            }
            CatSwitch {
                text: qsTr("Animate particles")
                checked: particles.animationEnabled
                onToggled: particles.animationEnabled = checked
            }
        }
    }
    Component {
        id: scene37
        ColumnLayout {
            spacing: 16
            CatPasswordField {
                text: "QCatUiKit"
                placeholderText: qsTr("Password")
                Layout.fillWidth: true
                Accessible.name: qsTr("Password")
            }
            CatLabel {
                text: qsTr("Use the reveal button to check your password.")
                Layout.fillWidth: true
                wrapMode: Text.Wrap
            }
        }
    }
    Component {
        id: scene38
        ColumnLayout {
            CatPipsPager {
                id: pages
                numberOfPages: 12
                selectedPageIndex: 3
                Layout.fillWidth: true
            }
            CatLabel {
                text: qsTr("Page %1 of %2").arg(pages.selectedPageIndex + 1).arg(
                          pages.numberOfPages)
            }
        }
    }
    Component {
        id: scene39
        ColumnLayout {
            CatPivot {
                id: selector
                model: [qsTr("Overview"), qsTr("Activity"), qsTr("Settings")]
                Layout.fillWidth: true
            }
            CatLabel {
                text: qsTr("Selected item: %1").arg(selector.selectedIndex + 1)
            }
        }
    }
    Component {
        id: scene40
        RowLayout {
            spacing: 20
            CatProgressRing {}
            CatProgressRing {
                indeterminate: false
                value: 64
                Layout.preferredWidth: 64
                Layout.preferredHeight: 64
            }
            CatLabel {
                text: qsTr("Active and determinate progress.")
                Layout.fillWidth: true
                wrapMode: Text.Wrap
            }
        }
    }
    Component {
        id: scene41
        ColumnLayout {
            spacing: 16
            CatRatingControl {
                value: 3
                caption: qsTr("Your rating")
            }
            CatRatingControl {
                value: 4
                isReadOnly: true
                caption: qsTr("Read only")
            }
        }
    }
    Component {
        id: scene42
        ColumnLayout {
            id: scene
            property int activations: 0
            CatRepeatButton {
                text: qsTr("Run action")
                delay: 400
                interval: 100
                onClicked: scene.activations++
            }
            CatLabel {
                text: qsTr("Activated %1 times").arg(scene.activations)
            }
        }
    }
    Component {
        id: scene43
        ColumnLayout {
            CatScrollBar {
                id: bar
                orientation: Qt.Horizontal
                size: 0.25
                position: 0.2
                policy: CatScrollBar.AlwaysOn
                Layout.fillWidth: true
            }
            CatLabel {
                text: qsTr("Position: %1%").arg(Math.round(bar.position * 100))
            }
        }
    }
    Component {
        id: scene44
        CatScrollView {
            implicitHeight: 220
            contentWidth: availableWidth
            Column {
                width: parent.width
                spacing: 8
                Repeater {
                    model: 12
                    CatPane {
                        id: rowItem
                        required property int index
                        width: parent.width
                        contentItem: CatLabel {
                            text: qsTr("Item %1").arg(rowItem.index + 1)
                        }
                    }
                }
            }
        }
    }
    Component {
        id: scene45
        ColumnLayout {
            CatSelectorBar {
                id: selector
                model: [qsTr("Overview"), qsTr("Activity"), qsTr("Settings")]
                Layout.fillWidth: true
            }
            CatLabel {
                text: qsTr("Selected item: %1").arg(selector.selectedIndex + 1)
            }
        }
    }
    Component {
        id: scene46
        ColumnLayout {
            spacing: 16
            CatShimmer {
                Layout.fillWidth: true
                Layout.preferredHeight: 24
            }
            CatShimmer {
                Layout.fillWidth: true
                Layout.preferredHeight: 60
            }
            CatLabel {
                text: qsTr("Reduced motion keeps the skeleton still.")
                Layout.fillWidth: true
                wrapMode: Text.Wrap
            }
        }
    }
    Component {
        id: scene47
        ColumnLayout {
            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 180
                CatSpatialItem {
                    id: spatial
                    anchors.centerIn: parent
                    rotation3d: Qt.vector3d(-8, 12, 0)
                    CatPane {
                        anchors.fill: parent
                        contentItem: CatButton {
                            text: qsTr("A live surface")
                            checkable: true
                        }
                    }
                }
            }
            CatLabel {
                text: qsTr("Software rendering and reduced motion use a flat view.")
                Layout.fillWidth: true
                wrapMode: Text.Wrap
            }
        }
    }
    Component {
        id: scene48
        ColumnLayout {
            CatSpatialView {
                id: spatial
                Layout.fillWidth: true
                Layout.preferredHeight: 180
                CatSpatialItem {
                    anchors.centerIn: parent
                    rotation3d: Qt.vector3d(-8, 12, 0)
                    CatPane {
                        anchors.fill: parent
                        contentItem: CatButton {
                            text: qsTr("A live surface")
                            checkable: true
                        }
                    }
                }
            }
            CatLabel {
                text: qsTr("Software rendering and reduced motion use a flat view.")
                Layout.fillWidth: true
                wrapMode: Text.Wrap
            }
        }
    }
    Component {
        id: scene49
        ColumnLayout {
            spacing: 16
            CatSpinBox {
                from: 1
                to: 100
                value: 12
                editable: true
                Layout.fillWidth: true
                Accessible.name: qsTr("Quantity")
            }
            CatSpinBox {
                from: 1
                to: 100
                value: 12
                enabled: false
                Layout.fillWidth: true
            }
        }
    }
    Component {
        id: scene50
        ColumnLayout {
            CatButton {
                text: qsTr("Preview startup")
                onClicked: {
                    splash.open();
                    completion.restart();
                }
            }
            CatSplashScreen {
                id: splash
                title: "QCatUiKit"
                subtitle: qsTr("Preparing your workspace…")
                iconSource: "qrc:/qt/qml/QCatUiKit/Gallery/assets/brand/png/qcat-uikit-128.png"
            }
            Timer {
                id: completion
                interval: 2000
                onTriggered: splash.close()
            }
            CatLabel {
                text: qsTr("This preview closes after two seconds.")
                Layout.fillWidth: true
                wrapMode: Text.Wrap
            }
        }
    }
    Component {
        id: scene51
        ColumnLayout {
            id: scene
            property string status: qsTr("Choose an action")
            CatSplitButton {
                text: qsTr("Save")
                menu: commands
                onClicked: scene.status = qsTr("Saved")
            }
            CatMenu {
                id: commands
                CatMenuItem {
                    text: qsTr("Save a copy")
                    onTriggered: scene.status = qsTr("Copy saved")
                }
                CatMenuItem {
                    text: qsTr("Export")
                    onTriggered: scene.status = qsTr("Export requested")
                }
            }
            CatLabel {
                text: scene.status
                Layout.fillWidth: true
                wrapMode: Text.Wrap
            }
        }
    }
    Component {
        id: scene52
        CatSplitView {
            CatPane {
                B.SplitView.preferredWidth: 160
                B.SplitView.minimumWidth: 80
                contentItem: CatLabel {
                    text: qsTr("Navigation")
                    wrapMode: Text.Wrap
                }
            }
            CatPane {
                B.SplitView.fillWidth: true
                B.SplitView.minimumWidth: 80
                contentItem: CatLabel {
                    text: qsTr("Drag the divider to resize.")
                    wrapMode: Text.Wrap
                }
            }
        }
    }
    Component {
        id: scene53
        ColumnLayout {
            Component {
                id: firstPage
                CatPane {
                    contentItem: CatLabel {
                        text: qsTr("First page")
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }
            Component {
                id: nextPage
                CatPane {
                    contentItem: CatLabel {
                        text: qsTr("Another page")
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }
            CatStackView {
                id: stack
                initialItem: firstPage
                Layout.fillWidth: true
                Layout.preferredHeight: 170
            }
            Flow {
                Layout.fillWidth: true
                spacing: 8
                CatButton {
                    text: qsTr("Push page")
                    onClicked: stack.push(nextPage)
                }
                CatButton {
                    text: qsTr("Go back")
                    enabled: stack.depth > 1
                    onClicked: stack.pop()
                }
            }
        }
    }
    Component {
        id: scene54
        ColumnLayout {
            CatStepper {
                id: steps
                model: [
                    {
                        text: qsTr("Account"),
                        state: "completed"
                    },
                    {
                        text: qsTr("Details")
                    },
                    {
                        text: qsTr("Finish")
                    }
                ]
                currentIndex: 1
                Layout.fillWidth: true
                onStepRequested: index => currentIndex = index
            }
            CatLabel {
                text: qsTr("The application decides whether a step change is allowed.")
                Layout.fillWidth: true
                wrapMode: Text.Wrap
            }
        }
    }
    Component {
        id: scene55
        CatTabView {
            titles: [qsTr("Documents"), qsTr("Settings")]
            tabsClosable: true
            addTabButtonVisible: true
            onCloseRequested: index => status.text = qsTr("Close requested for tab %1").arg(index
                                                                                            + 1)
            onAddRequested: status.text = qsTr("New tab requested")
            CatPane {
                contentItem: CatTextField {
                    objectName: "demoTabDraft"
                    Component.onCompleted: text = qsTr("Keep this draft when switching tabs.")
                }
            }
            CatPane {
                contentItem: CatLabel {
                    id: status
                    text: qsTr("The application owns tab creation and closing.")
                    wrapMode: Text.Wrap
                }
            }
        }
    }
    Component {
        id: scene56
        ColumnLayout {
            CatButton {
                id: anchor
                text: qsTr("Show guidance")
                onClicked: tip.open()
            }
            CatTeachingTip {
                id: tip
                target: anchor
                placement: Qt.BottomEdge
                title: qsTr("A useful detail")
                text: qsTr("This action opens your workspace without leaving the current page.")
                actionText: qsTr("Got it")
                onActionTriggered: tip.close()
            }
        }
    }
    Component {
        id: scene57
        ColumnLayout {
            spacing: 16
            CatTextArea {
                id: editor
                objectName: "demoTextArea"
                placeholderText: qsTr("Write a longer message…")
                Component.onCompleted: text = qsTr(
                                           "A workspace for your ideas.\nKeep notes, drafts and everyday details together.")
                Layout.fillWidth: true
                Layout.preferredHeight: 150
            }
            CatLabel {
                text: qsTr("%1 characters").arg(editor.text.length)
            }
        }
    }
    Component {
        id: scene58
        ColumnLayout {
            spacing: 16
            CatTimePicker {
                Layout.fillWidth: true
            }
            CatLabel {
                text: qsTr("Open the picker and confirm your selection.")
                Layout.fillWidth: true
                wrapMode: Text.Wrap
            }
        }
    }
    Component {
        id: scene59
        CatTimeline {
            model: [
                {
                    title: qsTr("Workspace created"),
                    description: qsTr("Your project is ready."),
                    timestamp: "09:00"
                },
                {
                    title: qsTr("Files added"),
                    description: qsTr("Documents are available to the team."),
                    timestamp: "09:15"
                },
                {
                    title: qsTr("Review complete"),
                    description: qsTr("Everything is up to date."),
                    timestamp: "10:00"
                }
            ]
        }
    }
    Component {
        id: scene60
        Item {
            implicitHeight: sceneLayout.implicitHeight
            property CatWindow sampleObject: CatWindow {
                id: sample
                width: 480
                height: 320
                title: qsTr("Sample workspace")
                frameless: true
                CatPane {
                    anchors.centerIn: parent
                    width: parent.width - 48
                    contentItem: CatLabel {
                        text: qsTr("Your window, your content.")
                        wrapMode: Text.Wrap
                    }
                }
            }
            ColumnLayout {
                id: sceneLayout
                width: parent.width
                CatTitleBar {
                    targetWindow: sample
                    Layout.fillWidth: true
                }
                CatButton {
                    text: qsTr("Open sample window")
                    onClicked: sample.show()
                }
            }
        }
    }
    Component {
        id: scene61
        Item {
            implicitHeight: sceneLayout.implicitHeight
            property CatToast toastObject: CatToast {
                id: toast
                title: qsTr("Workspace")
                message: qsTr("Your changes have been saved.")
                severity: 1
                actionText: qsTr("Dismiss")
                onActionTriggered: close()
                anchors.centerIn: parent
                width: Math.min(340, parent ? parent.width : 340)
            }
            ColumnLayout {
                id: sceneLayout
                width: parent.width
                CatButton {
                    text: qsTr("Show notification")
                    onClicked: toast.open()
                }

                CatLabel {
                    text: qsTr("The notification closes automatically and pauses while hovered.")
                    Layout.fillWidth: true
                    wrapMode: Text.Wrap
                }
            }
        }
    }
    Component {
        id: scene62
        ColumnLayout {
            id: scene
            property int activations: 0
            CatToggleButton {
                text: qsTr("Run action")
                checkable: true
                onClicked: scene.activations++
            }
            CatLabel {
                text: qsTr("Activated %1 times").arg(scene.activations)
            }
        }
    }
    Component {
        id: scene63
        ColumnLayout {
            id: scene
            property string status: qsTr("Choose an action")
            CatToggleSplitButton {
                text: qsTr("Save")
                menu: commands
                onClicked: scene.status = qsTr("Saved")
            }
            CatMenu {
                id: commands
                CatMenuItem {
                    text: qsTr("Save a copy")
                    onTriggered: scene.status = qsTr("Copy saved")
                }
                CatMenuItem {
                    text: qsTr("Export")
                    onTriggered: scene.status = qsTr("Export requested")
                }
            }
            CatLabel {
                text: scene.status
                Layout.fillWidth: true
                wrapMode: Text.Wrap
            }
        }
    }
    Component {
        id: scene64
        ColumnLayout {
            TreeModel {
                id: hierarchy
                items: [
                    {
                        text: qsTr("Workspace"),
                        children: [
                            {
                                text: qsTr("Documents")
                            },
                            {
                                text: qsTr("Images")
                            }
                        ]
                    },
                    {
                        text: qsTr("Settings"),
                        children: [
                            {
                                text: qsTr("Appearance")
                            },
                            {
                                text: qsTr("Language")
                            }
                        ]
                    }
                ]
            }
            CatTreeView {
                id: tree
                model: hierarchy
                Layout.fillWidth: true
                Layout.preferredHeight: 220
                Component.onCompleted: expandRecursively()
            }
        }
    }
    Component {
        id: scene65
        Item {
            implicitHeight: sceneLayout.implicitHeight
            property CatWindow sampleObject: CatWindow {
                id: sample
                width: 480
                height: 320
                title: qsTr("Sample workspace")
                frameless: true
                CatPane {
                    anchors.centerIn: parent
                    width: parent.width - 48
                    contentItem: CatLabel {
                        text: qsTr("Your window, your content.")
                        wrapMode: Text.Wrap
                    }
                }
            }
            ColumnLayout {
                id: sceneLayout
                width: parent.width
                CatLabel {
                    text: qsTr("A themed application window with optional custom title bar.")
                    Layout.fillWidth: true
                    wrapMode: Text.Wrap
                }
                CatButton {
                    text: qsTr("Open sample window")
                    onClicked: sample.show()
                }
            }
        }
    }
}
