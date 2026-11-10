// SPDX-License-Identifier: Apache-2.0
import QtQuick
import QtTest
import QCatUiKit

TestCase {
    id: test
    name: "QCatUiKitContracts"
    when: windowShown
    width: 800
    height: 700
    visible: true
    Component {
        id: buttonFactory
        CatButton {
            text: "Run"
            width: 140
            height: 48
        }
    }
    Component {
        id: inputFactory
        CatIpAddressField {}
    }
    Component {
        id: pagerFactory
        CatPagination {
            total: 100
        }
    }
    Component {
        id: calendarFactory
        CatCalendar {}
    }
    Component {
        id: paletteFactory
        ThemePalette {}
    }
    SignalSpy {
        id: clickSpy
        signalName: "clicked"
    }
    SignalSpy {
        id: pageSpy
        signalName: "pageRequested"
    }
    SignalSpy {
        id: dateSpy
        signalName: "dateSelected"
    }
    function init() {
        failOnWarning(/.*/);
        clickSpy.clear();
        pageSpy.clear();
        dateSpy.clear();
    }
    function cleanup() {
        clickSpy.target = null;
        pageSpy.target = null;
        dateSpy.target = null;
        Theme.colorScheme = ThemePalette.Light;
        Theme.density = ThemePalette.Desktop;
        Theme.reducedMotion = false;
        Theme.colorPalette = ({});
    }
    function test_buttonInput() {
        const button = createTemporaryObject(buttonFactory, test);
        clickSpy.target = button;
        verify(waitForRendering(button));
        mouseClick(button, 70, 24);
        compare(clickSpy.count, 1);
        button.forceActiveFocus();
        keyClick(Qt.Key_Space);
        compare(clickSpy.count, 2);
        button.enabled = false;
        mouseClick(button, 70, 24);
        keyClick(Qt.Key_Space);
        compare(clickSpy.count, 2);
    }
    function test_moduleInfo() {
        const base = QCatUiKit.versionMajor + "." + QCatUiKit.versionMinor + "." + QCatUiKit.versionPatch;
        verify(QCatUiKit.releaseChannel === "stable" || QCatUiKit.releaseChannel === "experimental");
        compare(QCatUiKit.isExperimental, QCatUiKit.releaseChannel === "experimental");
        compare(QCatUiKit.isStable, !QCatUiKit.isExperimental);
        compare(QCatUiKit.version, base + (QCatUiKit.isExperimental ? "-experimental" : ""));
        verify(QCatUiKit.abiVersion > 0);
    }
    function test_loading() {
        const button = createTemporaryObject(buttonFactory, test, {
            loading: true
        });
        clickSpy.target = button;
        verify(waitForRendering(button));
        mouseClick(button, 70, 24);
        compare(clickSpy.count, 0);
        button.loading = false;
        verify(button.enabled);
        mouseClick(button, 70, 24);
        compare(clickSpy.count, 1);
    }
    function test_popupEscape() {
        const popup = Qt.createQmlObject('import QtQuick; import QCatUiKit; CatPopup { width: 220; height: 120 }', test);
        popup.open();
        tryCompare(popup, "opened", true);
        keyClick(Qt.Key_Escape);
        tryCompare(popup, "opened", false);
        popup.destroy();
    }
    function test_touchActivation() {
        const button = createTemporaryObject(buttonFactory, test);
        clickSpy.target = button;
        verify(waitForRendering(button));
        const touch = touchEvent(button);
        touch.press(0, button, 70, 24).commit();
        touch.release(0, button, 70, 24).commit();
        compare(clickSpy.count, 1);
    }
    function test_loadingRestoresBinding() {
        const palette = createTemporaryObject(paletteFactory, test);
        const button = createTemporaryObject(buttonFactory, test);
        button.enabled = Qt.binding(() => !palette.reducedMotion);
        button.loading = true;
        verify(!button.enabled);
        palette.reducedMotion = true;
        button.loading = false;
        verify(!button.enabled);
        palette.reducedMotion = false;
        verify(button.enabled);
    }
    function test_ipValidation() {
        const field = createTemporaryObject(inputFactory, test);
        field.text = "192.168.1.1";
        verify(field.acceptableInput);
        field.text = "256.1.1.1";
        verify(!field.acceptableInput);
        field.text = "1.2.3";
        verify(!field.acceptableInput);
        field.text = "0.0.0.0";
        verify(field.acceptableInput);
    }
    function test_pagination() {
        const pager = createTemporaryObject(pagerFactory, test);
        pageSpy.target = pager;
        pager.requestPage(100);
        compare(pager.currentPage, 10);
        compare(pageSpy.count, 1);
        pager.requestPage(10);
        compare(pageSpy.count, 1);
        pager.total = 0;
        compare(pager.currentPage, 1);
        compare(pager.pageCount, 1);
        pager.pageSize = 0;
        verify(isFinite(pager.pageCount));
    }
    function test_calendar() {
        const calendar = createTemporaryObject(calendarFactory, test, {
            year: 2024,
            month: 11,
            minimumDate: new Date(2024, 0, 1),
            maximumDate: new Date(2025, 11, 31)
        });
        dateSpy.target = calendar;
        calendar.moveMonth(1);
        compare(calendar.year, 2025);
        compare(calendar.month, 0);
        calendar.selectDate(new Date(2024, 1, 29));
        compare(calendar.selectedDate.getDate(), 29);
        compare(dateSpy.count, 1);
        calendar.selectDate(new Date(2023, 0, 1));
        compare(dateSpy.count, 1);
    }
    function test_themeIsolation() {
        const palette = createTemporaryObject(paletteFactory, test, {
            density: ThemePalette.Touch
        });
        const local = createTemporaryObject(buttonFactory, test, {
            theme: palette
        });
        const global = createTemporaryObject(buttonFactory, test);
        compare(local.theme.tokens.controlHeight, 48);
        compare(global.theme.tokens.controlHeight, 32);
        Theme.colorScheme = ThemePalette.Dark;
        compare(global.theme.colorScheme, ThemePalette.Dark);
        compare(local.theme.colorScheme, ThemePalette.Light);
        palette.reducedMotion = true;
        compare(local.theme.tokens.motionDuration, 0);
    }
    function test_colorPaletteIsolation() {
        const teal = createTemporaryObject(paletteFactory, test, {
            colorPalette: ({surface: "#123456", primary: "#00a6a6"}),
            overrides: ({radius: 2, fontSize: 20})
        });
        const amber = createTemporaryObject(paletteFactory, test, {
            colorPalette: ({surface: "#654321", primary: "#ffb000"}),
            overrides: ({radius: 20, fontSize: 11})
        });
        const first = createTemporaryObject(buttonFactory, test, {theme: teal});
        const second = createTemporaryObject(buttonFactory, test, {theme: amber});
        verify(Qt.colorEqual(first.background.color, "#123456"));
        verify(Qt.colorEqual(second.background.color, "#654321"));
        verify(Qt.colorEqual(first.tokens.primary, "#00a6a6"));
        verify(Qt.colorEqual(second.tokens.primary, "#ffb000"));
        compare(first.background.radius, 2);
        compare(second.background.radius, 20);
        compare(first.font.pixelSize, 20);
        compare(second.font.pixelSize, 11);
        // A local palette never leaks into the default theme.
        verify(!Qt.colorEqual(first.tokens.surface, Theme.tokens.surface));
        verify(!Qt.colorEqual(second.tokens.surface, Theme.tokens.surface));
    }
    function test_colorPaletteSwitchRefreshesControl() {
        const palette = createTemporaryObject(paletteFactory, test, {
            colorPalette: ({surface: "#123456"}),
            reducedMotion: true
        });
        const button = createTemporaryObject(buttonFactory, test, {theme: palette});
        verify(Qt.colorEqual(button.background.color, "#123456"));
        palette.colorPalette = ({surface: "#abcdef"});
        wait(1);
        verify(Qt.colorEqual(button.background.color, "#abcdef"));
        palette.colorPalette = ({});
        wait(1);
        verify(Qt.colorEqual(button.background.color, Theme.tokens.surface));
    }
    function test_destroyedPaletteFallsBackToDefaultTheme() {
        Theme.reducedMotion = true;
        const palette = paletteFactory.createObject(test, {
            colorPalette: ({surface: "#123456"}),
            reducedMotion: true
        });
        const button = createTemporaryObject(buttonFactory, test, {theme: palette});
        verify(Qt.colorEqual(button.background.color, "#123456"));
        palette.destroy();
        wait(10);
        compare(button.theme, null);
        verify(Qt.colorEqual(button.background.color, Theme.tokens.surface));
    }
    function test_componentImports_data() {
        return ["CatAccordion", "CatAlert", "CatAnnotatedScrollBar", "CatAutoSuggestBox", "CatAvatar", "CatBreadcrumb", "CatBusyIndicator", "CatButton", "CatCalendar", "CatCalendarDatePicker", "CatCarousel", "CatChatInput", "CatChatView", "CatCheckBox", "CatCircularProgress", "CatCoachMark", "CatColorPicker", "CatComboBox", "CatCommandBar", "CatCommandBarFlyout", "CatCompoundButton", "CatConicalGradient", "CatContentDialog", "CatDatePicker", "CatDialog", "CatDivider", "CatDragHandle", "CatDrawer", "CatDropDownButton", "CatEffect", "CatExpander", "CatField", "CatFileDropZone", "CatFileListView", "CatFlipView", "CatFlowView", "CatFlyout", "CatFontIcon", "CatFpsCounter", "CatGridView", "CatHyperlinkButton", "CatImage", "CatImageMotion", "CatInfoBadge", "CatIpAddressField", "CatLabel", "CatListView", "CatMarquee", "CatMenu", "CatMenuBar", "CatMenuBarItem", "CatMenuItem", "CatMenuSeparator", "CatMultiSelectComboBox", "CatNavigationView", "CatNumberBox", "CatPagination", "CatPane", "CatParticleBackdrop", "CatPasswordField", "CatPipsPager", "CatPivot", "CatPopup", "CatProgressBar", "CatProgressRing", "CatRadioButton", "CatRatingControl", "CatRepeatButton", "CatResizeHandle", "CatScrollBar", "CatScrollView", "CatSelectorBar", "CatShimmer", "CatSlider", "CatRangeSlider", "CatSliderField", "CatSpatialItem", "CatSpatialView", "CatSpinBox", "CatSplashScreen", "CatSplitButton", "CatSplitView", "CatStackView", "CatStepper", "CatSwitch", "CatTabView", "CatTableView", "CatTeachingTip", "CatTextArea", "CatTextField", "CatTimePicker", "CatTimeline", "CatTitleBar", "CatToast", "CatToggleButton", "CatToggleSplitButton", "CatToolTip", "CatTreeView", "CatTypewriter", "CatWindow"].map(name => ({
                    tag: name,
                    type: name
                }));
    }
    function test_componentImports(data) {
        const object = Qt.createQmlObject('import QtQuick; import QCatUiKit; ' + data.type + ' {}', test);
        verify(object !== null);
        object.destroy();
        wait(1);
    }
}
