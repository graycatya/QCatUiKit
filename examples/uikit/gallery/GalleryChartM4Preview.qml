// SPDX-License-Identifier: Apache-2.0
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import QCatUiKit
import "GalleryChartExamples.js" as Examples

ColumnLayout {
    id: root
    objectName: "chartM4Preview"
    required property int mode
    property real parameter: mode === 25 ? 1.5 : mode === 27 ? 7 : mode === 29 ? .25 : 1
    property bool optionA: mode === 24 || mode === 26 || mode === 27
    property bool optionB: mode === 24 || mode === 25 || mode === 28
    property bool optionC: mode === 24 || mode === 27 || mode === 29
    property bool ready: false
    readonly property alias chart: plot
    readonly property var adapter: mode === 24 ? ohlc : mode === 25 ? box : mode === 28 ? bubbles : grid
    readonly property string exampleCode: Examples.code(mode, {parameter:parameter, a:optionA, b:optionB, c:optionC})
    readonly property string explanation: mode === 24 ? qsTr("Inspect original OHLC and volume. Compress time to remove trading gaps, or retain the original timestamps.")
        : mode === 25 ? qsTr("Quartiles use linear interpolation. Whiskers use the IQR factor; outliers retain their original observation index.")
        : mode === 26 ? qsTr("Dense textures and sparse cells share one color scale. Null cells are gaps. Updates and scrolling preserve original values.")
        : mode === 27 ? qsTr("Contours use four linear triangles around each cell center. Missing corners omit the cell; bands fill only between the selected levels.")
        : mode === 28 ? qsTr("Radius mode uses logical pixels; area mode makes area proportional to size. Color uses an independent value and range.")
        : qsTr("Compare up to 32 dimensions. Per-dimension ranges normalize the view while inspection retains original values.")
    spacing: 12
    function updateData() {
        if (!ready) return;
        if (mode === 24) {
            ohlc.setData([{time:1704067200000,open:10,high:15,low:8,close:12,volume:80,label:qsTr("Mon")},
                {time:1704153600000,open:12,high:14,low:7,close:9,volume:100,label:qsTr("Tue")},
                {time:1704412800000,open:9,high:16,low:8,close:9+parameter*2,volume:60,label:qsTr("Fri")},
                {time:1704672000000,open:12,high:18,low:10,close:16,volume:120,label:qsTr("Mon 2")}]);
        } else if (mode === 25) {
            box.setData(optionA ? [{label:"A",minimum:1,q1:2,median:3,q3:4,maximum:6,outliers:[9]},
                {label:"B",minimum:2,q1:3,median:4,q3:5,maximum:7,outliers:[10]}]
                : [{label:"A",values:[1,2,2,3,3,4,6,9]},{label:"B",values:[2,3,3,4,4,5,7,10]}]);
        } else if (mode === 26 || mode === 27) {
            const values=[];
            for(let r=0;r<24;++r) for(let c=0;c<32;++c)
                values.push(optionB && r>=8 && r<=12 && c>=13 && c<=17 ? null : parameterForGrid()*(Math.sin(c*.25)+Math.cos(r*.3)));
            grid.setGrid(24,32,values);
            if (mode === 26 && optionC) grid.colorScale.setRange(-2,2);
            else grid.colorScale.automatic=true;
        } else if (mode === 28) {
            bubbles.setData([{x:0,y:2,size:8,colorValue:-1},{x:1,y:4,size:16,colorValue:0},{x:2,y:3,size:24,colorValue:1},{x:3,y:5,size:32,colorValue:2}]);
            if(optionC)bubbles.colorScale.setRange(0,1);else bubbles.colorScale.automatic=true;
        } else {
            first.setData([{x:0,y:70},{x:1,y:6},{x:2,y:50},{x:3,y:7},{x:4,y:65}]);
            second.setData([{x:0,y:55},{x:1,y:8},{x:2,y:70},{x:3,y:4},{x:4,y:80}]);
        }
    }
    function parameterForGrid() { return mode === 26 ? parameter : 1; }
    Component.onCompleted: { ready=true; updateData(); }
    onParameterChanged: updateData()
    onOptionAChanged: updateData()
    onOptionBChanged: updateData()
    onOptionCChanged: updateData()
    CatLabel {
        text: root.mode === 24 ? qsTr("Close adjustment") : root.mode === 25 ? qsTr("Whisker IQR factor") : root.mode === 26 ? qsTr("Amplitude")
            : root.mode === 27 ? qsTr("Contour levels") : root.mode === 28 ? qsTr("Size multiplier") : qsTr("Fill opacity")
    }
    CatSliderField {
        objectName: "m4Parameter"; Layout.fillWidth: true
        from: root.mode === 27 ? 3 : root.mode === 28 ? .1 : 0
        to: root.mode === 27 ? 12 : root.mode === 29 ? 1 : 3
        stepSize: root.mode === 27 ? 1 : .05; decimals: root.mode === 27 ? 0 : 2
        value: root.parameter; defaultValue: root.mode === 25 ? 1.5 : root.mode === 27 ? 7 : root.mode === 29 ? .25 : 1
        resetVisible: true; valueToolTipVisible: true
        Accessible.name: root.mode === 27 ? qsTr("Contour levels") : qsTr("Chart parameter")
        onValueEdited: value => root.parameter=value
    }
    Flow {
        Layout.fillWidth: true; spacing: 12
        CatCheckBox {
            objectName: "m4OptionA"; checked: root.optionA; onToggled: root.optionA=checked
            text: root.mode === 24 ? qsTr("Candlesticks") : root.mode === 25 ? qsTr("Precomputed summaries") : root.mode === 26 ? qsTr("Dense texture")
                : root.mode === 27 ? qsTr("Filled bands") : root.mode === 28 ? qsTr("Area encoding") : qsTr("Dashed lines")
        }
        CatCheckBox {
            objectName: "m4OptionB"; checked: root.optionB; onToggled: root.optionB=checked
            text: root.mode === 24 ? qsTr("Compress trading gaps") : root.mode === 25 ? qsTr("Show outliers") : root.mode === 26 || root.mode === 27 ? qsTr("Missing cells")
                : root.mode === 28 ? qsTr("Color mapping") : qsTr("Normalize dimensions")
        }
        CatCheckBox {
            objectName: "m4OptionC"; checked: root.optionC; onToggled: root.optionC=checked; visible: root.mode !== 25
            text: root.mode === 24 ? qsTr("Volume") : root.mode === 27 ? qsTr("Level labels") : root.mode === 29 ? qsTr("Point markers") : qsTr("Fixed color range")
        }
    }
    Flow {
        Layout.fillWidth: true; spacing: 8; visible: root.mode === 26
        CatButton { objectName:"m4CellUpdate"; text:qsTr("Update one cell"); onClicked:grid.setValue(4,4,grid.values[132] > 0 ? -2 : 2) }
        CatButton { objectName:"m4AppendRow"; text:qsTr("Append scrolling row"); onClicked:{const values=[];for(let i=0;i<32;++i)values.push(Math.sin(i*.25));grid.appendRow(values);} }
    }
    CatLabel { Layout.fillWidth:true; wrapMode:Text.Wrap; text:root.explanation; color:Theme.tokens.textSecondary }
    CatLabel { Layout.fillWidth:true; wrapMode:Text.Wrap; text:root.mode===27 ? contour.error : root.mode===29 ? "" : root.adapter.error; visible:text.length>0; color:Theme.tokens.danger }
    CatChartOhlc {
        id:ohlc; candlesticks:root.optionA; compressedTime:root.optionB
        risingColor:Theme.tokens.success; fallingColor:Theme.tokens.danger
        series.name:qsTr("Price"); volumeSeries.name:qsTr("Volume")
        volumeSeries.visible:root.optionC; volumeSeries.color:Theme.tokens.primary
        volumeSeries.yAxis: CatChartAxis { side:CatChartAxis.FarSide; visible:root.optionC; title:qsTr("Volume"); Component.onCompleted:setRange(0,500) }
    }
    CatChartBoxPlot {
        id:box; whiskerFactor:root.mode===25 ? root.parameter : 1.5
        series.name:qsTr("Distribution"); series.color:Theme.tokens.primary; series.borderColor:Theme.tokens.text
        outlierSeries.name:qsTr("Outliers"); outlierSeries.color:Theme.tokens.danger; outlierSeries.visible:root.optionB
    }
    CatChartGrid { id:grid; dense:root.optionA; series.name:qsTr("Intensity") }
    CatChartContour {
        id:contour; source:root.mode===27 ? grid : null; filled:root.optionA; labelsVisible:root.optionC
        levels: { const values=[]; const count=root.mode===27 ? Math.round(root.parameter) : 7;for(let i=0;i<count;++i)values.push(-2+4*i/(count-1));return values; }
        series.name:qsTr("Contours")
    }
    CatChartBubble { id:bubbles; sizeMode:root.optionA ? CatChartBubble.Area : CatChartBubble.Radius; sizeScale:(root.mode===28 ? root.parameter : 1)*(root.optionA ? 64 : 1); colorMapping:root.optionB; series.name:qsTr("Observations") }
    CatChartSeries { id:first; name:qsTr("Series A"); color:Theme.tokens.primary; fillOpacity:root.mode===29 ? root.parameter : .25; strokeStyle:root.optionA ? CatChartSeries.DashLine : CatChartSeries.SolidLine; markersVisible:root.optionC }
    CatChartSeries { id:second; name:qsTr("Series B"); color:Theme.tokens.success; fillOpacity:first.fillOpacity; strokeStyle:first.strokeStyle; markersVisible:first.markersVisible }
    CatChart {
        id:plot; objectName:"m4Chart"; Layout.fillWidth:true; Layout.preferredHeight:480
        chartType: root.mode===29 ? CatChartSeries.Radar : CatChartSeries.Line
        series: root.mode===24 ? [ohlc.volumeSeries,ohlc.series] : root.mode===25 ? [box.series,box.outlierSeries] : root.mode===26 ? [grid.series] : root.mode===27 ? [contour.series] : root.mode===28 ? [bubbles.series] : [first,second]
        xAxis.scale:root.mode===25 || (root.mode===24 && root.optionB) ? CatChartAxis.Category : root.mode===24 ? CatChartAxis.DateTime : CatChartAxis.Linear
        xAxis.categories:root.mode===24 ? ohlc.categories : root.mode===25 ? box.categories : [qsTr("Speed"),qsTr("Quality"),qsTr("Capacity"),qsTr("Efficiency"),qsTr("Reliability")]
        radarRanges:root.mode===29 && root.optionB ? [{minimum:0,maximum:100},{minimum:0,maximum:10},{minimum:0,maximum:100},{minimum:0,maximum:10},{minimum:0,maximum:100}] : []
        Component.onCompleted: if (root.mode === 28) { xAxis.setRange(-1,4); yAxis.setRange(1,6); }
    }
}
