// SPDX-License-Identifier: Apache-2.0
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Basic as B
import QtTest
import QCatUiKit

TestCase {
    id: test
    name: "ExtendedControls"
    when: windowShown
    width: 900
    height: 800
    visible: true
    property var objects: []
    SignalSpy {
        id: spy
    }
    function create(source) {
        const object = Qt.createQmlObject(
                  'import QtQuick; import QtQuick.Layouts; import QtQuick.Controls.Basic as B; import QCatUiKit; '
                  + source, test);
        verify(object !== null);
        objects.push(object);
        return object;
    }
    function init() {
        failOnWarning(/.*/);
        Theme.reducedMotion = true;
        spy.clear();
    }
    function cleanup() {
        spy.target = null;
        for (const object of objects)
            object.destroy();
        objects = [];
        wait(10);
        Theme.reducedMotion = false;
    }
    function watch(object, signal) {
        spy.target = null;
        spy.signalName = signal;
        spy.target = object;
        spy.clear();
    }
    function test_numberExpressionsAndBounds() {
        const number = create(
                  'CatNumberBox { width: 260; minimum: 0; maximum: 100; smallChange: 0.5; acceptsExpression: true }');
        number.text = "(2 + 3) * 4";
        verify(number.commitText());
        compare(number.value, 20);
        number.text = "1 / 0";
        verify(!number.commitText());
        verify(number.invalid);
        compare(number.value, 20);
        number.text = "Math.random()";
        verify(!number.commitText());
        compare(number.value, 20);
        number.forceActiveFocus();
        keyClick(Qt.Key_Up);
        compare(number.value, 20.5);
        verify(!number.invalid);
        number.commitValue(200);
        compare(number.value, 100);
        number.commitValue(-10);
        compare(number.value, 0);
    }
    function test_numberLocale() {
        const number = create(
                  'CatNumberBox { width: 260; locale: Qt.locale("de_DE"); decimals: 2 }');
        number.text = "12,50";
        verify(number.commitText());
        compare(number.value, 12.5);
        verify(number.text.includes(","));
        number.text = "invalid";
        verify(!number.commitText());
        verify(number.invalid);
    }
    function test_spinEditingAndButtons() {
        const spin = create('CatSpinBox { width: 220; from: 0; to: 20; value: 5; editable: true }');
        verify(waitForRendering(spin));
        mouseClick(spin, spin.width - 10, spin.height / 2);
        compare(spin.value, 6);
        spin.contentItem.forceActiveFocus();
        spin.contentItem.selectAll();
        keyClick(Qt.Key_1);
        keyClick(Qt.Key_2);
        keyClick(Qt.Key_Return);
        compare(spin.value, 12);
        spin.value = 20;
        mouseClick(spin, spin.width - 10, spin.height / 2);
        compare(spin.value, 20);
    }
    function test_passwordRevealKeyboard() {
        const password = create('CatPasswordField { width: 260; text: "secret" }');
        verify(waitForRendering(password));
        compare(password.echoMode, TextInput.Password);
        password.forceActiveFocus();
        keyClick(Qt.Key_Tab);
        keyClick(Qt.Key_Space);
        verify(password.revealed);
        compare(password.echoMode, TextInput.Normal);
        compare(password.password, "secret");
    }
    function test_suggestionsKeyboard() {
        const search = create(
                  'CatAutoSuggestBox { width: 260; suggestions: ["Documents", "Drawings", "Images"] }');
        watch(search, "suggestionAccepted");
        verify(waitForRendering(search));
        search.forceActiveFocus();
        keyClick(Qt.Key_D);
        tryCompare(search, "isSuggestionListOpen", true);
        keyClick(Qt.Key_Down);
        keyClick(Qt.Key_Return);
        compare(search.text, "Drawings");
        compare(spy.count, 1);
        verify(!search.isSuggestionListOpen);
    }
    function test_multiselectAndColor() {
        const selection = create(
                  'CatMultiSelectComboBox { width: 240; model: [{name: "One"}, {name: "Two"}]; textRole: "name" }');
        watch(selection, "selectionEdited");
        selection.toggleIndex(1);
        selection.toggleIndex(0);
        compare(selection.selectedTexts.join(","), "One,Two");
        compare(selection.selectedCount, 2);
        selection.toggleIndex(1);
        compare(selection.selectedCount, 1);
        selection.toggleIndex(10);
        compare(spy.count, 3);
        const picker = create('CatColorPicker { width: 220 }');
        watch(picker, "colorSelected");
        picker.chooseColor("#008577");
        compare(picker.selectedColor, "#008577");
        compare(spy.count, 1);
    }
    function test_ratingAndTriState() {
        const rating = create('CatRatingControl { value: -1; caption: "Rating" }');
        verify(waitForRendering(rating));
        mouseClick(rating, Theme.tokens.controlHeight * 2.5, Theme.tokens.controlHeight / 2);
        compare(rating.value, 3);
        mouseClick(rating, Theme.tokens.controlHeight * 2.5, Theme.tokens.controlHeight / 2);
        compare(rating.value, -1);
        rating.forceActiveFocus();
        keyClick(Qt.Key_Right);
        compare(rating.value, 1);
        keyClick(Qt.Key_Delete);
        compare(rating.value, -1);
        rating.isReadOnly = true;
        mouseClick(rating, 16, 16);
        compare(rating.value, -1);
        const toggle = create('CatToggleButton { x: 300; text: "Mixed"; threeState: true }');
        for (const state of [Qt.PartiallyChecked, Qt.Checked, Qt.Unchecked]) {
            mouseClick(toggle, toggle.width / 2, toggle.height / 2);
            compare(toggle.checkState, state);
        }
    }
    function test_fileDropContract() {
        const drop = create('CatFileDropZone { acceptedExtensions: [".txt", ".md"]; width: 300 }');
        verify(drop.acceptsUrls(["file:///notes.txt", "file:///README.MD"]));
        verify(!drop.acceptsUrls(["file:///photo.png"]));
        verify(!drop.acceptsUrls(["https://example.com/notes.txt"]));
        verify(!drop.acceptsUrls([]));
        watch(drop, "browseRequested");
        verify(waitForRendering(drop));
        // The browse action is a standard button reachable through Tab.
        drop.forceActiveFocus();
        keyClick(Qt.Key_Tab);
        keyClick(Qt.Key_Space);
        compare(spy.count, 1);
    }
    function test_listAndGridSelection() {
        for (const type of ["CatListView", "CatGridView", "CatFlowView"]) {
            const view = create(type
                                + ' { width: 300; height: 180; model: ["One", "Two", "Three"] }');
            watch(view, "activated");
            verify(waitForRendering(view));
            mouseClick(view, 50, 16);
            compare(view.currentIndex, 0);
            compare(spy.count, 1);
            view.y = 300;
        }
    }
    function test_treeHierarchy() {
        const root = create(
                  'Item { width: 300; height: 220; property alias tree: tree; property alias hierarchy: hierarchy; TreeModel { id: hierarchy; items: [{text: "Parent", children: [{text: "Child"}]}] } CatTreeView { id: tree; anchors.fill: parent; model: hierarchy } }');
        verify(waitForRendering(root));
        compare(root.tree.rows, 1);
        root.tree.expand(0);
        tryCompare(root.tree, "rows", 2);
        root.tree.collapse(0);
        tryCompare(root.tree, "rows", 1);
        root.hierarchy.items = [];
        tryCompare(root.tree, "rows", 0);
    }
    function test_carouselAndStack() {
        const carousel = create('CatCarousel { width: 320; height: 160; CatPane { } CatPane { } }');
        verify(waitForRendering(carousel));
        compare(carousel.count, 2);
        mouseDrag(carousel, 260, 80, -220, 0);
        tryCompare(carousel, "currentIndex", 1);
        const stack = create(
                  'CatStackView { y: 200; width: 320; height: 160; initialItem: Component { CatPane { } } }');
        compare(stack.depth, 1);
        stack.push('qrc:/qt/qml/QCatUiKit/qml/CatPane.qml');
        compare(stack.depth, 2);
        stack.pop();
        compare(stack.depth, 1);
    }
    function test_splitResizing() {
        const split = create(
                  'CatSplitView { width: 340; height: 160; property alias first: first; CatPane { id: first; B.SplitView.preferredWidth: 120; B.SplitView.minimumWidth: 60 } CatPane { B.SplitView.fillWidth: true; B.SplitView.minimumWidth: 60 } }');
        verify(waitForRendering(split));
        const before = split.first.width;
        mouseDrag(split, before + 4, 80, 50, 0);
        verify(split.first.width > before + 25);
    }
    function test_selectorAndBreadcrumb() {
        for (const type of ["CatSelectorBar", "CatPivot"]) {
            const selector = create(type + ' { width: 320; model: ["One", "Two"] }');
            verify(waitForRendering(selector));
            mouseClick(selector, 120, 16);
            compare(selector.selectedIndex, 1);
            selector.y = 300;
        }
        const path = create(
                  'CatBreadcrumb { width: 320; model: ["Home", "Files", "Notes"]; autoTruncateOnItemClick: true }');
        watch(path, "itemClicked");
        verify(waitForRendering(path));
        mouseClick(path, 140, 16);
        compare(spy.count, 1);
        compare(path.model.length, 2);
    }
    function test_stepperRequestsOnly() {
        const stepper = create('CatStepper { width: 320; model: ["One", "Two"]; currentIndex: 0 }');
        watch(stepper, "stepRequested");
        verify(waitForRendering(stepper));
        mouseClick(stepper, 200, 16);
        compare(spy.count, 1);
        compare(spy.signalArguments[0][0], 1);
        compare(stepper.currentIndex, 0);
    }
    function test_tabsPreserveStateAndRequestClose() {
        const tabs = create(
                  'CatTabView { width: 320; height: 220; titles: ["One", "Two"]; property alias draft: input.text; CatPane { contentItem: CatTextField { id: input; text: "My draft" } } CatPane { } }');
        verify(waitForRendering(tabs));
        compare(tabs.tabCount, 2);
        mouseClick(tabs, 124, 16);
        compare(tabs.selectedIndex, 1);
        mouseClick(tabs, 40, 16);
        compare(tabs.selectedIndex, 0);
        compare(tabs.draft, "My draft");
        tabs.tabsClosable = true;
        watch(tabs, "closeRequested");
        verify(waitForRendering(tabs));
        mouseClick(tabs, 212, 16);
        compare(spy.count, 1);
        compare(spy.signalArguments[0][0], 1);
        compare(tabs.tabCount, 2);
    }
    function test_expanderAndAccordion() {
        const accordion = create(
                  'CatAccordion { width: 300; property alias first: first; property alias second: second; CatExpander { id: first; width: parent.width; headerText: "One"; expanded: true; CatLabel { text: "Details" } } CatExpander { id: second; width: parent.width; headerText: "Two"; CatLabel { text: "Details" } } }');
        verify(waitForRendering(accordion));
        compare(accordion.count, 2);
        mouseClick(accordion, 100, accordion.first.height + accordion.spacing + 16);
        verify(accordion.second.expanded);
        verify(!accordion.first.expanded);
    }
    function test_dateAndTimeBounds() {
        for (const type of ["CatCalendarDatePicker", "CatDatePicker"]) {
            const picker = create(type
                                  + ' { minimumDate: new Date(2026,0,1); maximumDate: new Date(2026,11,31) }');
            compare(picker.selectedDate, null);
            verify(!picker.choose(new Date(2025, 11, 31)));
            verify(picker.choose(new Date(2026, 1, 28)));
            compare(picker.selectedDate.getDate(), 28);
        }
        const time = create('CatTimePicker { }');
        verify(!time.choose(24, 0));
        verify(!time.choose(12, 60));
        verify(time.choose(9, 30));
        compare(time.selectedTime.getHours(), 9);
        compare(time.selectedTime.getMinutes(), 30);
    }
    function test_menuKeyboardAndDialogButtons() {
        const menu = create(
                  'CatMenu { CatMenuItem { text: "One"; checkable: true } CatMenuItem { text: "Two" } }');
        watch(menu.itemAt(0), "triggered");
        menu.open();
        tryCompare(menu, "opened", true);
        menu.itemAt(0).forceActiveFocus();
        keyClick(Qt.Key_Return);
        tryCompare(menu, "opened", false);
        verify(menu.itemAt(0).checked);
        compare(spy.count, 1);
        const dialog = create(
                  'CatDialog { title: "Save?"; standardButtons: CatDialog.Ok | CatDialog.Cancel; contentItem: CatLabel { text: "Your changes" } }');
        watch(dialog, "accepted");
        dialog.open();
        tryCompare(dialog, "opened", true);
        const button = dialog.standardButton(CatDialog.Ok);
        verify(button !== null);
        mouseClick(button, button.width / 2, button.height / 2);
        tryCompare(dialog, "opened", false);
        compare(spy.count, 1);
    }
    function test_toastTimeoutAndNavigationDrawer() {
        const toast = create(
                  'CatToast { message: "Saved"; duration: 100; pauseOnHoverEnabled: false }');
        toast.open();
        tryCompare(toast, "opened", true);
        tryCompare(toast, "opened", false, 1000);
        const navigation = create(
                  'CatNavigationView { width: 360; height: 200; model: ["One", "Two"] }');
        verify(waitForRendering(navigation));
        navigation.paneOpen = true;
        wait(10);
        verify(navigation.paneOpen);
        navigation.activate(1);
        compare(navigation.selectedIndex, 1);
        verify(!navigation.paneOpen);
    }
    function test_reducedMotionAndSpatialFallback() {
        const shimmer = create('CatShimmer { }');
        const particles = create('CatParticleBackdrop { y: 100 }');
        const spatial = create('CatSpatialView { y: 350 }');
        verify(waitForRendering(shimmer));
        const phase = shimmer.shimmerProgress;
        const particlePhase = particles.phase;
        wait(80);
        compare(shimmer.shimmerProgress, phase);
        compare(particles.phase, particlePhase);
        verify(spatial.flatMode);
    }
    function test_spatialHostAndCameraDistance() {
        const spatial = create(
                  'CatSpatialView { width: 320; height: 220; cameraDistance: 800; forceFlat: true; property alias surface: surface; Item { CatSpatialItem { id: surface; width: 100; height: 100; rotation3d: Qt.vector3d(20, 30, 0); position: Qt.vector3d(20, 20, 100) } } }');
        verify(waitForRendering(spatial));
        compare(spatial.surface.spatialHost, spatial);
        compare(spatial.surface.effectiveCameraDistance, 800);
        verify(spatial.surface.flatMode);
        compare(spatial.surface.scale, 1);
        spatial.cameraDistance = 1200;
        compare(spatial.surface.effectiveCameraDistance, 1200);
        const point = spatial.surface.mapToItem(spatial, 10, 10);
        compare(point.x, 30);
        compare(point.y, 30);
    }
}
