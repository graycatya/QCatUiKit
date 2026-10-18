// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import QCatUiKit

GridLayout {
    id: control
    objectName: "chartAreaOptions"
    readonly property CatChartSeries defaultStyle: CatChartSeries {}
    property bool baselineEnabled: false
    property int baselineMode: 0
    property int baselineValue: 0
    property int style: 0
    property int fillPalette: 0
    property int outline: 0
    property bool fade: false
    property bool multiStops: false
    property int textureMode: 0
    property int tileSize: 32
    readonly property url texture: Qt.resolvedUrl("assets/chart-weave.png")
    readonly property var gradientStops: multiStops ? [{position: 0, color: startColor},
        {position: 0.45, color: "#ffd666"}, {position: 1, color: endColor}] : []
    property real defaultAlpha: 65 / 255
    property real fillAlpha: defaultAlpha
    readonly property color startColor: fillPalette === 2 ? "#ff9845" : "#22b8cf"
    readonly property color endColor: fade ? Qt.rgba(startColor.r, startColor.g, startColor.b, 0)
        : fillPalette === 2 ? "#c23c82" : "#5265e8"
    readonly property color outlineColor: outline === 3 ? "transparent" : outline === 2 ? "white" : Theme.tokens.text
    columns: width >= 620 ? 3 : 1
    columnSpacing: 12; rowSpacing: 8
    function reset() {
        baselineMode = 0; baselineValue = 0; style = 0; fillPalette = 0; outline = 0;
        fade = false; fillAlpha = defaultAlpha; multiStops = false; textureMode = 0; tileSize = 32;
    }
    ColumnLayout {
        Layout.fillWidth: true
        visible: control.baselineEnabled
        CatLabel { text: qsTr("Area baseline"); Layout.fillWidth: true; wrapMode: Text.Wrap }
        CatComboBox {
            objectName: "chartAreaBaselineMode"
            Layout.fillWidth: true
            Accessible.name: qsTr("Area baseline")
            textRole: "text"; valueRole: "value"
            model: [{text: qsTr("Zero"), value: 0}, {text: qsTr("Custom value"), value: 1},
                    {text: qsTr("Plot bottom"), value: 2}]
            currentValue: control.baselineMode
            onActivated: index => control.baselineMode = index
        }
    }
    ColumnLayout {
        Layout.fillWidth: true
        visible: control.baselineEnabled
        CatLabel { text: qsTr("Baseline value"); Layout.fillWidth: true; wrapMode: Text.Wrap }
        CatSliderField {
            objectName: "chartAreaBaselineValue"
            Layout.fillWidth: true
            Accessible.name: qsTr("Baseline value")
            enabled: control.baselineMode === 1
            from: -100; to: 100;  value: control.baselineValue
            onValueEdited: value => control.baselineValue = value
            decimals: 0; stepSize: 1; fillOrigin: 0; defaultValue: 0; defaultIndicatorVisible: true; tickInterval: 50
            valueToolTipVisible: true; resetVisible: true; inputWidth: 90
            locale: Qt.locale(GalleryLanguage.localeName)
        }
    }
    ColumnLayout {
        Layout.fillWidth: true
        CatLabel { text: qsTr("Area fill"); Layout.fillWidth: true; wrapMode: Text.Wrap }
        CatComboBox {
            objectName: "chartAreaFillStyle"
            Layout.fillWidth: true
            Accessible.name: qsTr("Area fill")
            textRole: "text"; valueRole: "value"
            model: [{text: qsTr("Solid"), value: 0}, {text: qsTr("Vertical gradient"), value: 1},
                    {text: qsTr("Horizontal gradient"), value: 2}, {text: qsTr("Texture"), value: 3}]
            currentValue: control.style
            onActivated: index => control.style = index
        }
    }
    ColumnLayout {
        Layout.fillWidth: true
        CatLabel { text: qsTr("Fill colors"); Layout.fillWidth: true; wrapMode: Text.Wrap }
        CatComboBox {
            objectName: "chartAreaPalette"
            enabled: control.style !== 3
            Layout.fillWidth: true
            Accessible.name: qsTr("Fill colors")
            textRole: "text"; valueRole: "value"
            model: [{text: qsTr("Series color"), value: 0}, {text: qsTr("Ocean"), value: 1},
                    {text: qsTr("Sunset"), value: 2}]
            currentValue: control.fillPalette
            onActivated: index => control.fillPalette = index
        }
    }
    ColumnLayout {
        Layout.fillWidth: true
        CatLabel { text: qsTr("Area boundary"); Layout.fillWidth: true; wrapMode: Text.Wrap }
        CatComboBox {
            objectName: "chartAreaOutline"
            Layout.fillWidth: true
            Accessible.name: qsTr("Area boundary")
            textRole: "text"; valueRole: "value"
            model: [{text: qsTr("Series color"), value: 0}, {text: qsTr("Theme text"), value: 1},
                    {text: qsTr("White"), value: 2}, {text: qsTr("Hidden"), value: 3}]
            currentValue: control.outline
            onActivated: index => control.outline = index
        }
    }
    ColumnLayout {
        Layout.fillWidth: true
        CatLabel { text: qsTr("Fill opacity: %1%").arg(Math.round(control.fillAlpha * 100)); Layout.fillWidth: true; wrapMode: Text.Wrap }
        CatSliderField {
            objectName: "chartAreaOpacity"
            Layout.fillWidth: true
            Accessible.name: qsTr("Fill opacity")
            from: 0; to: 100; stepSize: 1; value: control.fillAlpha * 100
            onValueEdited: value => control.fillAlpha = value / 100
            decimals: 1; suffix: "%"; defaultValue: control.defaultAlpha * 100; tickInterval: 25
            valueToolTipVisible: true; resetVisible: true; inputWidth: 90
            locale: Qt.locale(GalleryLanguage.localeName)
        }
    }
    CatCheckBox {
        objectName: "chartAreaMultiStops"
        visible: control.style === 1 || control.style === 2
        text: qsTr("Three color stops")
        checked: control.multiStops
        onToggled: control.multiStops = checked
    }
    ColumnLayout {
        Layout.fillWidth: true
        visible: control.style === 3
        CatLabel { text: qsTr("Texture placement") }
        CatComboBox {
            objectName: "chartAreaTextureMode"
            Layout.fillWidth: true
            Accessible.name: qsTr("Texture placement")
            textRole: "text"; valueRole: "value"
            model: [{text: qsTr("Stretch to plot"), value: 0}, {text: qsTr("Tile"), value: 1}]
            currentValue: control.textureMode
            onActivated: index => control.textureMode = index
        }
    }
    ColumnLayout {
        Layout.fillWidth: true
        visible: control.style === 3
        CatLabel { text: qsTr("Tile size (px)") }
        CatSpinBox {
            objectName: "chartAreaTileSize"
            Layout.fillWidth: true
            Accessible.name: qsTr("Tile size (px)")
            enabled: control.textureMode === 1
            from: 8; to: 256; stepSize: 8; value: control.tileSize
            onValueModified: control.tileSize = value
        }
    }
    CatCheckBox {
        objectName: "chartAreaFade"
        Layout.columnSpan: control.columns
        text: qsTr("Fade gradient to transparent")
        visible: control.style === 1 || control.style === 2
        checked: control.fade
        onToggled: control.fade = checked
    }
    CatLabel {
        Layout.fillWidth: true
        Layout.columnSpan: control.columns
        text: control.style === 3 ? qsTr("Texture colors are preserved. Opacity controls the fill; invalid images fall back to a solid color.")
            : qsTr("Gradients span the plot from top to bottom or left to right. Fill opacity leaves the boundary and markers unchanged.")
        wrapMode: Text.Wrap
        color: Theme.tokens.textSecondary
    }
}
