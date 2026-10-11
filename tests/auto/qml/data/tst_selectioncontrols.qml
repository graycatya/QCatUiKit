// SPDX-License-Identifier: Apache-2.0
import QtQuick
import QtTest
import QCatUiKit

TestCase {
    id: test
    name: "SelectionControls"
    when: windowShown
    width: 800
    height: 700
    visible: true
    Component {
        id: checkFactory
        CatCheckBox {
            text: "Notifications"
            width: 200
        }
    }
    Component {
        id: switchFactory
        CatSwitch {
            text: "Wi-Fi"
            width: 200
        }
    }
    Component {
        id: radiosFactory
        Column {
            CatRadioButton {
                objectName: "first"
                text: "Local"
                checked: true
            }
            CatRadioButton {
                objectName: "second"
                text: "Cloud"
            }
        }
    }
    Component {
        id: sliderFactory
        CatSlider {
            width: 240
            from: 0
            to: 100
            value: 50
            stepSize: 10
            snapMode: CatSlider.SnapAlways
        }
    }
    Component {
        id: comboFactory
        CatComboBox {
            width: 200
            model: ["Local", "Cloud", "Remote"]
        }
    }
    function init() {
        failOnWarning(/.*/);
        Theme.reducedMotion = true;
    }
    function cleanup() {
        Theme.reducedMotion = false;
    }
    Component {
        id: themeFactory
        ThemePalette {}
    }
    function test_compactComboDisplayFits_data() {
        const rows = [];
        for (const density of [0, 1, 2])
            rows.push({
                          tag: "density_" + density,
                          density: density
                      });
        return rows;
    }
    function test_compactComboDisplayFits(data) {
        const colors = createTemporaryObject(themeFactory, test, {
                                                 density: data.density
                                             });
        const combo = createTemporaryObject(comboFactory, test, {
                                                theme: colors,
                                                width: 140,
                                                model: ["English", "中文"]
                                            });
        verify(waitForRendering(combo));
        const text = combo.contentItem;
        verify(text.width - text.leftPadding - text.rightPadding >= text.contentWidth,
               "Compact label must fit horizontally: width=" + text.width + " content=" + text.contentWidth + " padding=" + text.leftPadding + "," + text.rightPadding);
        verify(text.height - text.topPadding - text.bottomPadding >= text.contentHeight,
               "Combo label must fit vertically");
    }
    function test_checkStates() {
        const check = createTemporaryObject(checkFactory, test, {
                                                tristate: true
                                            });
        verify(waitForRendering(check));
        mouseClick(check, 100, check.height / 2);
        compare(check.checkState, Qt.PartiallyChecked);
        check.forceActiveFocus();
        keyClick(Qt.Key_Space);
        compare(check.checkState, Qt.Checked);
        keyClick(Qt.Key_Space);
        compare(check.checkState, Qt.Unchecked);
        check.enabled = false;
        mouseClick(check, 100, check.height / 2);
        compare(check.checkState, Qt.Unchecked);
    }
    function test_radioExclusivity() {
        const group = createTemporaryObject(radiosFactory, test);
        verify(waitForRendering(group));
        const first = findChild(group, "first");
        const second = findChild(group, "second");
        second.forceActiveFocus();
        keyClick(Qt.Key_Space);
        verify(second.checked);
        verify(!first.checked);
        mouseClick(first, first.width / 2, first.height / 2);
        verify(first.checked);
        verify(!second.checked);
    }
    function test_switchTouchAndKeyboard() {
        const toggle = createTemporaryObject(switchFactory, test);
        verify(waitForRendering(toggle));
        const touch = touchEvent(toggle);
        touch.press(0, toggle, 100, toggle.height / 2).commit();
        touch.release(0, toggle, 100, toggle.height / 2).commit();
        verify(toggle.checked);
        toggle.forceActiveFocus();
        keyClick(Qt.Key_Space);
        verify(!toggle.checked);
        toggle.LayoutMirroring.enabled = true;
        compare(toggle.visualPosition, 1);
    }
    function test_slider_data() {
        return [
                    {
                        tag: "horizontal",
                        orientation: Qt.Horizontal
                    },
                    {
                        tag: "vertical",
                        orientation: Qt.Vertical
                    }
                ];
    }
    function test_slider(data) {
        const slider = createTemporaryObject(sliderFactory, test, {
                                                 orientation: data.orientation
                                             });
        if (data.orientation === Qt.Vertical) {
            slider.width = 40;
            slider.height = 240;
        }
        verify(waitForRendering(slider));
        slider.forceActiveFocus();
        keyClick(data.orientation === Qt.Horizontal ? Qt.Key_Right : Qt.Key_Up);
        compare(slider.value, 60);
        mouseClick(slider, data.orientation === Qt.Horizontal ? slider.width - 8 : slider.width / 2,
                   data.orientation === Qt.Horizontal ? slider.height / 2 : 8);
        compare(slider.value, 100);
        slider.enabled = false;
        mouseClick(slider, 8, slider.height - 8);
        compare(slider.value, 100);
    }
    function test_comboKeyboardAndPopup() {
        const combo = createTemporaryObject(comboFactory, test);
        verify(waitForRendering(combo));
        combo.forceActiveFocus();
        keyClick(Qt.Key_Down);
        compare(combo.currentIndex, 1);
        mouseClick(combo, 100, combo.height / 2);
        tryCompare(combo.popup, "opened", true);
        keyClick(Qt.Key_Down);
        keyClick(Qt.Key_Return);
        tryCompare(combo.popup, "opened", false);
        compare(combo.currentIndex, 2);
        combo.editable = true;
        combo.contentItem.forceActiveFocus();
        combo.contentItem.selectAll();
        keyClick(Qt.Key_X);
        compare(combo.editText, "x");
    }
    function test_comboRoles() {
        const combo = createTemporaryObject(comboFactory, test, {
                                                textRole: "name",
                                                valueRole: "id",
                                                model: [
                                                    {
                                                        name: "Local",
                                                        id: 10
                                                    },
                                                    {
                                                        name: "Cloud",
                                                        id: 20
                                                    }
                                                ]
                                            });
        compare(combo.displayText, "Local");
        combo.currentIndex = 1;
        compare(combo.displayText, "Cloud");
        compare(combo.currentValue, 20);
    }
}
