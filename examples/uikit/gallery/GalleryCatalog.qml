// SPDX-License-Identifier: Apache-2.0
pragma Singleton
pragma ComponentBehavior: Bound
import QtQuick

QtObject {
    readonly property var categories: [
        {
            id: "charts",
            title: qsTr("Charts"),
            icon: "data"
        },
        {
            id: "input",
            title: qsTr("Input & surfaces"),
            icon: "input"
        },
        {
            id: "feedback",
            title: qsTr("Status & feedback"),
            icon: "feedback"
        },
        {
            id: "navigation",
            title: qsTr("Navigation & overlays"),
            icon: "navigation"
        },
        {
            id: "data",
            title: qsTr("Data & conversation"),
            icon: "data"
        },
        {
            id: "motion",
            title: qsTr("Motion & text"),
            icon: "motion"
        },
        {
            id: "media",
            title: qsTr("Images & effects"),
            icon: "media"
        },
        {
            id: "tools",
            title: qsTr("Interaction tools"),
            icon: "tools"
        },
        {
            id: "collections",
            title: qsTr("Collections"),
            icon: "data"
        },
        {
            id: "dates",
            title: qsTr("Date & time"),
            icon: "navigation"
        },
        {
            id: "dialogs",
            title: qsTr("Dialogs & guidance"),
            icon: "navigation"
        },
        {
            id: "layout",
            title: qsTr("Layout"),
            icon: "tools"
        },
        {
            id: "menus",
            title: qsTr("Menus & commands"),
            icon: "navigation"
        },
        {
            id: "scrolling",
            title: qsTr("Scrolling"),
            icon: "tools"
        },
        {
            id: "spatial",
            title: qsTr("Spatial"),
            icon: "media"
        },
        {
            id: "windowing",
            title: qsTr("Windows"),
            icon: "tools"
        }
    ]
    readonly property var components: GalleryChartCatalog.entries.concat([
        {
            id: "attitude-indicator",
            name: "CatAttitudeIndicator",
            category: "spatial",
            title: qsTr("Aircraft attitude"),
            description: qsTr("Display aircraft pitch and roll with a fixed aircraft marker, pitch ladder and validity state."),
            icon: "media",
            code: "CatAttitudeIndicator {\n    width: 280; height: 280\n    pitchDegrees: 12; rollDegrees: 25\n    attitudeValid: true\n    // Bind these properties to AIRCRAFT telemetry in your application.\n}"
        },
        {
            id: "tilt-control",
            name: "CatTiltControlPanel",
            category: "spatial",
            title: qsTr("Tilt control"),
            description: qsTr("Device-independent calibration, filtering and control-input preview. Sensors and flight protocols belong to the application."),
            icon: "media",
            code: "import QtQuick.Layouts\n\nColumnLayout {\n    id: demo\n    TiltController { id: tilt }\n    // Your application submits device-to-reference quaternions and timestamps.\n    CatTiltControlPanel {\n        Layout.fillWidth: true\n        controlEnabled: tilt.enabled; controlAllowed: tilt.controlAllowed\n        controllerState: tilt.state; sampleFresh: tilt.sampleFresh\n        rollInput: tilt.rollInput; pitchInput: tilt.pitchInput\n        fullScaleAngleDegrees: tilt.fullScaleAngleDegrees\n        deadzoneDegrees: tilt.deadzoneDegrees\n        filterTimeConstantMs: tilt.filterTimeConstantMs; expo: tilt.expo\n        errorText: tilt.error\n        onEnabledRequested: value => tilt.enabled = value\n        onCalibrateRequested: tilt.calibrate()\n        onResetCalibrationRequested: tilt.resetCalibration()\n        onEngageRequested: tilt.engage()\n        onDisengageRequested: tilt.disengage()\n        onFullScaleAngleEdited: degrees => tilt.fullScaleAngleDegrees = degrees\n        onDeadzoneEdited: degrees => tilt.deadzoneDegrees = degrees\n        onFilterTimeConstantEdited: milliseconds => tilt.filterTimeConstantMs = milliseconds\n        onExpoEdited: value => tilt.expo = value\n    }\n    // The app grants controlAllowed and manages transport, freshness and release.\n}"
        },
        {
            id: "led-preview",
            name: "LedPreview",
            category: "spatial",
            title: qsTr("LED preview"),
            description: qsTr("Shared LED layouts, independent Decks, Storage presets, multichannel mixing and memory output."),
            icon: "media",
            code: "LedPreview {\n    width: 640; height: 360\n    source: LedSource { id: leds }\n    pointSize: 6\n    previewBrightness: 1\n    showStatistics: true\n}\n// Submit XYZ layouts and packed RGBA8 frames from C++ (see LEDPREVIEW.md)."
        },
        {
            id: "accordion",
            name: "CatAccordion",
            category: "layout",
            title: qsTr("Accordion"),
            description: qsTr("Coordinate expandable sections, with one or several open at a time."),
            icon: "tools",
            code: "import QtQuick.Layouts\n\nCatAccordion {\n    CatExpander { width: parent.width; headerText: qsTr(\"Workspace\"); expanded: true; CatLabel { width: parent.width; text: qsTr(\"Your local files and settings.\"); wrapMode: Text.Wrap } }\n    CatExpander { width: parent.width; headerText: qsTr(\"Notifications\"); CatCheckBox { text: qsTr(\"Enable notifications\") } }\n}"
        },
        {
            id: "annotated-scrollbar",
            name: "CatAnnotatedScrollBar",
            category: "scrolling",
            title: qsTr("Annotated Scroll Bar"),
            description: qsTr("Jump to labeled positions along a scroll range."),
            icon: "tools",
            code: "import QtQuick.Layouts\n\nRowLayout {\n    CatAnnotatedScrollBar { id: bar; value: 20; annotations: [{label: qsTr(\"Introduction\"), position: 0}, {label: qsTr(\"Details\"), position: 0.5}, {label: qsTr(\"Conclusion\"), position: 1}]; Layout.preferredWidth: 140; Layout.preferredHeight: 200 }\n    CatLabel { text: qsTr(\"Position: %1%\").arg(Math.round(bar.value)); Layout.fillWidth: true; wrapMode: Text.Wrap }\n}"
        },
        {
            id: "auto-suggest-box",
            name: "CatAutoSuggestBox",
            category: "input",
            title: qsTr("Auto Suggest Box"),
            description: qsTr(
                             "Filter suggestions while typing and select with arrow keys or Enter."),
            icon: "input",
            code: "import QtQuick.Layouts\n\nColumnLayout {\n    spacing: 16\n    CatAutoSuggestBox { id: search; suggestions: [qsTr(\"Documents\"), qsTr(\"Images\"), qsTr(\"Audio\"), qsTr(\"Settings\")]; placeholderText: qsTr(\"Type to find a suggestion…\"); Layout.fillWidth: true }\n    CatLabel { text: qsTr(\"Use arrow keys and Enter to choose.\"); Layout.fillWidth: true; wrapMode: Text.Wrap }\n}"
        },
        {
            id: "avatar",
            name: "CatAvatar",
            category: "feedback",
            title: qsTr("Avatar"),
            description: qsTr("Show initials or a circular photo with optional presence status."),
            icon: "input",
            code: "import QtQuick.Layouts\n\nFlow {\n    spacing: 16\n    CatAvatar { name: \"QCat UiKit\"; presence: \"online\" }\n    CatAvatar { name: qsTr(\"Guest\"); presence: \"busy\" }\n    CatAvatar { name: \"QCatUiKit\"; source: \"qrc:/qt/qml/QCatUiKit/Gallery/assets/brand/png/qcat-uikit-128.png\" }\n}"
        },
        {
            id: "breadcrumb",
            name: "CatBreadcrumb",
            category: "navigation",
            title: qsTr("Breadcrumb"),
            description: qsTr("Navigate a path and optionally trim it after selection."),
            icon: "input",
            code: "import QtQuick.Layouts\n\nColumnLayout {\n    id: scene\n    property int chosen: -1\n    CatBreadcrumb { model: [qsTr(\"Home\"), qsTr(\"Workspace\"), qsTr(\"Documents\")]; Layout.fillWidth: true; onItemClicked: index => scene.chosen = index }\n    CatLabel { text: qsTr(\"Selected item: %1\").arg(scene.chosen + 1) }\n}"
        },
        {
            id: "calendar-date-picker",
            name: "CatCalendarDatePicker",
            category: "dates",
            title: qsTr("Calendar Date Picker"),
            description: qsTr("Choose a date from a calendar with explicit date bounds."),
            icon: "input",
            code: "import QtQuick.Layouts\n\nColumnLayout {\n    spacing: 16\n    CatCalendarDatePicker { Layout.fillWidth: true }\n    CatLabel { text: qsTr(\"Open the picker and confirm your selection.\"); Layout.fillWidth: true; wrapMode: Text.Wrap }\n}"
        },
        {
            id: "flip-view",
            name: "CatCarousel",
            category: "collections",
            title: qsTr("Flip View"),
            description: qsTr("Swipe between persistent pages and connect a page indicator."),
            icon: "data",
            code: "import QtQuick.Layouts\n\nColumnLayout {\n    spacing: 12\n    CatCarousel { id: carousel; Layout.fillWidth: true; Layout.preferredHeight: 180\n        CatPane { contentItem: CatLabel { text: qsTr(\"Swipe to explore\"); horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter } }\n        CatPane { contentItem: CatLabel { text: qsTr(\"Another perspective\"); horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter } }\n    }\n    CatPipsPager { numberOfPages: carousel.count; selectedPageIndex: carousel.currentIndex; onPageRequested: index => carousel.setCurrentIndex(index); Layout.alignment: Qt.AlignHCenter }\n}"
        },
        {
            id: "coach-mark",
            name: "CatCoachMark",
            category: "dialogs",
            title: qsTr("Coach Mark"),
            description: qsTr("Draw attention to a target with a modal guidance surface."),
            icon: "navigation",
            code: "import QtQuick.Layouts\n\nColumnLayout {\n    CatButton { id: anchor; text: qsTr(\"Show guidance\"); onClicked: tip.open() }\n    CatCoachMark { id: tip; target: anchor; placement: Qt.BottomEdge; title: qsTr(\"A useful detail\"); text: qsTr(\"This action opens your workspace without leaving the current page.\"); actionText: qsTr(\"Got it\"); onActionTriggered: tip.close() }\n}"
        },
        {
            id: "color-picker",
            name: "CatColorPicker",
            category: "input",
            title: qsTr("Color Picker"),
            description: qsTr("Choose a swatch or edit RGB, opacity and hex values."),
            icon: "input",
            code: "import QtQuick.Layouts\n\nColumnLayout {\n    spacing: 16\n    CatColorPicker { id: picker; alphaEnabled: true; Layout.fillWidth: true }\n    Rectangle { color: picker.selectedColor; radius: 8; Layout.fillWidth: true; Layout.preferredHeight: 80 }\n}"
        },
        {
            id: "command-bar",
            name: "CatCommandBar",
            category: "menus",
            title: qsTr("Command Bar"),
            description: qsTr("Compose primary actions and an overflow menu for secondary actions."),
            icon: "navigation",
            code: "import QtQuick.Layouts\nimport QtQuick.Controls.Basic as B\n\nColumnLayout {\n    id: scene\n    property string status: qsTr(\"Choose an action\")\n    B.Action { id: openAction; text: qsTr(\"Open\"); onTriggered: scene.status = text }\n    B.Action { id: saveAction; text: qsTr(\"Save\"); onTriggered: scene.status = text }\n    B.Action { id: exportAction; text: qsTr(\"Export\"); onTriggered: scene.status = text }\n    CatCommandBar { primaryActions: [openAction, saveAction]; secondaryActions: [exportAction]; Layout.fillWidth: true }\n    CatLabel { text: scene.status }\n}"
        },
        {
            id: "command-bar-flyout",
            name: "CatCommandBarFlyout",
            category: "menus",
            title: qsTr("Command Bar Flyout"),
            description: qsTr("Present contextual actions in an anchored flyout."),
            icon: "navigation",
            code: "import QtQuick.Layouts\nimport QtQuick.Controls.Basic as B\n\nColumnLayout {\n    id: scene\n    property string status: qsTr(\"Choose an action\")\n    B.Action { id: openAction; text: qsTr(\"Open\"); onTriggered: scene.status = text }\n    B.Action { id: saveAction; text: qsTr(\"Save\"); onTriggered: scene.status = text }\n    B.Action { id: exportAction; text: qsTr(\"Export\"); onTriggered: scene.status = text }\n    CatButton { id: anchor; text: qsTr(\"Open commands\"); onClicked: commands.open() }\n    CatCommandBarFlyout { id: commands; target: anchor; actions: [openAction, saveAction, exportAction] }\n    CatLabel { text: scene.status }\n}"
        },
        {
            id: "compound-button",
            name: "CatCompoundButton",
            category: "input",
            title: qsTr("Compound Button"),
            description: qsTr("Combine an action title with a supporting description."),
            icon: "input",
            code: "import QtQuick.Layouts\n\nColumnLayout {\n    spacing: 16\n    CatCompoundButton { text: qsTr(\"Open workspace\"); secondaryText: qsTr(\"Continue with your local files.\"); Layout.fillWidth: true }\n    CatCompoundButton { text: qsTr(\"Create workspace\"); secondaryText: qsTr(\"Start a new project.\"); primary: true; Layout.fillWidth: true }\n}"
        },
        {
            id: "content-dialog",
            name: "CatContentDialog",
            category: "dialogs",
            title: qsTr("Content Dialog"),
            description: qsTr(
                             "Present content with application-defined primary, secondary and close actions."),
            icon: "navigation",
            code: "import QtQuick.Layouts\n\nItem {\n    implicitHeight: scene.implicitHeight\n    property CatContentDialog dialogObject: CatContentDialog { id: dialog; title: qsTr(\"Save changes?\"); primaryButtonText: qsTr(\"Save\"); closeButtonText: qsTr(\"Cancel\"); width: Math.min(360, parent ? parent.width : 360)\n        contentItem: CatLabel { text: qsTr(\"Your workspace contains changes. Choose whether to save them.\"); wrapMode: Text.Wrap }\n        onAccepted: scene.status = qsTr(\"Saved\")\n        onRejected: scene.status = qsTr(\"Cancelled\")\n    }\n    ColumnLayout {\n    width: parent.width\n    id: scene\n    property string status: qsTr(\"No changes yet\")\n    CatButton { text: qsTr(\"Open dialog\"); onClicked: dialog.open() }\n    CatLabel { text: scene.status }\n\n\n    }\n}"
        },
        {
            id: "date-picker",
            name: "CatDatePicker",
            category: "dates",
            title: qsTr("Date Picker"),
            description: qsTr("Confirm a bounded date using month, day and year editors."),
            icon: "input",
            code: "import QtQuick.Layouts\n\nColumnLayout {\n    spacing: 16\n    CatDatePicker { Layout.fillWidth: true }\n    CatLabel { text: qsTr(\"Open the picker and confirm your selection.\"); Layout.fillWidth: true; wrapMode: Text.Wrap }\n}"
        },
        {
            id: "dialog",
            name: "CatDialog",
            category: "dialogs",
            title: qsTr("Dialog"),
            description: qsTr(
                             "Use a themed modal dialog with Qt standard accept and cancel buttons."),
            icon: "navigation",
            code: "import QtQuick.Layouts\n\nItem {\n    implicitHeight: scene.implicitHeight\n    property CatDialog dialogObject: CatDialog { id: dialog; title: qsTr(\"Save changes?\"); standardButtons: CatDialog.Ok | CatDialog.Cancel; width: Math.min(360, parent ? parent.width : 360)\n        contentItem: CatLabel { text: qsTr(\"Your workspace contains changes. Choose whether to save them.\"); wrapMode: Text.Wrap }\n        onAccepted: scene.status = qsTr(\"Saved\")\n        onRejected: scene.status = qsTr(\"Cancelled\")\n    }\n    ColumnLayout {\n    width: parent.width\n    id: scene\n    property string status: qsTr(\"No changes yet\")\n    CatButton { text: qsTr(\"Open dialog\"); onClicked: dialog.open() }\n    CatLabel { text: scene.status }\n\n\n    }\n}"
        },
        {
            id: "divider",
            name: "CatDivider",
            category: "layout",
            title: qsTr("Divider"),
            description: qsTr("Separate sections with a horizontal or vertical rule."),
            icon: "tools",
            code: "import QtQuick.Layouts\n\nColumnLayout {\n    spacing: 20\n    CatLabel { text: qsTr(\"Workspace\") }\n    CatDivider { Layout.fillWidth: true }\n    CatLabel { text: qsTr(\"Keep related sections easy to scan.\"); Layout.fillWidth: true; wrapMode: Text.Wrap }\n}"
        },
        {
            id: "dropdown-button",
            name: "CatDropDownButton",
            category: "input",
            title: qsTr("Drop Down Button"),
            description: qsTr("Open a menu from a keyboard-accessible action button."),
            icon: "input",
            code: "import QtQuick.Layouts\n\nColumnLayout {\n    id: scene\n    property string status: qsTr(\"Choose an action\")\n    CatDropDownButton { text: qsTr(\"Save\"); menu: commands; onClicked: scene.status = qsTr(\"Saved\") }\n    CatMenu { id: commands\n        CatMenuItem { text: qsTr(\"Save a copy\"); onTriggered: scene.status = qsTr(\"Copy saved\") }\n        CatMenuItem { text: qsTr(\"Export\"); onTriggered: scene.status = qsTr(\"Export requested\") }\n    }\n    CatLabel { text: scene.status; Layout.fillWidth: true; wrapMode: Text.Wrap }\n}"
        },
        {
            id: "expander",
            name: "CatExpander",
            category: "layout",
            title: qsTr("Expander"),
            description: qsTr("Reveal persistent content below a disclosure header."),
            icon: "tools",
            code: "import QtQuick.Layouts\n\nCatExpander {\n    headerText: qsTr(\"Workspace details\")\n    expanded: true\n    CatLabel { width: parent.width; text: qsTr(\"Additional settings stay close without crowding the main view.\"); wrapMode: Text.Wrap }\n    CatCheckBox { text: qsTr(\"Enable notifications\") }\n}"
        },
        {
            id: "field",
            name: "CatField",
            category: "layout",
            title: qsTr("Field"),
            description: qsTr("Compose a label, editor, helper text and validation message."),
            icon: "tools",
            code: "import QtQuick.Layouts\n\nCatField {\n    labelText: qsTr(\"Workspace name\")\n    required: true\n    helperText: qsTr(\"Use a short, descriptive name.\")\n    editor: CatTextField { placeholderText: qsTr(\"My workspace\") }\n}"
        },
        {
            id: "file-drop-zone",
            name: "CatFileDropZone",
            category: "input",
            title: qsTr("File Drop Zone"),
            description: qsTr(
                             "Accept filtered local file drops and delegate browsing to the application."),
            icon: "input",
            code: "import QtQuick.Layouts\nimport QtQuick.Dialogs as Dialogs\n\nColumnLayout {\n    id: scene\n    property string status: qsTr(\"Accepts local files. Browsing is handled by the application.\")\n    CatFileDropZone { title: qsTr(\"Drop files here\"); description: qsTr(\"Choose documents to add to the workspace.\"); acceptedExtensions: [\".txt\", \".md\", \".pdf\"]; Layout.fillWidth: true; onFilesDropped: urls => scene.status = qsTr(\"%1 files selected\").arg(urls.length); onBrowseRequested: chooser.open() }\n    CatLabel { text: scene.status; Layout.fillWidth: true; wrapMode: Text.Wrap }\n    Dialogs.FileDialog { id: chooser; fileMode: Dialogs.FileDialog.OpenFiles; nameFilters: [qsTr(\"Documents (*.txt *.md *.pdf)\")]; onAccepted: scene.status = qsTr(\"%1 files selected\").arg(selectedFiles.length) }\n}"
        },
        {
            id: "file-list-view",
            name: "CatFileListView",
            category: "collections",
            title: qsTr("File List View"),
            description: qsTr("Show names and descriptions from an application-owned file model."),
            icon: "data",
            code: "import QtQuick.Layouts\n\nCatFileListView {\n    model: [{name: qsTr(\"Project notes\"), description: qsTr(\"Text document · Ready\"), url: \"file:///notes.txt\"}, {name: qsTr(\"Preview image\"), description: qsTr(\"Image · Uploading\"), url: \"file:///preview.png\"}]\n}"
        },
        {
            id: "flow-view",
            name: "CatFlowView",
            category: "collections",
            title: qsTr("Flow View"),
            description: qsTr("Wrap selectable items across the available width."),
            icon: "data",
            code: "import QtQuick.Layouts\n\nColumnLayout {\n    CatFlowView { id: collection; model: [qsTr(\"Documents\"), qsTr(\"Images\"), qsTr(\"Audio\"), qsTr(\"Settings\")]; Layout.fillWidth: true; Layout.preferredHeight: 200 }\n    CatLabel { text: qsTr(\"Selected item: %1\").arg(collection.currentIndex + 1) }\n}"
        },
        {
            id: "flyout",
            name: "CatFlyout",
            category: "dialogs",
            title: qsTr("Flyout"),
            description: qsTr("Place contextual content beside a target and dismiss it easily."),
            icon: "navigation",
            code: "import QtQuick.Layouts\n\nColumnLayout {\n    CatButton { id: anchor; text: qsTr(\"Show guidance\"); onClicked: tip.open() }\n    CatFlyout { id: tip; target: anchor; placement: Qt.BottomEdge; contentItem: ColumnLayout { CatLabel { text: qsTr(\"More details for the current action.\"); Layout.fillWidth: true; wrapMode: Text.Wrap } CatButton { text: qsTr(\"Close\"); onClicked: tip.close() } } }\n}"
        },
        {
            id: "font-icon",
            name: "CatFontIcon",
            category: "tools",
            title: qsTr("Font Icon"),
            description: qsTr("Render a glyph using the selected font, size and theme color."),
            icon: "input",
            code: "import QtQuick.Layouts\n\nFlow {\n    spacing: 24\n    CatFontIcon { glyph: \"◇\"; iconSize: 32 }\n    CatFontIcon { glyph: \"☆\"; iconSize: 32 }\n    CatFontIcon { glyph: \"→\"; iconSize: 32 }\n}"
        },
        {
            id: "grid-view",
            name: "CatGridView",
            category: "collections",
            title: qsTr("Grid View"),
            description: qsTr("Browse and select items in a scrollable grid."),
            icon: "data",
            code: "import QtQuick.Layouts\n\nColumnLayout {\n    CatGridView { id: collection; model: [qsTr(\"Documents\"), qsTr(\"Images\"), qsTr(\"Audio\"), qsTr(\"Settings\")]; Layout.fillWidth: true; Layout.preferredHeight: 200 }\n    CatLabel { text: qsTr(\"Selected item: %1\").arg(collection.currentIndex + 1) }\n}"
        },
        {
            id: "hyperlink-button",
            name: "CatHyperlinkButton",
            category: "input",
            title: qsTr("Hyperlink Button"),
            description: qsTr("Activate a text link and optionally open an assigned URL."),
            icon: "input",
            code: "import QtQuick.Layouts\n\nColumnLayout {\n    id: scene\n    property int activations: 0\n    CatHyperlinkButton { text: qsTr(\"Run action\"); showUnderline: true; onClicked: scene.activations++ }\n    CatLabel { text: qsTr(\"Activated %1 times\").arg(scene.activations) }\n}"
        },
        {
            id: "info-badge",
            name: "CatInfoBadge",
            category: "feedback",
            title: qsTr("Info Badge"),
            description: qsTr("Display a count, status dot or compact symbol."),
            icon: "input",
            code: "import QtQuick.Layouts\n\nFlow {\n    spacing: 16\n    CatInfoBadge { value: 8 }\n    CatInfoBadge { value: 108 }\n    CatInfoBadge { severity: 1; iconGlyph: \"✓\" }\n    CatInfoBadge { severity: 3 }\n}"
        },
        {
            id: "list-view",
            name: "CatListView",
            category: "collections",
            title: qsTr("List View"),
            description: qsTr(
                             "Browse a list with keyboard navigation and optional multiple selection."),
            icon: "data",
            code: "import QtQuick.Layouts\n\nColumnLayout {\n    CatListView { id: collection; model: [qsTr(\"Documents\"), qsTr(\"Images\"), qsTr(\"Audio\"), qsTr(\"Settings\")]; Layout.fillWidth: true; Layout.preferredHeight: 200 }\n    CatLabel { text: qsTr(\"Selected item: %1\").arg(collection.currentIndex + 1) }\n}"
        },
        {
            id: "menu",
            name: "CatMenu",
            category: "menus",
            title: qsTr("Menu"),
            description: qsTr("Organize actions, checked options, separators and nested menus."),
            icon: "navigation",
            code: "import QtQuick.Layouts\n\nColumnLayout {\n    id: scene\n    property string status: qsTr(\"Choose an action\")\n    CatDropDownButton { text: qsTr(\"Open menu\"); menu: actionsMenu }\n    CatMenu { id: actionsMenu\n        CatMenuItem { text: qsTr(\"Open workspace\"); onTriggered: scene.status = text }\n        CatMenuItem { text: qsTr(\"Show details\"); checkable: true }\n        CatMenuSeparator { }\n        CatMenuItem { text: qsTr(\"Disabled\"); enabled: false }\n        CatMenu { title: qsTr(\"More options\"); CatMenuItem { text: qsTr(\"Export\"); onTriggered: scene.status = text } }\n    }\n    CatLabel { text: scene.status }\n}"
        },
        {
            id: "menu-bar",
            name: "CatMenuBar",
            category: "menus",
            title: qsTr("Menu Bar"),
            description: qsTr("Expose application menus through mouse and keyboard navigation."),
            icon: "navigation",
            code: "import QtQuick.Layouts\n\nColumnLayout {\n    CatMenuBar { Layout.fillWidth: true; delegate: CatMenuBarItem { }\n        CatMenu { title: qsTr(\"File\"); CatMenuItem { text: qsTr(\"Open workspace\") } CatMenuItem { text: qsTr(\"Save\") } }\n        CatMenu { title: qsTr(\"Edit\"); CatMenuItem { text: qsTr(\"Copy\") } CatMenuItem { text: qsTr(\"Paste\") } }\n    }\n    CatLabel { text: qsTr(\"Use Alt to reach the menu, then navigate with arrow keys.\"); Layout.fillWidth: true; wrapMode: Text.Wrap }\n}"
        },
        {
            id: "menubaritem",
            name: "CatMenuBarItem",
            category: "menus",
            title: qsTr("Menu Bar Item"),
            description: qsTr("Use a themed entry in an application menu bar."),
            icon: "navigation",
            code: "import QtQuick.Layouts\n\nColumnLayout {\n    CatMenuBar { Layout.fillWidth: true; delegate: CatMenuBarItem { }\n        CatMenu { title: qsTr(\"File\"); CatMenuItem { text: qsTr(\"Open workspace\") } CatMenuItem { text: qsTr(\"Save\") } }\n        CatMenu { title: qsTr(\"Edit\"); CatMenuItem { text: qsTr(\"Copy\") } CatMenuItem { text: qsTr(\"Paste\") } }\n    }\n    CatLabel { text: qsTr(\"Use Alt to reach the menu, then navigate with arrow keys.\"); Layout.fillWidth: true; wrapMode: Text.Wrap }\n}"
        },
        {
            id: "menuitem",
            name: "CatMenuItem",
            category: "menus",
            title: qsTr("Menu Item"),
            description: qsTr("Trigger an action or toggle a checked option from a menu."),
            icon: "navigation",
            code: "import QtQuick.Layouts\n\nColumnLayout {\n    id: scene\n    property string status: qsTr(\"Choose an action\")\n    CatDropDownButton { text: qsTr(\"Open menu\"); menu: actionsMenu }\n    CatMenu { id: actionsMenu\n        CatMenuItem { text: qsTr(\"Open workspace\"); onTriggered: scene.status = text }\n        CatMenuItem { text: qsTr(\"Show details\"); checkable: true }\n        CatMenuSeparator { }\n        CatMenuItem { text: qsTr(\"Disabled\"); enabled: false }\n        CatMenu { title: qsTr(\"More options\"); CatMenuItem { text: qsTr(\"Export\"); onTriggered: scene.status = text } }\n    }\n    CatLabel { text: scene.status }\n}"
        },
        {
            id: "menuseparator",
            name: "CatMenuSeparator",
            category: "menus",
            title: qsTr("Menu Separator"),
            description: qsTr("Group related menu actions with a visual separator."),
            icon: "navigation",
            code: "import QtQuick.Layouts\n\nColumnLayout {\n    id: scene\n    property string status: qsTr(\"Choose an action\")\n    CatDropDownButton { text: qsTr(\"Open menu\"); menu: actionsMenu }\n    CatMenu { id: actionsMenu\n        CatMenuItem { text: qsTr(\"Open workspace\"); onTriggered: scene.status = text }\n        CatMenuItem { text: qsTr(\"Show details\"); checkable: true }\n        CatMenuSeparator { }\n        CatMenuItem { text: qsTr(\"Disabled\"); enabled: false }\n        CatMenu { title: qsTr(\"More options\"); CatMenuItem { text: qsTr(\"Export\"); onTriggered: scene.status = text } }\n    }\n    CatLabel { text: scene.status }\n}"
        },
        {
            id: "multi-select-combobox",
            name: "CatMultiSelectComboBox",
            category: "input",
            title: qsTr("Multi Select Combo Box"),
            description: qsTr("Choose several options and optionally filter them by text."),
            icon: "input",
            code: "import QtQuick.Layouts\n\nColumnLayout {\n    spacing: 16\n    CatMultiSelectComboBox { id: selection; model: [qsTr(\"Documents\"), qsTr(\"Images\"), qsTr(\"Audio\")]; searchEnabled: true; selectedIndexes: [0]; Layout.fillWidth: true }\n    CatLabel { text: qsTr(\"%1 selected\").arg(selection.selectedCount) }\n}"
        },
        {
            id: "navigation-view",
            name: "CatNavigationView",
            category: "navigation",
            title: qsTr("Navigation View"),
            description: qsTr("Host navigation and content, with a drawer on compact layouts."),
            icon: "input",
            code: "import QtQuick.Layouts\n\nCatNavigationView {\n    id: navigation\n    model: [qsTr(\"Overview\"), qsTr(\"Documents\"), qsTr(\"Settings\")]\n    selectedIndex: 0\n    implicitHeight: 240\n    page: Component { CatPane { contentItem: CatLabel { text: qsTr(\"Current section: %1\").arg(navigation.selectedIndex + 1); wrapMode: Text.Wrap; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter } } }\n}"
        },
        {
            id: "number-box",
            name: "CatNumberBox",
            category: "input",
            title: qsTr("Number Box"),
            description: qsTr(
                             "Edit bounded decimal values with steps, locale formatting and arithmetic."),
            icon: "input",
            code: "import QtQuick.Layouts\n\nColumnLayout {\n    spacing: 16\n    CatNumberBox { id: number; value: 12.5; minimum: 0; maximum: 100; smallChange: 0.5; acceptsExpression: true; Layout.fillWidth: true; Accessible.name: qsTr(\"Quantity\") }\n    CatLabel { text: qsTr(\"Enter a number or an expression such as (2 + 3) * 4. Press Enter or leave the field to apply. Value: %1\").arg(number.value); Layout.fillWidth: true; wrapMode: Text.Wrap }\n}"
        },
        {
            id: "particle-backdrop",
            name: "CatParticleBackdrop",
            category: "layout",
            title: qsTr("Particle Backdrop"),
            description: qsTr("Animate lightweight themed particles and respect reduced motion."),
            icon: "tools",
            code: "import QtQuick.Layouts\n\nColumnLayout {\n    CatParticleBackdrop { id: particles; Layout.fillWidth: true; Layout.preferredHeight: 180; particleCount: 30 }\n    CatSwitch { text: qsTr(\"Animate particles\"); checked: particles.animationEnabled; onToggled: particles.animationEnabled = checked }\n}"
        },
        {
            id: "password-box",
            name: "CatPasswordField",
            category: "input",
            title: qsTr("Password Box"),
            description: qsTr("Enter a password and reveal it with a keyboard-accessible toggle."),
            icon: "input",
            code: "import QtQuick.Layouts\n\nColumnLayout {\n    spacing: 16\n    CatPasswordField { text: \"QCatUiKit\"; placeholderText: qsTr(\"Password\"); Layout.fillWidth: true; Accessible.name: qsTr(\"Password\") }\n    CatLabel { text: qsTr(\"Use the reveal button to check your password.\"); Layout.fillWidth: true; wrapMode: Text.Wrap }\n}"
        },
        {
            id: "pips-pager",
            name: "CatPipsPager",
            category: "scrolling",
            title: qsTr("Pips Pager"),
            description: qsTr("Navigate pages with a bounded set of selectable indicators."),
            icon: "tools",
            code: "import QtQuick.Layouts\n\nColumnLayout {\n    CatPipsPager { id: pages; numberOfPages: 12; selectedPageIndex: 3; Layout.fillWidth: true }\n    CatLabel { text: qsTr(\"Page %1 of %2\").arg(pages.selectedPageIndex + 1).arg(pages.numberOfPages) }\n}"
        },
        {
            id: "pivot",
            name: "CatPivot",
            category: "navigation",
            title: qsTr("Pivot"),
            description: qsTr("Switch sections using a text selector with an active underline."),
            icon: "input",
            code: "import QtQuick.Layouts\n\nColumnLayout {\n    CatPivot { id: selector; model: [qsTr(\"Overview\"), qsTr(\"Activity\"), qsTr(\"Settings\")]; Layout.fillWidth: true }\n    CatLabel { text: qsTr(\"Selected item: %1\").arg(selector.selectedIndex + 1) }\n}"
        },
        {
            id: "progress-ring",
            name: "CatProgressRing",
            category: "feedback",
            title: qsTr("Progress Ring"),
            description: qsTr("Switch between an active loading arc and determinate progress."),
            icon: "input",
            code: "import QtQuick.Layouts\n\nRowLayout {\n    spacing: 20\n    CatProgressRing { }\n    CatProgressRing { indeterminate: false; value: 64; Layout.preferredWidth: 64; Layout.preferredHeight: 64 }\n    CatLabel { text: qsTr(\"Active and determinate progress.\"); Layout.fillWidth: true; wrapMode: Text.Wrap }\n}"
        },
        {
            id: "rating-control",
            name: "CatRatingControl",
            category: "input",
            title: qsTr("Rating Control"),
            description: qsTr(
                             "Select or clear a rating, including fractional display and keyboard steps."),
            icon: "input",
            code: "import QtQuick.Layouts\n\nColumnLayout {\n    spacing: 16\n    CatRatingControl { value: 3; caption: qsTr(\"Your rating\") }\n    CatRatingControl { value: 4; isReadOnly: true; caption: qsTr(\"Read only\") }\n}"
        },
        {
            id: "repeat-button",
            name: "CatRepeatButton",
            category: "input",
            title: qsTr("Repeat Button"),
            description: qsTr("Repeat an action while a mouse button or key is held."),
            icon: "input",
            code: "import QtQuick.Layouts\n\nColumnLayout {\n    id: scene\n    property int activations: 0\n    CatRepeatButton { text: qsTr(\"Run action\"); delay: 400; interval: 100; onClicked: scene.activations++ }\n    CatLabel { text: qsTr(\"Activated %1 times\").arg(scene.activations) }\n}"
        },
        {
            id: "scrollbar",
            name: "CatScrollBar",
            category: "scrolling",
            title: qsTr("Scroll Bar"),
            description: qsTr("Drag a themed scroll thumb or attach it to a flickable view."),
            icon: "tools",
            code: "import QtQuick.Layouts\n\nColumnLayout {\n    CatScrollBar { id: bar; orientation: Qt.Horizontal; size: 0.25; position: 0.2; policy: CatScrollBar.AlwaysOn; Layout.fillWidth: true }\n    CatLabel { text: qsTr(\"Position: %1%\").arg(Math.round(bar.position * 100)) }\n}"
        },
        {
            id: "scroll-view",
            name: "CatScrollView",
            category: "scrolling",
            title: qsTr("Scroll View"),
            description: qsTr(
                             "Scroll content by mouse drag, wheel or touch with themed horizontal and vertical bars."),
            icon: "tools",
            code: "import QtQuick.Layouts\n\nCatScrollView {\n    implicitHeight: 220\n    contentWidth: availableWidth\n    Column { width: parent.width; spacing: 8\n        Repeater { model: 12; CatPane { id: rowItem; required property int index; width: parent.width; contentItem: CatLabel { text: qsTr(\"Item %1\").arg(rowItem.index + 1) } } }\n    }\n}"
        },
        {
            id: "selector-bar",
            name: "CatSelectorBar",
            category: "navigation",
            title: qsTr("Selector Bar"),
            description: qsTr("Choose one section from a group of labeled options."),
            icon: "input",
            code: "import QtQuick.Layouts\n\nColumnLayout {\n    CatSelectorBar { id: selector; model: [qsTr(\"Overview\"), qsTr(\"Activity\"), qsTr(\"Settings\")]; Layout.fillWidth: true }\n    CatLabel { text: qsTr(\"Selected item: %1\").arg(selector.selectedIndex + 1) }\n}"
        },
        {
            id: "shimmer",
            name: "CatShimmer",
            category: "feedback",
            title: qsTr("Shimmer"),
            description: qsTr("Show a loading skeleton that stays still under reduced motion."),
            icon: "input",
            code: "import QtQuick.Layouts\n\nColumnLayout {\n    spacing: 16\n    CatShimmer { Layout.fillWidth: true; Layout.preferredHeight: 24 }\n    CatShimmer { Layout.fillWidth: true; Layout.preferredHeight: 60 }\n    CatLabel { text: qsTr(\"Reduced motion keeps the skeleton still.\"); Layout.fillWidth: true; wrapMode: Text.Wrap }\n}"
        },
        {
            id: "spatial-item",
            name: "CatSpatialItem",
            category: "spatial",
            title: qsTr("Spatial Item"),
            description: qsTr("Transform a live surface in depth, with a flat rendering fallback."),
            icon: "tools",
            code: "import QtQuick.Layouts\n\nColumnLayout {\n    Item { Layout.fillWidth: true; Layout.preferredHeight: 180\n        CatSpatialItem { id: spatial; anchors.centerIn: parent; rotation3d: Qt.vector3d(-8, 12, 0); CatPane { anchors.fill: parent; contentItem: CatButton { text: qsTr(\"A live surface\"); checkable: true } } }\n    }\n    CatLabel { text: qsTr(\"Software rendering and reduced motion use a flat view.\"); Layout.fillWidth: true; wrapMode: Text.Wrap }\n}"
        },
        {
            id: "spatial-view",
            name: "CatSpatialView",
            category: "spatial",
            title: qsTr("Spatial View"),
            description: qsTr("Compose interactive surfaces with perspective and pointer tilt."),
            icon: "tools",
            code: "import QtQuick.Layouts\n\nColumnLayout {\n    CatSpatialView { id: spatial; Layout.fillWidth: true; Layout.preferredHeight: 180\n        CatSpatialItem { anchors.centerIn: parent; rotation3d: Qt.vector3d(-8, 12, 0); CatPane { anchors.fill: parent; contentItem: CatButton { text: qsTr(\"A live surface\"); checkable: true } } }\n    }\n    CatLabel { text: qsTr(\"Software rendering and reduced motion use a flat view.\"); Layout.fillWidth: true; wrapMode: Text.Wrap }\n}"
        },
        {
            id: "spinbox",
            name: "CatSpinBox",
            category: "input",
            title: qsTr("Spin Box"),
            description: qsTr("Edit an integer within bounds using text, buttons or keys."),
            icon: "input",
            code: "import QtQuick.Layouts\n\nColumnLayout {\n    spacing: 16\n    CatSpinBox { from: 1; to: 100; value: 12; editable: true; Layout.fillWidth: true; Accessible.name: qsTr(\"Quantity\") }\n    CatSpinBox { from: 1; to: 100; value: 12; enabled: false; Layout.fillWidth: true }\n}"
        },
        {
            id: "splash-screen",
            name: "CatSplashScreen",
            category: "feedback",
            title: qsTr("Splash Screen"),
            description: qsTr("Cover a window with branding, startup text and progress."),
            icon: "input",
            code: "import QtQuick.Layouts\n\nColumnLayout {\n    CatButton { text: qsTr(\"Preview startup\"); onClicked: { splash.open(); completion.restart(); } }\n    CatSplashScreen { id: splash; title: \"QCatUiKit\"; subtitle: qsTr(\"Preparing your workspace…\"); iconSource: \"qrc:/qt/qml/QCatUiKit/Gallery/assets/brand/png/qcat-uikit-128.png\" }\n    Timer { id: completion; interval: 2000; onTriggered: splash.close() }\n    CatLabel { text: qsTr(\"This preview closes after two seconds.\"); Layout.fillWidth: true; wrapMode: Text.Wrap }\n}"
        },
        {
            id: "split-button",
            name: "CatSplitButton",
            category: "input",
            title: qsTr("Split Button"),
            description: qsTr("Keep a primary action separate from its additional menu actions."),
            icon: "input",
            code: "import QtQuick.Layouts\n\nColumnLayout {\n    id: scene\n    property string status: qsTr(\"Choose an action\")\n    CatSplitButton { text: qsTr(\"Save\"); menu: commands; onClicked: scene.status = qsTr(\"Saved\") }\n    CatMenu { id: commands\n        CatMenuItem { text: qsTr(\"Save a copy\"); onTriggered: scene.status = qsTr(\"Copy saved\") }\n        CatMenuItem { text: qsTr(\"Export\"); onTriggered: scene.status = qsTr(\"Export requested\") }\n    }\n    CatLabel { text: scene.status; Layout.fillWidth: true; wrapMode: Text.Wrap }\n}"
        },
        {
            id: "split-view",
            name: "CatSplitView",
            category: "collections",
            title: qsTr("Split View"),
            description: qsTr("Resize adjacent panes while preserving their minimum sizes."),
            icon: "data",
            code: "import QtQuick.Layouts\nimport QtQuick.Controls.Basic as B\n\nCatSplitView {\n    CatPane { B.SplitView.preferredWidth: 160; B.SplitView.minimumWidth: 80; contentItem: CatLabel { text: qsTr(\"Navigation\"); wrapMode: Text.Wrap } }\n    CatPane { B.SplitView.fillWidth: true; B.SplitView.minimumWidth: 80; contentItem: CatLabel { text: qsTr(\"Drag the divider to resize.\"); wrapMode: Text.Wrap } }\n}"
        },
        {
            id: "stack-view",
            name: "CatStackView",
            category: "collections",
            title: qsTr("Stack View"),
            description: qsTr("Push, pop or replace pages with theme-controlled transitions."),
            icon: "data",
            code: "import QtQuick.Layouts\n\nColumnLayout {\n    Component { id: firstPage; CatPane { contentItem: CatLabel { text: qsTr(\"First page\"); horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter } } }\n    Component { id: nextPage; CatPane { contentItem: CatLabel { text: qsTr(\"Another page\"); horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter } } }\n    CatStackView { id: stack; initialItem: firstPage; Layout.fillWidth: true; Layout.preferredHeight: 170 }\n    Flow { Layout.fillWidth: true; spacing: 8\n        CatButton { text: qsTr(\"Push page\"); onClicked: stack.push(nextPage) }\n        CatButton { text: qsTr(\"Go back\"); enabled: stack.depth > 1; onClicked: stack.pop() }\n    }\n}"
        },
        {
            id: "stepper",
            name: "CatStepper",
            category: "navigation",
            title: qsTr("Stepper"),
            description: qsTr("Request a step change while the application owns workflow state."),
            icon: "input",
            code: "import QtQuick.Layouts\n\nColumnLayout {\n    CatStepper { id: steps; model: [{text: qsTr(\"Account\"), state: \"completed\"}, {text: qsTr(\"Details\")}, {text: qsTr(\"Finish\")}]; currentIndex: 1; Layout.fillWidth: true; onStepRequested: index => currentIndex = index }\n    CatLabel { text: qsTr(\"The application decides whether a step change is allowed.\"); Layout.fillWidth: true; wrapMode: Text.Wrap }\n}"
        },
        {
            id: "tab-view",
            name: "CatTabView",
            category: "navigation",
            title: qsTr("Tab View"),
            description: qsTr("Preserve page state and request tab creation or closing."),
            icon: "input",
            code: "import QtQuick.Layouts\n\nCatTabView {\n    titles: [qsTr(\"Documents\"), qsTr(\"Settings\")]\n    tabsClosable: true\n    addTabButtonVisible: true\n    onCloseRequested: index => status.text = qsTr(\"Close requested for tab %1\").arg(index + 1)\n    onAddRequested: status.text = qsTr(\"New tab requested\")\n    CatPane { contentItem: CatTextField { Component.onCompleted: text = qsTr(\"Keep this draft when switching tabs.\") } }\n    CatPane { contentItem: CatLabel { id: status; text: qsTr(\"The application owns tab creation and closing.\"); wrapMode: Text.Wrap } }\n}"
        },
        {
            id: "teaching-tip",
            name: "CatTeachingTip",
            category: "dialogs",
            title: qsTr("Teaching Tip"),
            description: qsTr("Explain a target action in an anchored, dismissible surface."),
            icon: "navigation",
            code: "import QtQuick.Layouts\n\nColumnLayout {\n    CatButton { id: anchor; text: qsTr(\"Show guidance\"); onClicked: tip.open() }\n    CatTeachingTip { id: tip; target: anchor; placement: Qt.BottomEdge; title: qsTr(\"A useful detail\"); text: qsTr(\"This action opens your workspace without leaving the current page.\"); actionText: qsTr(\"Got it\"); onActionTriggered: tip.close() }\n}"
        },
        {
            id: "text-edit",
            name: "CatTextArea",
            category: "input",
            title: qsTr("Text Edit"),
            description: qsTr(
                             "Write and select multiline text with themed focus and validation states."),
            icon: "input",
            code: "import QtQuick.Layouts\n\nColumnLayout {\n    spacing: 16\n    CatTextArea { id: editor; placeholderText: qsTr(\"Write a longer message…\"); Component.onCompleted: text = qsTr(\"A workspace for your ideas.\\nKeep notes, drafts and everyday details together.\"); Layout.fillWidth: true; Layout.preferredHeight: 150 }\n    CatLabel { text: qsTr(\"%1 characters\").arg(editor.text.length) }\n}"
        },
        {
            id: "time-picker",
            name: "CatTimePicker",
            category: "dates",
            title: qsTr("Time Picker"),
            description: qsTr("Confirm an hour and minute with configurable minute steps."),
            icon: "input",
            code: "import QtQuick.Layouts\n\nColumnLayout {\n    spacing: 16\n    CatTimePicker { Layout.fillWidth: true }\n    CatLabel { text: qsTr(\"Open the picker and confirm your selection.\"); Layout.fillWidth: true; wrapMode: Text.Wrap }\n}"
        },
        {
            id: "timeline",
            name: "CatTimeline",
            category: "collections",
            title: qsTr("Timeline"),
            description: qsTr("Present ordered events with descriptions and timestamps."),
            icon: "data",
            code: "import QtQuick.Layouts\n\nCatTimeline {\n    model: [{title: qsTr(\"Workspace created\"), description: qsTr(\"Your project is ready.\"), timestamp: \"09:00\"}, {title: qsTr(\"Files added\"), description: qsTr(\"Documents are available to the team.\"), timestamp: \"09:15\"}, {title: qsTr(\"Review complete\"), description: qsTr(\"Everything is up to date.\"), timestamp: \"10:00\"}]\n}"
        },
        {
            id: "title-bar",
            name: "CatTitleBar",
            category: "windowing",
            title: qsTr("Title Bar"),
            description: qsTr("Move a window and access minimize, maximize and close actions."),
            icon: "navigation",
            code: "import QtQuick.Layouts\n\nItem {\n    implicitHeight: sceneLayout.implicitHeight\n    property CatWindow sampleObject: CatWindow { id: sample; width: 480; height: 320; title: qsTr(\"Sample workspace\"); frameless: true\n        CatPane { anchors.centerIn: parent; width: parent.width - 48; contentItem: CatLabel { text: qsTr(\"Your window, your content.\"); wrapMode: Text.Wrap } }\n    }\n    ColumnLayout {\n    width: parent.width\n    id: sceneLayout\n    CatTitleBar { targetWindow: sample; Layout.fillWidth: true }\n    CatButton { text: qsTr(\"Open sample window\"); onClicked: sample.show() }\n\n\n    }\n}"
        },
        {
            id: "toast",
            name: "CatToast",
            category: "feedback",
            title: qsTr("Toast"),
            description: qsTr("Show a timed notification with an action and optional hover pause."),
            icon: "input",
            code: "import QtQuick.Layouts\n\nItem {\n    implicitHeight: sceneLayout.implicitHeight\n    property CatToast toastObject: CatToast { id: toast; title: qsTr(\"Workspace\"); message: qsTr(\"Your changes have been saved.\"); severity: 1; actionText: qsTr(\"Dismiss\"); onActionTriggered: close(); anchors.centerIn: parent; width: Math.min(340, parent ? parent.width : 340) }\n    ColumnLayout {\n    width: parent.width\n    id: sceneLayout\n    CatButton { text: qsTr(\"Show notification\"); onClicked: toast.open() }\n\n    CatLabel { text: qsTr(\"The notification closes automatically and pauses while hovered.\"); Layout.fillWidth: true; wrapMode: Text.Wrap }\n\n    }\n}"
        },
        {
            id: "toggle-button",
            name: "CatToggleButton",
            category: "input",
            title: qsTr("Toggle Button"),
            description: qsTr("Keep a checked state or cycle through three selection states."),
            icon: "input",
            code: "import QtQuick.Layouts\n\nColumnLayout {\n    id: scene\n    property int activations: 0\n    CatToggleButton { text: qsTr(\"Run action\"); checkable: true; onClicked: scene.activations++ }\n    CatLabel { text: qsTr(\"Activated %1 times\").arg(scene.activations) }\n}"
        },
        {
            id: "toggle-split-button",
            name: "CatToggleSplitButton",
            category: "input",
            title: qsTr("Toggle Split Button"),
            description: qsTr("Toggle a primary action while keeping its menu independent."),
            icon: "input",
            code: "import QtQuick.Layouts\n\nColumnLayout {\n    id: scene\n    property string status: qsTr(\"Choose an action\")\n    CatToggleSplitButton { text: qsTr(\"Save\"); menu: commands; onClicked: scene.status = qsTr(\"Saved\") }\n    CatMenu { id: commands\n        CatMenuItem { text: qsTr(\"Save a copy\"); onTriggered: scene.status = qsTr(\"Copy saved\") }\n        CatMenuItem { text: qsTr(\"Export\"); onTriggered: scene.status = qsTr(\"Export requested\") }\n    }\n    CatLabel { text: scene.status; Layout.fillWidth: true; wrapMode: Text.Wrap }\n}"
        },
        {
            id: "tree-view",
            name: "CatTreeView",
            category: "collections",
            title: qsTr("Tree View"),
            description: qsTr("Expand and select hierarchical rows from a Qt item model."),
            icon: "data",
            code: "import QtQuick.Layouts\n\nColumnLayout {\n    TreeModel { id: hierarchy; items: [{text: qsTr(\"Workspace\"), children: [{text: qsTr(\"Documents\")}, {text: qsTr(\"Images\")}]}, {text: qsTr(\"Settings\"), children: [{text: qsTr(\"Appearance\")}, {text: qsTr(\"Language\")}]}] }\n    CatTreeView { id: tree; model: hierarchy; Layout.fillWidth: true; Layout.preferredHeight: 220; Component.onCompleted: expandRecursively() }\n}"
        },
        {
            id: "window",
            name: "CatWindow",
            category: "windowing",
            title: qsTr("Window"),
            description: qsTr("Host content with themed colors and an optional custom title bar."),
            icon: "navigation",
            code: "import QtQuick.Layouts\n\nItem {\n    implicitHeight: sceneLayout.implicitHeight\n    property CatWindow sampleObject: CatWindow { id: sample; width: 480; height: 320; title: qsTr(\"Sample workspace\"); frameless: true\n        CatPane { anchors.centerIn: parent; width: parent.width - 48; contentItem: CatLabel { text: qsTr(\"Your window, your content.\"); wrapMode: Text.Wrap } }\n    }\n    ColumnLayout {\n    width: parent.width\n    id: sceneLayout\n    CatLabel { text: qsTr(\"A themed application window with optional custom title bar.\"); Layout.fillWidth: true; wrapMode: Text.Wrap }\n    CatButton { text: qsTr(\"Open sample window\"); onClicked: sample.show() }\n\n\n    }\n}"
        },
        {
            id: "checkbox",
            name: "CatCheckBox",
            category: "input",
            title: qsTr("Check box"),
            description: qsTr("Select independent options, including a mixed state."),
            icon: "input",
            code: "CatCheckBox { text: qsTr(\"Enable notifications\"); checked: true }"
        },
        {
            id: "radio",
            name: "CatRadioButton",
            category: "input",
            title: qsTr("Radio button"),
            description: qsTr("Choose one option from an exclusive group."),
            icon: "input",
            code: "Column {\n    CatRadioButton { text: qsTr(\"Local workspace\"); checked: true }\n    CatRadioButton { text: qsTr(\"Cloud workspace\") }\n}"
        },
        {
            id: "switch",
            name: "CatSwitch",
            category: "input",
            title: qsTr("Switch"),
            description: qsTr("Turn settings on and off with mouse, touch or keyboard."),
            icon: "input",
            code: "CatSwitch { text: qsTr(\"Wi-Fi\"); checked: true }"
        },
        {
            id: "slider",
            name: "CatSlider",
            category: "input",
            title: qsTr("Slider"),
            description: qsTr("Continuous, stepped, centered and vertical sliders with ticks and value tooltips."),
            icon: "input",
            code: "CatSlider { from: 0; to: 100; value: 60; stepSize: 5 }"
        },
        {
            id: "rangeslider",
            name: "CatRangeSlider",
            category: "input",
            title: qsTr("Range slider"),
            description: qsTr("Select an interval, constrain its span, or drag the whole selection."),
            icon: "input",
            code: "CatRangeSlider { from: 0; to: 100; first.value: 20; second.value: 70; rangeDragEnabled: true }"
        },
        {
            id: "sliderfield",
            name: "CatSliderField",
            category: "input",
            title: qsTr("Slider field"),
            description: qsTr("Combine a slider with exact numeric input, units and default restoration."),
            icon: "input",
            code: "CatSliderField { width: 360; from: -60; to: 12; value: -12; decimals: 1; suffix: \" dB\"; resetVisible: true; defaultValue: -12 }"
        },
        {
            id: "combobox",
            name: "CatComboBox",
            category: "input",
            title: qsTr("Combo box"),
            description: qsTr("Choose or edit a value in a themed dropdown."),
            icon: "input",
            code: "CatComboBox { model: [qsTr(\"Desktop\"), qsTr(\"Touch\"), qsTr(\"Mobile\")] }"
        },
        {
            id: "button",
            name: "CatButton",
            category: "input",
            title: qsTr("Button"),
            description: qsTr("Actions with primary, default, disabled and loading states."),
            icon: "input",
            code: "CatButton {\n    text: qsTr(\"Continue\")\n    primary: true\n}"
        },
        {
            id: "textfield",
            name: "CatTextField",
            category: "input",
            title: qsTr("Text field"),
            description: qsTr("Keyboard-friendly text entry with focus and placeholder states."),
            icon: "input",
            code: "CatTextField {\n    placeholderText: qsTr(\"Type something…\")\n}"
        },
        {
            id: "ipaddress",
            name: "CatIpAddressField",
            category: "input",
            title: qsTr("IP address"),
            description: qsTr("A focused editor for IPv4 addresses."),
            icon: "input",
            code: "CatIpAddressField { text: \"192.168.1.1\" }"
        },
        {
            id: "label",
            name: "CatLabel",
            category: "input",
            title: qsTr("Typography"),
            description: qsTr("Readable text that follows your theme."),
            icon: "input",
            code: "CatLabel {\n    text: qsTr(\"Make room for good design.\")\n    font.pixelSize: 28\n}"
        },
        {
            id: "pane",
            name: "CatPane",
            category: "input",
            title: qsTr("Surface"),
            description: qsTr("A themed surface for grouping related content."),
            icon: "input",
            code: "CatPane {\n    contentItem: CatLabel { text: qsTr(\"A place for your content.\") }\n}"
        },
        {
            id: "alert",
            name: "CatAlert",
            category: "feedback",
            title: qsTr("Alert"),
            description: qsTr("Inline information, success, warning and error messages."),
            icon: "feedback",
            code: "CatAlert {\n    text: qsTr(\"Your changes have been saved.\")\n    severity: 1\n    closable: true\n}"
        },
        {
            id: "busy",
            name: "CatBusyIndicator",
            category: "feedback",
            title: qsTr("Loading indicator"),
            description: qsTr("A steady loading arc that respects reduced motion."),
            icon: "feedback",
            code: "CatBusyIndicator {\n    width: 32\n    height: 32\n    running: true\n}"
        },
        {
            id: "progress",
            name: "CatProgressBar",
            category: "feedback",
            title: qsTr("Progress bar"),
            description: qsTr("Make the progress of a task visible."),
            icon: "feedback",
            code: "import QtQuick.Layouts\n\nColumnLayout {\n    width: 360\n    CatProgressBar { value: progress.value / 100; Layout.fillWidth: true }\n    CatSlider {\n        id: progress\n        Layout.fillWidth: true\n        from: 0; to: 100; value: 64; stepSize: 1\n        suffix: \"%\"; tickInterval: 25; tickLabelsVisible: true\n        valueToolTipVisible: true; defaultValue: 64; defaultIndicatorVisible: true\n        Accessible.name: qsTr(\"Progress\")\n    }\n    CatButton { text: qsTr(\"Reset\"); onClicked: progress.reset() }\n}"
        },
        {
            id: "circular",
            name: "CatCircularProgress",
            category: "feedback",
            title: qsTr("Circular progress"),
            description: qsTr("A compact circular view of task completion."),
            icon: "feedback",
            code: "import QtQuick.Layouts\n\nColumnLayout {\n    width: 360\n    CatCircularProgress { value: progress.value; Layout.alignment: Qt.AlignHCenter }\n    CatSlider {\n        id: progress\n        Layout.fillWidth: true\n        from: 0; to: 100; value: 64; stepSize: 1\n        suffix: \"%\"; tickInterval: 25; tickLabelsVisible: true\n        valueToolTipVisible: true; defaultValue: 64; defaultIndicatorVisible: true\n        Accessible.name: qsTr(\"Progress\")\n    }\n    CatButton { text: qsTr(\"Reset\"); onClicked: progress.reset() }\n}"
        },
        {
            id: "tooltip",
            name: "CatToolTip",
            category: "feedback",
            title: qsTr("Tooltip"),
            description: qsTr("Contextual help when you hover over a control."),
            icon: "feedback",
            code: "CatButton {\n    id: button\n    text: qsTr(\"Hover over me\")\n    CatToolTip {\n        visible: button.hovered\n        text: qsTr(\"A little context, right where you need it.\")\n    }\n}"
        },
        {
            id: "pagination",
            name: "CatPagination",
            category: "navigation",
            title: qsTr("Pagination"),
            description: qsTr("Navigate larger collections one page at a time."),
            icon: "navigation",
            code: "CatPagination { total: 190; pageSize: 10 }"
        },
        {
            id: "calendar",
            name: "CatCalendar",
            category: "navigation",
            title: qsTr("Calendar"),
            description: qsTr("Browse months and select dates in your language."),
            icon: "navigation",
            code: "CatCalendar {\n    onDateSelected: value => console.log(value)\n}"
        },
        {
            id: "popup",
            name: "CatPopup",
            category: "navigation",
            title: qsTr("Popup"),
            description: qsTr("Focused content above the current page."),
            icon: "navigation",
            code: "Item {\n    width: 360; height: 240\n    CatPopup {\n        id: popup\n        anchors.centerIn: parent\n        width: 300\n        contentItem: CatLabel { text: qsTr(\"A moment of focus.\") }\n    }\n    CatButton { text: qsTr(\"Open popup\"); onClicked: popup.open() }\n}"
        },
        {
            id: "drawer",
            name: "CatDrawer",
            category: "navigation",
            title: qsTr("Drawer"),
            description: qsTr("Extra space for navigation or contextual content."),
            icon: "navigation",
            code: "Item {\n    width: 360; height: 240\n    CatDrawer {\n        id: drawer\n        width: 280\n        height: parent.height\n        contentItem: CatLabel { text: qsTr(\"More room for the details.\") }\n    }\n    CatButton { text: qsTr(\"Open drawer\"); onClicked: drawer.open() }\n}"
        },
        {
            id: "table",
            name: "CatTableView",
            category: "data",
            title: qsTr("Table"),
            description: qsTr("Structured data backed by a C++ table model."),
            icon: "data",
            code: "Item {\n    width: 500; height: 200\n    TableModel {\n        id: rows\n        columns: [\"component\", \"state\"]\n        headers: [qsTr(\"Component\"), qsTr(\"Status\")]\n        Component.onCompleted: {\n            append({component: \"CatButton\", state: qsTr(\"Ready\")})\n            append({component: \"CatTextField\", state: qsTr(\"Ready\")})\n        }\n    }\n    CatTableView { anchors.fill: parent; model: rows }\n}"
        },
        {
            id: "chat",
            name: "CatChatView",
            category: "data",
            title: qsTr("Conversation"),
            description: qsTr("Messages with sender, delivery state and a composer."),
            icon: "data",
            code: "import QtQuick.Layouts\n\nColumnLayout {\n    width: 500; height: 280\n    ChatModel { id: messages }\n    CatChatView { model: messages; currentUser: \"me\"; Layout.fillWidth: true; Layout.fillHeight: true }\n    CatChatInput {\n        Layout.fillWidth: true\n        onSubmitted: text => messages.append({sender: \"me\", content: text, status: ChatModel.Sent})\n    }\n}"
        },
        {
            id: "chatinput",
            name: "CatChatInput",
            category: "data",
            title: qsTr("Message input"),
            description: qsTr("A message composer with a submit action."),
            icon: "data",
            code: "CatChatInput { onSubmitted: text => console.log(text) }"
        },
        {
            id: "flip",
            name: "CatFlipView",
            category: "motion",
            title: qsTr("Flip view"),
            description: qsTr("Switch between two faces of the same surface."),
            icon: "motion",
            code: "Column {\n    width: 240; spacing: 12\n    CatFlipView {\n        id: card\n        width: 200; height: 120\n        front: CatPane { anchors.fill: parent; contentItem: CatLabel { text: qsTr(\"Front\") } }\n        back: CatPane { anchors.fill: parent; contentItem: CatLabel { text: qsTr(\"Back\") } }\n    }\n    CatButton { text: qsTr(\"Flip card\"); onClicked: card.flipped = !card.flipped }\n}"
        },
        {
            id: "marquee",
            name: "CatMarquee",
            category: "motion",
            title: qsTr("Marquee"),
            description: qsTr("Let longer text move through a compact space."),
            icon: "motion",
            code: "CatMarquee {\n    width: 200\n    text: qsTr(\"Thoughtful details make everyday interfaces feel better.\")\n    duration: 3500\n}"
        },
        {
            id: "typewriter",
            name: "CatTypewriter",
            category: "motion",
            title: qsTr("Typewriter"),
            description: qsTr("Reveal text gradually with a restart action."),
            icon: "motion",
            code: "CatTypewriter {\n    fullText: qsTr(\"Every good interface starts with a small detail.\")\n    interval: 70\n}"
        },
        {
            id: "imagemotion",
            name: "CatImageMotion",
            category: "motion",
            title: qsTr("Image motion"),
            description: qsTr("A gentle scale and opacity animation for images."),
            icon: "motion",
            code: "CatImageMotion {\n    source: \"qrc:/qt/qml/QCatUiKit/Gallery/assets/preview-art.svg\"\n    width: 240; height: 140\n    fromScale: 0.94; toScale: 1\n    fromOpacity: 0.65; toOpacity: 1\n}"
        },
        {
            id: "image",
            name: "CatImage",
            category: "media",
            title: qsTr("Image"),
            description: qsTr("Asynchronous images with aspect-ratio preservation."),
            icon: "media",
            code: "CatImage {\n    source: \"qrc:/qt/qml/QCatUiKit/Gallery/assets/preview-art.svg\"\n    width: 280; height: 164\n}"
        },
        {
            id: "gradient",
            name: "CatConicalGradient",
            category: "media",
            title: qsTr("Conical gradient"),
            description: qsTr("A color sweep around the center of a surface."),
            icon: "media",
            code: "CatConicalGradient {\n    width: 180; height: 180\n    startColor: \"#1677ff\"\n    endColor: \"#a8d6ff\"\n}"
        },
        {
            id: "effect",
            name: "CatEffect",
            category: "media",
            title: qsTr("Visual effect"),
            description: qsTr("Add a soft shadow to a rendered source."),
            icon: "media",
            code: "Item {\n    width: 240; height: 140\n    CatImage {\n        id: picture\n        source: \"qrc:/qt/qml/QCatUiKit/Gallery/assets/preview-art.svg\"\n        width: 240; height: 140\n        visible: false\n    }\n    CatEffect { source: picture; anchors.fill: picture }\n}"
        },
        {
            id: "drag",
            name: "CatDragHandle",
            category: "tools",
            title: qsTr("Drag handle"),
            description: qsTr("Move an item with mouse or touch."),
            icon: "tools",
            code: "Rectangle {\n    id: tile\n    width: 160; height: 90\n    color: Theme.tokens.surfaceHover\n    CatDragHandle { anchors.fill: parent; target: tile }\n}"
        },
        {
            id: "resize",
            name: "CatResizeHandle",
            category: "tools",
            title: qsTr("Resize handle"),
            description: qsTr("Resize a surface within explicit bounds."),
            icon: "tools",
            code: "Rectangle {\n    id: tile\n    width: 200; height: 110\n    color: Theme.tokens.surfaceHover\n    CatResizeHandle {\n        anchors.right: parent.right\n        anchors.bottom: parent.bottom\n        target: tile\n        maximumWidth: 300; maximumHeight: 180\n    }\n}"
        },
        {
            id: "fps",
            name: "CatFpsCounter",
            category: "tools",
            title: qsTr("Frame counter"),
            description: qsTr("Observe the rate of Qt Quick frame callbacks."),
            icon: "tools",
            code: "CatFpsCounter {}"
        }
    ])
    function componentForId(id: string): var {
        const canonical = GalleryChartCatalog.canonicalId(id);
        return components.find(entry => entry.id === canonical) || null;
    }
    function categoryTitle(id: string): string {
        const category = categories.find(entry => entry.id === id);
        return category ? category.title : qsTr("All components");
    }
}
