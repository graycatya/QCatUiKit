// SPDX-License-Identifier: Apache-2.0
.pragma library

// Small, standalone examples. These describe the selected variant; they are
// deliberately independent of GalleryChartData and the live/stress data buffer.
function number(value, fallback) {
    return value === undefined ? fallback : Math.round(value * 10000) / 10000;
}
function curve(options) {
    const names = ["LinearCurve", "StepLeft", "StepRight", "StepCenter", "MonotoneSpline", "CatmullRomSpline", "NaturalSpline"];
    let text = "        curveMode: CatChartSeries." + names[options.curve || 0] + "\n";
    if (options.curve === 5) text += "        splineTension: " + number(options.tension, 0) + "\n";
    if (options.dashed) text += "        strokeStyle: CatChartSeries.DashLine\n        dashPattern: [16, 8]\n";
    if (options.markers) text += "        markersVisible: true\n"
        + "        markerShape: CatChartSeries." + ["Circle", "Square", "Triangle", "Diamond"][options.marker || 0] + "\n"
        + "        markerSize: " + number(options.markerSize, 8) + "\n";
    return text;
}
function series(name, points, properties, range) {
    return "    CatChartSeries {\n        name: qsTr(" + JSON.stringify(name) + ")\n"
        + (properties || "") + "        Component.onCompleted: " + (range ? "setRangeData" : "setData")
        + "(" + points + ")\n    }\n";
}
function chart(type, properties, seriesCode) {
    return "CatChart {\n    width: 720; height: 420\n    chartType: CatChartSeries." + type + "\n"
        + (properties || "") + seriesCode + "}";
}
function code(mode, options) {
    const o = options || {};
    if (mode >= 24) return extended(mode, o);
    if (mode === 23) return waterfall(o);
    if (mode === 22 || mode === 14) return fitted(o);
    if (mode === 21) return histogram(o);
    if (mode === 20) return stacked(o);
    if (mode === 19) return threeD(o);
    if (mode === 18) return axes(o);
    if (mode === 17) return radial(o);
    const points = "[{x:0,y:3}, {x:1,y:6}, {x:2,y:4}, {x:3,y:8}]";
    const second = "[{x:0,y:2}, {x:1,y:4}, {x:2,y:5}, {x:3,y:3}]";
    const categories = "    xAxis.scale: CatChartAxis.Category\n    xAxis.categories: [\"A\", \"B\", \"C\", \"D\"]\n";
    if (mode === 0 || mode === 1 || mode === 16) {
        const data = mode === 16 ? "[{x:0,y:3}, {x:1,y:6}, {x:2,y:4,breakBefore:true}, {x:3,y:8}]" : points;
        const area = mode === 1 ? "        fillOpacity: " + number(o.fillAlpha, 65 / 255) + "\n"
            + "        areaBaselineMode: CatChartSeries." + ["ZeroBaseline", "CustomBaseline", "PlotBottomBaseline"][o.baselineMode || 0] + "\n"
            + "        areaBaseline: " + number(o.baselineValue, 0) + "\n" : "";
        const example = chart(mode === 1 ? "Area" : "Line", "", series("Samples", data, curve(o) + area));
        return mode === 1 ? example : withAxisSliders(example);
    }
    if (mode === 15) {
        const data = "[{x:0,lower:1,upper:3}, {x:1,lower:2,upper:6},\n            {x:2,lower:1,upper:4}, {x:3,lower:3,upper:7}]";
        return chart("RangeArea", "", series("Range", data, curve(o) + "        fillOpacity: " + number(o.fillAlpha, 0.3) + "\n", true));
    }
    if (mode === 2 || mode === 3 || mode === 13) {
        const variant = o.barVariant === undefined ? (mode === 2 ? 0 : 1) : o.barVariant;
        const types = ["Bar", "StackedBar", "PercentBar", "HorizontalBar", "HorizontalStackedBar", "HorizontalPercentBar"];
        const axis = variant >= 3 ? categories.replace(/xAxis/g, "yAxis") : categories;
        const signed = "[{x:0,y:3}, {x:1,y:-6}, {x:2,y:4}, {x:3,y:-2}]";
        const signedSecond = "[{x:0,y:2}, {x:1,y:-2}, {x:2,y:-3}, {x:3,y:5}]";
        const labels = o.barLabels ? "        barLabelsVisible: true\n" : "";
        const bars = "    barGroupWidth: " + number(o.barWidth, 0.8) + "\n    barSpacing: " + number(o.barGap, 0) + "\n";
        return chart(types[variant], axis + bars, series("Series A", mode === 13 ? signed : points, labels)
                     + series("Series B", mode === 13 ? signedSecond : second, labels));
    }
    if (mode === 4 || mode === 5) {
        const data = "[{x:0,y:3,value:8}, {x:1,y:6,value:14}, {x:2,y:4,value:20}, {x:3,y:8,value:10}]";
        const names = ["Circle", "Square", "Triangle", "Diamond"];
        const marker = "        markerShape: CatChartSeries." + (names[o.marker || 0] || "Circle") + "\n"
            + "        markerSize: " + number(o.markerSize, 8) + "\n";
        return chart(mode === 4 ? "Scatter" : "Bubble", "", series("Samples", mode === 5 ? data : points, marker));
    }
    if (mode === 6 || mode === 7) {
        const data = "[{x:0,y:30,label:\"A\"}, {x:1,y:45,label:\"B\"}, {x:2,y:25,label:\"C\"}]";
        return chart(mode === 6 ? "Pie" : "Doughnut", "    sliceLabelsVisible: true\n", series("Shares", data));
    }
    if (mode === 8) return chart("Radar", categories, series("Series A", points) + series("Series B", second));
    if (mode === 9) return chart("Heatmap", "    cellWidth: 1; cellHeight: 1\n", series("Intensity",
        "[{x:0,y:0,value:1}, {x:1,y:0,value:4}, {x:2,y:0,value:2},\n            {x:0,y:1,value:3}, {x:1,y:1,value:2}, {x:2,y:1,value:5}]"));
    if (mode === 10) return chart("Line", categories, series("Bars", points, "        type: CatChartSeries.Bar\n")
        + series("Line", second, "        type: CatChartSeries.Line\n" + curve(o)));
    if (mode === 11) return chart("Line", "    xAxis.scale: CatChartAxis.DateTime\n", series("Measurements",
        "[{x:1710000000000,y:3}, {x:1710000060000,y:6}, {x:1710000120000,y:4}]", curve(o)));
    if (mode === 12) return chart("Line", "    yAxis.scale: CatChartAxis.Logarithmic\n    yAxis.minorTickCount: 3\n",
        series("Measurements", "[{x:0,y:1}, {x:1,y:10}, {x:2,y:100}, {x:3,y:1000}]", curve(o)));
    return "";
}
function fitted(o) {
    return `Item {
    width: 720; height: 420
    property alias chart: plot
    CatChartFit {
        id: fit
        residualMultiplier: ${number(o.multiplier, 2)}
        series.name: qsTr("Linear fit"); series.color: Theme.tokens.danger
        band.name: qsTr("Residual band"); band.color: Theme.tokens.danger
        band.visible: ${o.bandVisible !== false}
${curve(o).replace(/(^\s+)([A-Za-z]\w*):/gm, "$1series.$2:")}        source: CatChartSeries {
            id: observations; type: CatChartSeries.Scatter
            name: qsTr("Observations"); color: Theme.tokens.primary
            Component.onCompleted: {
                const points = [];
                for (let i = 0; i < ${o.empty ? 0 : 40}; ++i)
                    points.push({x: ${o.constantX ? 1 : "i"}, y: 2 + i * .3 + ${number(o.noise, 3)} * Math.sin(i * 1.7)});
                setData(points);
            }
        }
    }
    CatChart { id: plot; anchors.fill: parent; series: [fit.band, observations, fit.series] }
}`;
}
function waterfall(o) {
    return `Item {
    width: 720; height: 420
    property alias chart: plot
    CatWaterfall {
        id: waterfall
        initialValue: ${number(o.initial, 0)}
        resetOnSubtotal: ${!!o.resetSubtotal}
        connectorsVisible: ${o.connectors !== false}
        increaseColor: Theme.tokens.success; decreaseColor: Theme.tokens.danger
        subtotalColor: Theme.tokens.warning; totalColor: Theme.tokens.primary
        series.name: qsTr("Net change")
        series.borderColor: Theme.tokens.textSecondary
        series.barBorderColor: Theme.tokens.border
        series.barBorderWidth: 1
        series.barLabelsVisible: ${o.labels !== false}
        steps: [{value:120,label:qsTr("Sales")}, {value:${number(o.cost, -45)},label:qsTr("Costs")},
            {kind:CatWaterfall.Subtotal,label:qsTr("Subtotal")}, {value:-25,label:qsTr("Adjustment")},
            {value:30,label:qsTr("Other")}, {kind:CatWaterfall.Total,label:qsTr("Total")}]
    }
    CatChart {
        id: plot; anchors.fill: parent
        series: [waterfall.series]
        barGroupWidth: ${number(o.barWidth, .8)}
        xAxis.scale: CatChartAxis.Category
        xAxis.categories: waterfall.categories
        xAxis.tickCount: width >= 600 ? 7 : 6
    }
}`;
}
function histogram(o) {
    return "Item {\n    width: 720; height: 420\n    property alias chart: plot\n"
        + "    CatHistogram {\n        id: histogram\n        binCount: " + number(o.bins, 12) + "\n"
        + "        binEdges: " + (o.customEdges ? "[0, 3, 4, 6, 10]" : "[]") + "\n"
        + "        normalization: CatHistogram." + ["Count", "Frequency", "Density"][o.normalization || 0] + "\n"
        + "        series.name: qsTr(\"Observations\")\n        series.color: Theme.tokens.primary\n"
        + "        Component.onCompleted: {\n            const samples = [];\n"
        + "            for (let i = 0; i < 300; ++i) samples.push(5 + 2 * Math.sin(i * .17) + Math.cos(i * .71));\n"
        + (o.outliers ? "            samples.push(-2, 12);\n" : "")
        + "            setData(samples);\n        }\n    }\n"
        + "    CatChart {\n        id: plot; anchors.fill: parent\n        series: [histogram.series]\n"
        + "        barSpacing: " + number(o.barGap, .04) + "\n    }\n}";
}
function stacked(o) {
    return `Item {
    width: 720; height: 420
    property alias chart: plot
    CatChartStack {
        id: stack
        percentage: ${!!o.percentage}
        CatChartSeries {
            name: qsTr("Series A"); color: "#1677ff"; fillOpacity: ${number(o.opacity, .65)}
            Component.onCompleted: setData([{x:0,y:3}, {x:1,y:5}, {x:2,y:${o.signed ? -4 : 4}}, {x:3,y:6,breakBefore:${!!o.gap}}, {x:4,y:3}])
        }
        CatChartSeries {
            name: qsTr("Series B"); color: "#52c41a"; fillOpacity: ${number(o.opacity, .65)}
            visible: ${o.secondVisible !== false}
            Component.onCompleted: setData([{x:0,y:2}, {x:1,y:3}, {x:2,y:2}, {x:3,y:${o.signed ? -3 : 3}}, {x:4,y:4}])
        }
    }
    CatChart { id: plot; anchors.fill: parent; series: stack.series }
}`;
}
function withAxisSliders(chartCode) {
    const body = chartCode.replace("    width: 720; height: 420", "    id: plot\n    Layout.fillWidth: true; Layout.preferredHeight: 420");
    return `import QtQuick.Layouts

ColumnLayout {
    id: root
    width: 720
    property alias chart: plot
    function sync() {
        const r = plot.plotRect();
        const a = plot.mapToValue(Qt.point(r.x, r.y));
        const b = plot.mapToValue(Qt.point(r.x + r.width, r.y + r.height));
        xRange.sync(Math.min(a.x, b.x), Math.max(a.x, b.x));
        yRange.sync(Math.min(a.y, b.y), Math.max(a.y, b.y));
    }
    Component.onCompleted: Qt.callLater(sync)
    Connections {
        target: plot
        function onViewChanged() { Qt.callLater(root.sync); }
    }
    Connections {
        target: plot.series[0]
        function onDataChanged() { Qt.callLater(root.sync); }
    }
${body.split("\n").map(line => "    " + line).join("\n")}
    CatLabel { text: "X: " + xRange.first.value.toFixed(2) + " – " + xRange.second.value.toFixed(2) }
    CatCheckBox { id: lockX; text: qsTr("Lock X span") }
    AxisSlider { id: xRange; fixedSpan: lockX.checked ? visibleSpan : -1; objectName: "xRange"; axis: plot.xAxis; Accessible.name: qsTr("X axis"); Layout.fillWidth: true }
    CatLabel { text: "Y: " + yRange.first.value.toFixed(2) + " – " + yRange.second.value.toFixed(2) }
    CatCheckBox { id: lockY; text: qsTr("Lock Y span") }
    AxisSlider { id: yRange; fixedSpan: lockY.checked ? visibleSpan : -1; objectName: "yRange"; axis: plot.yAxis; Accessible.name: qsTr("Y axis"); Layout.fillWidth: true }
    CatButton { text: qsTr("Show all data"); onClicked: { plot.resetView(); Qt.callLater(root.sync); } }

    component AxisSlider: CatRangeSlider {
        required property CatChartAxis axis
        property bool initialized: false
        stepSize: (to - from) / 1000
        property real visibleSpan: 1
        minimumSpan: Math.min(visibleSpan, (to - from) * 0.000001)
        tickValues: [from, (from + to) / 2, to]
        tickLabelsVisible: true
        decimals: 2
        rangeDragEnabled: true
        valueToolTipVisible: true
        function sync(low, high) {
            visibleSpan = high - low;
            from = !initialized || axis.automatic ? low : Math.min(from, low);
            to = !initialized || axis.automatic ? high : Math.max(to, high);
            initialized = true;
            setValues(low, high);
        }
        // User input writes to the axis; programmatic sync cannot feed back.
        onMoved: axis.setRange(first.value, second.value)
    }
}`;
}
function axes(options) {
    const o = options || {};
    const independent = o.independent !== false;
    let settings = "";
    if (independent) settings = "        yAxis: CatChartAxis {\n            title: qsTr(\"Pressure\")\n"
        + "            side: CatChartAxis." + (o.farSide === false ? "NearSide" : "FarSide") + "\n"
        + "            scale: CatChartAxis." + (o.logarithmic ? "Logarithmic" : "Linear") + "\n"
        + "            reversed: " + !!o.reversed + "\n            visible: " + (o.extraVisible !== false) + "\n        }\n";
    return chart("Line", "    yAxis.title: qsTr(\"Temperature\")\n",
        series("Temperature", "[{x:0,y:20}, {x:1,y:28}, {x:2,y:24}]", curve({curve:4}))
        + series("Pressure", "[{x:0,y:800}, {x:1,y:1200}, {x:2,y:950}]", settings));
}
function radial(options) {
    const o = options || {}, variant = o.variant === undefined ? 2 : o.variant;
    let properties = "    multiRing: " + (variant >= 2) + "\n"
        + "    holeSize: " + number(o.hole, .28) + "\n    ringGap: " + number(o.gap, 8) + "\n"
        + "    pieStartAngle: " + number(o.start, 0) + "\n    pieEndAngle: " + number(number(o.start, 0) + number(o.sweep, 300), 300) + "\n"
        + "    sliceLabelsVisible: true\n    sliceLabelPosition: CatChart.SliceLabelOutside\n";
    let rings = series("Outer", "[{x:0,y:30,label:\"A\"}, {x:1,y:45,label:\"B\"}, {x:2,y:25,label:\"C\"}]");
    if (variant >= 2) rings += series("Middle", "[{x:0,y:45,label:\"A\"}, {x:1,y:20,label:\"B\"}, {x:2,y:35,label:\"C\"}]");
    if (variant === 3) rings += series("Inner", "[{x:0,y:20,label:\"A\"}, {x:1,y:30,label:\"B\"}, {x:2,y:50,label:\"C\"}]");
    return chart(variant === 0 ? "Pie" : "Doughnut", properties, rings);
}
function threeD(options) {
    const o = options || {}, type = o.type || 0;
    const types = ["Scatter", "Bars", "Curve", "Surface"];
    const shapes = ["Sphere", "Cube", "Cylinder", "CustomMesh", "RoundedCube"];
    const polar = !!o.polar && (type === 0 || type === 3);
    const shape = o.shape === undefined ? (type === 1 ? 1 : 0) : o.shape;
    let text = "CatChart {\n    width: 720; height: 480\n    scene3D.enabled: true\n"
        + "    scene3D.lightingModel: CatChartScene3D." + (o.lightingModel === 0 ? "Phong" : "BlinnPhong") + "\n"
        + "    scene3D.projection: CatChartScene3D." + (o.orthographic ? "Orthographic" : "Perspective") + "\n"
        + "    scene3D.ambient: " + number(o.ambient, .25) + "\n    scene3D.diffuse: " + number(o.diffuse, .7)
        + "\n    scene3D.specular: " + number(o.specular, .5) + "\n    scene3D.shininess: " + number(o.shininess, 32)
        + "\n    scene3D.ambientGroundRatio: " + number(o.ground, .6) + "\n    scene3D.exposure: " + number(o.exposure, 1) + "\n"
        + "    scene3D.series: [CatChartSeries3D {\n        type: CatChartSeries3D." + types[type] + "\n"
        + "        color: Theme.tokens.primary\n        pointSize: 0.12\n";
    if (type !== 3) text += "        shape: CatChartSeries3D." + shapes[shape] + "\n"
        + "        meshDetail: CatChartSeries3D." + ["LowDetail", "MediumDetail", "HighDetail"][o.meshDetail === undefined ? 1 : o.meshDetail] + "\n";
    if (type !== 3 && shape === 3) text += "        customMesh: [Qt.vector3d(0,.5,0), Qt.vector3d(-.5,-.5,.5), Qt.vector3d(.5,-.5,.5),\n"
        + "            Qt.vector3d(0,.5,0), Qt.vector3d(.5,-.5,.5), Qt.vector3d(0,-.5,-.5),\n"
        + "            Qt.vector3d(0,.5,0), Qt.vector3d(0,-.5,-.5), Qt.vector3d(-.5,-.5,.5),\n"
        + "            Qt.vector3d(-.5,-.5,.5), Qt.vector3d(0,-.5,-.5), Qt.vector3d(.5,-.5,.5)]\n";
    if (polar) text += "        polar: true\n";
    if (type === 2) text += "        nodesVisible: true\n        closed: " + !!o.curveClosed
        + "\n        tension: " + number(o.curveTension, 0) + "\n        subdivisions: " + number(o.curveSteps, 12) + "\n";
    if (type === 3) text += "        surfaceMode: CatChartSeries3D." + ["Filled", "Wireframe", "FilledWireframe"][o.surface === undefined ? 2 : o.surface]
        + "\n        colorByHeight: true\n        closed: " + polar + "\n";
    if (type === 1 || type === 3) {
        text += "        Component.onCompleted: {\n            const points = [];\n            for (let r = 0; r < 9; ++r) {\n"
            + "                for (let c = 0; c < 12; ++c) {\n"
            + "                    const x = c / 11 * 6 - 3, z = r / 8 * 6 - 3;\n"
            + "                    points.push({x: " + (polar ? "c * 30" : "x") + ", y: Math.cos(x) * Math.sin(z), z: " + (polar ? "r / 8 * 3" : "z") + "});\n"
            + "                }\n            }\n            setGrid(9, 12, points);\n        }\n";
    } else if (polar) text += "        points: [{x:0,y:0,z:1}, {x:90,y:1,z:2}, {x:180,y:.5,z:1.5}]\n";
    else text += "        points: [{x:0,y:0,z:0}, {x:1,y:2,z:1}, {x:2,y:1,z:3}, {x:3,y:2,z:2}]\n";
    return text + "    }]\n}";
}

