// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import QCatUiKit

ColumnLayout {
    id: range
    required property CatChartAxis axis
    required property string axisName
    required property string sliderName
    property real domainMinimum: 0
    property real domainMaximum: 1
    property bool initialized: false
    property real visibleSpan: 1
    function clear() {
        initialized = false;
        domainMinimum = 0; domainMaximum = 1;
        slider.setValues(0, 1);
    }
    function synchronize(minimum: real, maximum: real): void {
        if (!isFinite(minimum) || !isFinite(maximum) || maximum <= minimum) return;
        visibleSpan = maximum - minimum;
        // Automatic view follows the data. Manual navigation retains the
        // overview domain, expanding only when the view moves beyond it.
        domainMinimum = !initialized || axis.automatic ? minimum : Math.min(domainMinimum, minimum);
        domainMaximum = !initialized || axis.automatic ? maximum : Math.max(domainMaximum, maximum);
        initialized = true;
        slider.setValues(minimum, maximum);
    }
    function apply(): void {
        axis.setRange(slider.first.value, slider.second.value);
    }
    function formatted(value: real): string {
        return Number(value.toPrecision(7)).toLocaleString(Qt.locale(GalleryLanguage.localeName), "g", 7);
    }
    RowLayout {
        Layout.fillWidth: true
        CatLabel {
            Layout.fillWidth: true
            // Changing range digits must not resize the two slider columns or
            // add a line while the user is moving the chart.
            Layout.minimumWidth: 0
            Layout.preferredWidth: 0
            elide: Text.ElideRight
            text: range.enabled ? qsTr("%1: %2 – %3").arg(range.axisName)
                .arg(range.formatted(slider.first.value)).arg(range.formatted(slider.second.value))
                : range.axisName + ": —"
        }
        CatCheckBox {
            id: lockSpan
            objectName: range.sliderName + "LockSpan"
            text: qsTr("Lock span")
            Accessible.name: range.axisName + " — " + text
        }
    }
    CatRangeSlider {
        id: slider
        objectName: range.sliderName
        Layout.fillWidth: true
        from: range.domainMinimum; to: range.domainMaximum
        stepSize: (to - from) / 1000
        minimumSpan: Math.min(range.visibleSpan, Math.max((to - from) * 1e-6,
            Math.max(Math.abs(from), Math.abs(to), 1) * Number.EPSILON * 16))
        rangeDragEnabled: true
        fixedSpan: lockSpan.checked ? range.visibleSpan : -1
        tickValues: [from, (from + to) / 2, to]
        tickLabelsVisible: true
        valueToolTipVisible: true
        valueFormatter: range.formatted
        Accessible.name: range.axisName
        onMoved: range.apply()
    }
}
