// SPDX-License-Identifier: Apache-2.0
import QtQuick
import QtTest
import QCatUiKit

TestCase {
    id: test
    name: "SliderVariants"
    when: windowShown
    width: 800; height: 700; visible: true
    Component { id: singleFactory; CatSlider { x: 60; y: 100; width: 400; from: -100; to: 100; value: 20; stepSize: 10 } }
    Component { id: rangeFactory; CatRangeSlider { x: 60; y: 100; width: 400; from: 0; to: 100; first.value: 20; second.value: 60; stepSize: 5; rangeDragEnabled: true } }
    Component { id: fieldFactory; CatSliderField { x: 60; y: 100; width: 500; from: -60; to: 12; value: -12; decimals: 1; stepSize: .5; resetVisible: true; defaultValue: -12 } }
    Component {
        id: boundFactory
        Item {
            width: 500; height: 200
            property real backend: -12
            property alias editor: field
            CatSliderField { id: field; width: 450; from: -60; to: 12; value: parent.backend; decimals: 1; defaultValue: -12; onValueEdited: value => parent.backend = value }
        }
    }
    SignalSpy { id: movedSpy; signalName: "moved" }
    SignalSpy { id: commitSpy }
    function init() { failOnWarning(/.*/); Theme.reducedMotion = true; }
    function cleanup() { movedSpy.target = null; commitSpy.target = null; movedSpy.clear(); commitSpy.clear(); Theme.reducedMotion = false; }
    function watch(item, range) { movedSpy.target = item; commitSpy.signalName = range ? "valuesCommitted" : "valueCommitted"; commitSpy.target = item; }
    function close(a, b) { fuzzyCompare(a, b, .00001); }
    function test_singleKeyboardResetAndFormatting() {
        const s = createTemporaryObject(singleFactory, test, {defaultValue: 40, fillOrigin: 0, tickInterval: 20, tickLabelsVisible: true});
        watch(s, false); verify(waitForRendering(s));
        s.forceActiveFocus(); keyClick(Qt.Key_Right);
        compare(s.value, 30); compare(movedSpy.count, 1); compare(commitSpy.count, 1);
        s.reset(); compare(s.value, 40); compare(commitSpy.count, 2);
        s.value = -20; compare(movedSpy.count, 2); compare(commitSpy.count, 2);
        s.locale = Qt.locale("en_US"); s.decimals = 1; s.suffix = " dB";
        compare(s.textFromValue(-12.5), "-12.5 dB");
        s.valueFormatter = v => "[" + v + "]"; compare(s.textFromValue(4), "[4]");
        s.enabled = false; keyClick(Qt.Key_Right); compare(s.value, -20);
    }
    function test_singleDeferredDrag() {
        const s = createTemporaryObject(singleFactory, test, {live: false, valueToolTipVisible: true});
        watch(s, false); verify(waitForRendering(s));
        const x = s.handle.x + s.handle.width / 2, y = s.handle.y + s.handle.height / 2;
        mousePress(s, x, y); mouseMove(s, x + 80, y, 30);
        compare(s.value, 20); verify(s.position > .65);
        mouseRelease(s, x + 80, y); verify(s.value > 20);
        compare(commitSpy.count, 1);
    }
    function test_paddingAndScaleToggle() {
        for (const factory of [singleFactory, rangeFactory]) {
            const s = createTemporaryObject(factory, test, {padding: 20, tickInterval: 20});
            compare(s.bottomPadding, 20); compare(s.rightPadding, 20);
            s.tickLabelsVisible = true;
            verify(s.bottomPadding > 20);
            s.tickLabelsVisible = false;
            compare(s.bottomPadding, 20);
            s.padding = 24; compare(s.bottomPadding, 24); compare(s.rightPadding, 24);
            s.orientation = Qt.Vertical; s.tickLabelsVisible = true;
            verify(s.rightPadding > 24); compare(s.bottomPadding, 24);
            s.tickLabelsVisible = false; compare(s.rightPadding, 24);
        }
    }
    function test_rangeConstraintsAndProgrammaticSync() {
        const s = createTemporaryObject(rangeFactory, test, {minimumSpan: 15, maximumSpan: 50});
        watch(s, true); verify(waitForRendering(s));
        s.setValues(95, 100); wait(1); close(s.first.value, 85); close(s.second.value, 100);
        s.setValues(0, 100); wait(1); close(s.second.value, 50);
        compare(movedSpy.count, 0); compare(commitSpy.count, 0);
        s.first.value = 49; s.first.moved(); close(s.first.value, 35); close(s.second.value, 50);
        s.second.value = 36; s.second.moved(); close(s.second.value, 50);
        s.setValues(20, 60); s.fixedSpan = 30; wait(1); close(s.second.value, 50);
        s.second.value = 70; s.second.moved(); close(s.first.value, 40); close(s.second.value, 70);
        s.to = 20; wait(1); close(s.first.value, 0); close(s.second.value, 20);
        s.from = 20; wait(1); close(s.first.value, 20); close(s.second.value, 20);
    }
    function test_rangeDrag_data() {
        return [
            {tag: "horizontal", orientation: Qt.Horizontal, mirrored: false, reversed: false, live: true},
            {tag: "rtl", orientation: Qt.Horizontal, mirrored: true, reversed: false, live: true},
            {tag: "vertical", orientation: Qt.Vertical, mirrored: false, reversed: false, live: true},
            {tag: "reversed", orientation: Qt.Horizontal, mirrored: false, reversed: true, live: true},
            {tag: "deferred", orientation: Qt.Horizontal, mirrored: false, reversed: false, live: false},
            {tag: "snap_release", orientation: Qt.Horizontal, mirrored: false, reversed: false, live: true, snapRelease: true}
        ];
    }
    function test_rangeDrag(data) {
        const s = createTemporaryObject(rangeFactory, test, {orientation: data.orientation, live: data.live,
            snapMode: data.snapRelease ? CatRangeSlider.SnapOnRelease : CatRangeSlider.SnapAlways});
        if (data.orientation === Qt.Vertical) { s.width = 70; s.height = 400; }
        s.LayoutMirroring.enabled = data.mirrored;
        if (data.reversed) { s.from = 100; s.to = 0; s.setValues(80, 40); }
        watch(s, true); verify(waitForRendering(s));
        const a = s.first.value, b = s.second.value;
        const x = (s.first.handle.x + s.second.handle.x + s.first.handle.width) / 2;
        const y = (s.first.handle.y + s.second.handle.y + s.first.handle.height) / 2;
        const dx = data.orientation === Qt.Horizontal ? (data.mirrored ? -70 : 70) : 0;
        const dy = data.orientation === Qt.Vertical ? -70 : 0;
        mousePress(s, x, y); verify(s.rangeDragging);
        mouseMove(s, x + dx, y + dy, 30);
        if (!data.live) { compare(s.first.value, a); compare(s.second.value, b); }
        mouseRelease(s, x + dx, y + dy);
        verify(!s.rangeDragging); close(Math.abs(s.second.value - s.first.value), 40);
        close(s.first.value, a + (data.reversed ? -20 : 20));
        compare(commitSpy.count, 1); verify(movedSpy.count > 0);
        // A large drag clamps both ends together, preserving the span.
        const midX = (s.first.handle.x + s.second.handle.x + s.first.handle.width) / 2;
        const midY = (s.first.handle.y + s.second.handle.y + s.first.handle.height) / 2;
        mouseDrag(s, midX, midY, dx * 8, dy * 8);
        close(Math.abs(s.second.value - s.first.value), 40);
        close(s.second.value, s.to);
    }
    function test_rangeHandlesKeyboardAndTouch() {
        const s = createTemporaryObject(rangeFactory, test, {minimumSpan: 10});
        watch(s, true); verify(waitForRendering(s));
        s.first.handle.forceActiveFocus(); keyClick(Qt.Key_Right);
        close(s.first.value, 25); compare(commitSpy.count, 1);
        keyClick(Qt.Key_Tab); verify(s.second.handle.activeFocus);
        keyClick(Qt.Key_Left); close(s.second.value, 55); compare(commitSpy.count, 2);
        const h = s.first.handle;
        mouseDrag(s, h.x + h.width / 2, h.y + h.height / 2, 30, 0);
        verify(s.first.value > 25); verify(s.second.value - s.first.value >= 10);
        s.setValues(20, 60);
        const touch = touchEvent(s);
        const x = (s.first.handle.x + s.second.handle.x + h.width) / 2, y = h.y + h.height / 2;
        touch.press(0, s, x, y).commit(); touch.move(0, s, x + 60, y).commit(); touch.release(0, s, x + 60, y).commit();
        verify(s.first.value > 20); close(s.second.value - s.first.value, 40);
        const first = s.first.value; s.enabled = false;
        mouseDrag(s, x, y, 60, 0); keyClick(Qt.Key_Right); close(s.first.value, first);
    }
    function test_rangeReset() {
        const s = createTemporaryObject(rangeFactory, test, {defaultFirstValue: 10, defaultSecondValue: 90, maximumSpan: 50});
        watch(s, true); s.reset(); close(s.first.value, 10); close(s.second.value, 60);
        compare(movedSpy.count, 1); compare(commitSpy.count, 1);
        s.reset(); compare(commitSpy.count, 1);
    }
    function test_fieldEditorsAndInvalidText() {
        const f = createTemporaryObject(fieldFactory, test);
        verify(waitForRendering(f)); compare(f.numberBox.value, -12);
        f.numberBox.forceActiveFocus(); f.numberBox.selectAll();
        keyClick(Qt.Key_Minus); keyClick(Qt.Key_2); keyClick(Qt.Key_4); keyClick(Qt.Key_Period); keyClick(Qt.Key_5); keyClick(Qt.Key_Return);
        close(f.value, -24.5);
        f.slider.forceActiveFocus(); keyClick(Qt.Key_Right); close(f.value, -24); close(f.numberBox.value, -24);
        f.value = 6; close(f.numberBox.value, 6);
        f.numberBox.text = "invalid"; verify(!f.numberBox.commitText()); verify(f.numberBox.invalid); close(f.value, 6);
        f.reset(); close(f.value, -12); close(f.numberBox.value, -12); verify(!f.numberBox.invalid);
        f.numberBox.commitValue(100); close(f.value, 12);
        f.width = 230; verify(f.compact); tryVerify(() => f.implicitHeight > f.slider.implicitHeight);
        const r = f.numberBox.mapToItem(f, 0, 0); verify(r.x >= 0); verify(r.x + f.numberBox.width <= f.width + 1);
    }
    function test_fieldBindingSurvivesEdits() {
        const root = createTemporaryObject(boundFactory, test);
        const f = root.editor;
        f.numberBox.commitValue(-30); close(root.backend, -30); close(f.value, -30);
        root.backend = -20; close(f.value, -20); close(f.numberBox.value, -20);
        f.reset(); close(root.backend, -12);
        root.backend = 4; close(f.value, 4); close(f.numberBox.value, 4);
    }
}