function extended(mode, o) {
    const p = number(o.parameter, mode === 25 ? 1.5 : mode === 27 ? 7 : mode === 29 ? .25 : 1);
    const a = o.a === undefined ? [24,26,27].includes(mode) : !!o.a;
    const b = o.b === undefined ? [24,25,28].includes(mode) : !!o.b;
    const c = o.c === undefined ? [24,27,29].includes(mode) : !!o.c;
    let data = "", chart = "";
    if (mode === 24) {
        data = `CatChartOhlc {
        id: data; candlesticks: ${a}; compressedTime: ${b}
        risingColor: Theme.tokens.success; fallingColor: Theme.tokens.danger
        series.name: qsTr("Price"); volumeSeries.name: qsTr("Volume"); volumeSeries.visible: ${c}
        volumeSeries.yAxis: CatChartAxis { side: CatChartAxis.FarSide; visible: ${c}; Component.onCompleted: setRange(0,500) }
        records: [{time:1704067200000,open:10,high:15,low:8,close:12,volume:80,label:"Mon"},
            {time:1704153600000,open:12,high:14,low:7,close:9,volume:100,label:"Tue"},
            {time:1704412800000,open:9,high:16,low:8,close:${9+p*2},volume:60,label:"Fri"},
            {time:1704672000000,open:12,high:18,low:10,close:16,volume:120,label:"Mon 2"}]
    }`;
        chart = `series: [data.volumeSeries, data.series]
        xAxis.scale: CatChartAxis.${b ? "Category" : "DateTime"}; xAxis.categories: data.categories`;
    } else if (mode === 25) {
        data = `CatChartBoxPlot {
        id: data; whiskerFactor: ${p}
        series.color: Theme.tokens.primary; series.borderColor: Theme.tokens.text
        outlierSeries.color: Theme.tokens.danger; outlierSeries.visible: ${b}
        groups: ${a ? '[{label:"A",minimum:1,q1:2,median:3,q3:4,maximum:6,outliers:[9]},{label:"B",minimum:2,q1:3,median:4,q3:5,maximum:7,outliers:[10]}]' : '[{label:"A",values:[1,2,2,3,3,4,6,9]},{label:"B",values:[2,3,3,4,4,5,7,10]}]'}
    }`;
        chart = "series: [data.series, data.outlierSeries]\n        xAxis.scale: CatChartAxis.Category; xAxis.categories: data.categories";
    } else if (mode === 26 || mode === 27) {
        data = `CatChartGrid {
        id: data; dense: ${a}
        Component.onCompleted: {
            const values = [];
            for (let r=0;r<24;++r) for (let col=0;col<32;++col)
                values.push(${b} && r>=8 && r<=12 && col>=13 && col<=17 ? null : ${mode===26?p:1}*(Math.sin(col*.25)+Math.cos(r*.3)));
            setGrid(24,32,values);
            ${mode===26&&c?'colorScale.setRange(-2,2);':''}
        }
    }`;
        if (mode === 27) {
            const levels=[];for(let i=0;i<Math.round(p);++i)levels.push(number(-2+4*i/(Math.round(p)-1),0));
            data += `
    CatChartContour { id: contours; source: data; levels: ${JSON.stringify(levels)}; filled: ${a}; labelsVisible: ${c} }`;
            chart = "series: [contours.series]";
        } else chart = "series: [data.series]";
    } else if (mode === 28) {
        data = `CatChartBubble {
        id: data; sizeMode: CatChartBubble.${a?'Area':'Radius'}; sizeScale: ${p*(a?64:1)}; colorMapping: ${b}
        points: [{x:0,y:2,size:8,colorValue:-1},{x:1,y:4,size:16,colorValue:0},{x:2,y:3,size:24,colorValue:1},{x:3,y:5,size:32,colorValue:2}]
        ${c?'Component.onCompleted: colorScale.setRange(0,1)':''}
    }`;
        chart = "series: [data.series]\n        Component.onCompleted: { xAxis.setRange(-1,4); yAxis.setRange(1,6); }";
    } else {
        data = `CatChartSeries { id: first; name: qsTr("Series A"); color: Theme.tokens.primary; fillOpacity: ${p}; strokeStyle: CatChartSeries.${a?'DashLine':'SolidLine'}; markersVisible: ${c}
        Component.onCompleted: setData([{x:0,y:70},{x:1,y:6},{x:2,y:50},{x:3,y:7},{x:4,y:65}]) }
    CatChartSeries { id: second; name: qsTr("Series B"); color: Theme.tokens.success; fillOpacity: first.fillOpacity; strokeStyle: first.strokeStyle; markersVisible: first.markersVisible
        Component.onCompleted: setData([{x:0,y:55},{x:1,y:8},{x:2,y:70},{x:3,y:4},{x:4,y:80}]) }`;
        chart = `chartType: CatChartSeries.Radar; series: [first, second]
        xAxis.categories: [qsTr("Speed"),qsTr("Quality"),qsTr("Capacity"),qsTr("Efficiency"),qsTr("Reliability")]
        radarRanges: ${b?'[{minimum:0,maximum:100},{minimum:0,maximum:10},{minimum:0,maximum:100},{minimum:0,maximum:10},{minimum:0,maximum:100}]':'[]'}`;
    }
    return `Item {
    width: 720; height: 480
    property alias chart: plot
    ${data}
    CatChart { id: plot; anchors.fill: parent
        ${chart}
    }
}`;
}
