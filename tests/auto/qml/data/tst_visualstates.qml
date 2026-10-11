// SPDX-License-Identifier: Apache-2.0
import QtQuick
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
}
