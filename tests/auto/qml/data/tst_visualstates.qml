// SPDX-License-Identifier: Apache-2.0
import QtQuick
import QtQuick.Window
import QtTest
import QCatUiKit

TestCase {
    id: test
    name: "ExistingControlStates"
    when: windowShown
    width: 800
    height: 600
    visible: true
    Component { id: buttonFactory; CatButton { text: "A useful action" } }
    Component { id: progressFactory; CatProgressBar { width: 240; height: 8; indeterminate: true } }
    Component { id: fieldFactory; CatTextField { placeholderText: "Your message" } }
    Component { id: titleBarFactory; CatTitleBar { width: 400 } }
    Component { id: windowFactory; Window { width: 200; height: 100 } }
    function init() { failOnWarning(/.*/); Theme.reducedMotion = true; }
    function cleanup() { Theme.density = ThemePalette.Desktop; Theme.reducedMotion = false; }
    function test_buttonSizing_data() {
        return [ThemePalette.Desktop, ThemePalette.Touch, ThemePalette.Mobile].map(density => ({tag: String(density), density: density}));
    }
    function test_buttonSizing(data) {
        Theme.density = data.density;
        const button = createTemporaryObject(buttonFactory, test);
        verify(button.height >= Theme.tokens.controlHeight);
        verify(button.availableHeight >= button.implicitContentHeight, "The content must fit between the vertical padding");
        button.font.pixelSize = 32;
        verify(button.availableHeight >= button.implicitContentHeight);
        button.width = 100;
        verify(waitForRendering(button));
        verify(button.implicitWidth > button.width, "A narrow explicit width must not shrink the natural size");
    }
    function test_reducedIndeterminateRemainsVisible() {
        const progress = createTemporaryObject(progressFactory, test);
        verify(waitForRendering(progress));
        const first = grabImage(progress);
        wait(100);
        verify(first.equals(grabImage(progress)));
        verify(first.pixel(120, 4).toString() !== first.pixel(12, 4).toString(), "Reduced motion must show a stationary segment");
        Theme.reducedMotion = false;
        wait(100);
        Theme.reducedMotion = true;
        wait(10);
        compare(grabImage(progress).pixel(120, 4), first.pixel(120, 4));
        progress.indeterminate = false;
        progress.value = 0.5;
        wait(10);
        verify(grabImage(progress).pixel(12, 4).toString() !== first.pixel(12, 4).toString());
    }
    function test_disabledField() {
        const field = createTemporaryObject(fieldFactory, test);
        field.enabled = false;
        compare(field.color, Theme.tokens.textDisabled);
        compare(field.placeholderTextColor, Theme.tokens.textDisabled);
    }
    function test_fieldAlignmentAcrossDensity_data() {
        return [{tag: "latin", text: "Input text"},
                {tag: "chinese", text: "输入内容"},
                {tag: "arabic", text: "مرحبا"}];
    }
    function test_fieldAlignmentAcrossDensity(data) {
        const field = createTemporaryObject(fieldFactory, test, {text: data.text});
        field.forceActiveFocus();
        field.select(1, 3);
        const selected = field.selectedText;
        for (const density of [0, 1, 2, 0, 2, 1, 0]) {
            Theme.density = density;
            verify(waitForRendering(field));
            compare(field.text, data.text);
            compare(field.selectedText, selected);
            const cursor = field.cursorRectangle;
            fuzzyCompare(cursor.y + cursor.height / 2, field.height / 2, 1,
                         "Input text must remain vertically centered at density " + density);
        }
    }
    function test_titleBarPressedState() {
        const target = createTemporaryObject(windowFactory, test);
        const bar = createTemporaryObject(titleBarFactory, test, {targetWindow: target});
        verify(bar !== null);
        verify(waitForRendering(bar));
        const minimize = findChild(bar, "titleBarMinimize");
        const maximize = findChild(bar, "titleBarMaximize");
        const close = findChild(bar, "titleBarClose");
        verify(minimize !== null && maximize !== null && close !== null);

        verify(Qt.colorEqual(minimize.faceColor, Theme.tokens.surface));
        mouseMove(minimize, 20, 20);
        tryCompare(minimize, "hovered", true);
        verify(Qt.colorEqual(minimize.faceColor, Theme.tokens.surfaceHover));
        mousePress(minimize, 20, 20);
        tryCompare(minimize, "pressed", true);
        verify(Qt.colorEqual(minimize.faceColor, Theme.tokens.surfacePressed));
        verify(!Qt.colorEqual(minimize.faceColor, Theme.tokens.disabled));
        bar.targetWindow = null;
        mouseRelease(minimize, 20, 20);

        bar.targetWindow = target;
        verify(Qt.colorEqual(maximize.faceColor, Theme.tokens.surface));
        mouseMove(maximize, 20, 20);
        tryCompare(maximize, "hovered", true);
        verify(Qt.colorEqual(maximize.faceColor, Theme.tokens.surfaceHover));
        mousePress(maximize, 20, 20);
        tryCompare(maximize, "pressed", true);
        verify(Qt.colorEqual(maximize.faceColor, Theme.tokens.surfacePressed));
        bar.targetWindow = null;
        mouseRelease(maximize, 20, 20);

        bar.targetWindow = target;
        mouseMove(close, 20, 20);
        tryCompare(close, "hovered", true);
        verify(Qt.colorEqual(close.faceColor, Theme.tokens.danger));
        mousePress(close, 20, 20);
        tryCompare(close, "pressed", true);
        verify(Qt.colorEqual(close.faceColor, Qt.darker(Theme.tokens.danger, 1.1)));
        bar.targetWindow = null;
        mouseRelease(close, 20, 20);
        mouseMove(test, 0, 0);
    }
}
