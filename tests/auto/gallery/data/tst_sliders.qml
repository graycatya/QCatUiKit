// SPDX-License-Identifier: Apache-2.0
import QtQuick
import QtTest
import QCatUiKit
import QCatUiKit.Gallery

TestCase {
    id: test
    name: "GallerySliders"
    when: windowShown
    property var gallery
    property var example
    Component { id: factory; Main { width: 1200; height: 840 } }
    function initTestCase() { failOnWarning(/.*/); GalleryLanguage.language = "en"; gallery = factory.createObject(test); }
    function init() {
        failOnWarning(/.*/);
        gallery.requestActivate();
        tryCompare(gallery, "active", true);
        tryVerify(() => gallery.activeFocusItem !== null);
    }
    function cleanup() {
        if (example) example.destroy(); example = null;
        gallery.width = 1200; Theme.density = ThemePalette.Desktop;
    }
    function cleanupTestCase() { gallery.close(); gallery.destroy(); }
    function find(name) { const item = findChild(gallery.contentItem, name); verify(item !== null, name); return item; }
    function test_variants_data() {
        const rows = [];
        for (const id of ["slider", "rangeslider", "sliderfield"])
            for (let variant = 0; variant < (id === "slider" ? 4 : id === "rangeslider" ? 3 : 1); ++variant)
                rows.push({tag: id + variant, id: id, variant: variant});
        return rows;
    }
    function test_variants(data) {
        gallery.showComponent(data.id);
        const picker = find("sliderVariantPicker");
        picker.currentIndex = data.variant; picker.activated(data.variant);
        wait(1);
        const code = find("codeSample").text;
        example = Qt.createQmlObject(code, test, "copied_slider.qml");
        verify(example !== null);
        if (data.id === "slider") {
            compare(example.orientation, data.variant === 3 ? Qt.Vertical : Qt.Horizontal);
            compare(example.from, data.variant === 2 ? -100 : 0);
            compare(example.stepSize, data.variant === 1 ? 20 : 0);
        } else if (data.id === "rangeslider") {
            compare(example.fixedSpan, data.variant === 2 ? 30 : -1);
            compare(example.minimumSpan, data.variant === 1 ? 10 : 0);
            verify(example.rangeDragEnabled);
        } else { compare(example.decimals, 1); verify(example.resetVisible); }
        const before = code;
        if (data.id !== "sliderfield") {
            picker.currentIndex = (data.variant + 1) % picker.count; picker.activated(picker.currentIndex);
            verify(find("codeSample").text !== before);
        }
    }
    function test_chartWholeRangeDrag() {
        gallery.showChartDemo(0);
        const slider = find("chartXRangeSlider"), chart = find("demoChart");
        tryVerify(() => slider.to > 1);
        const domain = slider.to - slider.from;
        slider.setValues(slider.from + domain * .2, slider.from + domain * .6); slider.moved();
        const flick = find("galleryContentScroll").contentItem;
        flick.contentY = Math.max(0, Math.min(flick.contentHeight - flick.height,
            flick.contentY + slider.mapToItem(flick, 0, 0).y - 100));
        wait(20);
        const count = chart.series[0].count, low = slider.first.value, span = slider.second.value - low;
        const x = (slider.first.handle.x + slider.second.handle.x + slider.first.handle.width) / 2;
        mouseDrag(slider, x, slider.first.handle.y + slider.first.handle.height / 2, slider.width * .1, 0);
        verify(slider.first.value > low);
        fuzzyCompare(slider.second.value - slider.first.value, span, .00001);
        fuzzyCompare(chart.xAxis.minimum, slider.first.value, .00001);
        fuzzyCompare(chart.xAxis.maximum, slider.second.value, .00001);
        verify(chart.yAxis.automatic); compare(chart.series[0].count, count);
        // A deep external zoom remains representable in the overview control.
        chart.xAxis.setRange(10, 10.00001); wait(10);
        fuzzyCompare(slider.first.value, 10, 1e-10); fuzzyCompare(slider.second.value, 10.00001, 1e-10);
    }
    function test_chartLockedSpanAndExternalZoom() {
        gallery.showChartDemo(0);
        const slider = find("chartXRangeSlider"), chart = find("demoChart");
        tryVerify(() => slider.to > 1);
        const size = slider.to - slider.from;
        chart.xAxis.setRange(slider.from + size * .2, slider.from + size * .6);
        wait(20);
        find("chartXRangeSliderLockSpan").click();
        const span = slider.second.value - slider.first.value, low = slider.first.value;
        slider.first.handle.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Right);
        verify(slider.first.value > low);
        fuzzyCompare(slider.second.value - slider.first.value, span, 1e-6);
        fuzzyCompare(chart.xAxis.maximum - chart.xAxis.minimum, span, 1e-6);
        chart.xAxis.setRange(10, 10.25); wait(20);
        fuzzyCompare(slider.fixedSpan, .25, 1e-8);
        fuzzyCompare(slider.first.value, 10, 1e-8);
        fuzzyCompare(slider.second.value, 10.25, 1e-8);
        find("chartXRangeSliderLockSpan").click();
        slider.first.handle.forceActiveFocus(Qt.TabFocusReason); keyClick(Qt.Key_Right);
        verify(slider.second.value - slider.first.value < .25);
    }
    function test_chartFieldResetAndBinding_data() {
        return [
            {tag: "bar-percent", mode: 2, field: "chartBarWidth", value: 55, defaults: 80, owner: "chartPreview", property: "barWidth", next: .72, scale: 100, code: "barGroupWidth: 0.55"},
            {tag: "bar-spacing", mode: 2, field: "chartBarSpacing", value: 25, defaults: 0, owner: "chartPreview", property: "barGap", next: .15, scale: 100, code: "barSpacing: 0.25"},
            {tag: "area-percent", mode: 1, field: "chartAreaOpacity", value: 45, defaults: 65 / 255 * 100, owner: "chartAreaOptions", property: "fillAlpha", next: .7, scale: 100, code: "fillOpacity: 0.45"}
        ];
    }
    function test_chartFieldResetAndBinding(data) {
        gallery.showChartDemo(data.mode);
        verify(waitForRendering(gallery.contentItem));
        const field = find(data.field), owner = find(data.owner);
        field.numberBox.forceActiveFocus(); field.numberBox.selectAll();
        for (const c of String(data.value)) keyClick(c);
        keyClick(Qt.Key_Return);
        fuzzyCompare(owner[data.property], data.value / data.scale, 1e-8);
        verify(find("codeSample").text.includes(data.code));
        field.reset();
        fuzzyCompare(owner[data.property], data.defaults / data.scale, 1e-8);
        owner[data.property] = data.next;
        fuzzyCompare(field.value, data.next * data.scale, 1e-8);
        fuzzyCompare(field.numberBox.value, field.value, 1e-8);
        gallery.width = 420; Theme.density = ThemePalette.Mobile;
        tryVerify(() => field.compact, 5000, "field width=" + field.width + ", gallery=" + gallery.width);
        const input = field.numberBox.mapToItem(field, 0, 0);
        verify(input.y >= field.slider.height);
        verify(input.x + field.numberBox.width <= field.width + 1);
    }
    function test_progressReset_data() { return [{tag: "progress"}, {tag: "circular"}]; }
    function test_progressReset(data) {
        gallery.showComponent(data.tag);
        const slider = find("progressSlider");
        slider.forceActiveFocus(); keyClick(Qt.Key_Right);
        compare(slider.value, 65);
        find("progressReset").click(); compare(slider.value, 64);
        example = Qt.createQmlObject(find("codeSample").text, test, "copied_progress.qml");
        verify(example !== null);
    }
}
