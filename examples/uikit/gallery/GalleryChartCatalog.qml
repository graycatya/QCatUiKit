// SPDX-License-Identifier: Apache-2.0
pragma Singleton
pragma ComponentBehavior: Bound
import QtQuick
import "GalleryChartExamples.js" as Examples

QtObject {
    readonly property var modeNames: [qsTr("Line"), qsTr("Area"), qsTr("Grouped bars"),
        qsTr("Stacked bars"), qsTr("Scatter"), qsTr("Bubble"), qsTr("Pie"), qsTr("Doughnut"),
        qsTr("Radar"), qsTr("Heatmap"), qsTr("Bar + line"), qsTr("Time axis"), qsTr("Log axis"),
        qsTr("Positive / negative"), qsTr("Scatter + trend"), qsTr("Range area"),
        qsTr("Segmented curves"), qsTr("Pie / ring variants"), qsTr("Multiple axes"), qsTr("3D charts"), qsTr("Stacked area"), qsTr("Histogram"), qsTr("Linear fit"), qsTr("Waterfall"), qsTr("OHLC and volume"), qsTr("Box plots"), qsTr("Grid heatmap"), qsTr("Contours"), qsTr("Bubble mapping"), qsTr("Radar styling")]
    readonly property var groups: [
        { id: "2d", title: qsTr("2D charts") },
        { id: "3d", title: qsTr("3D charts") },
        { id: "extended", title: qsTr("Extended charts") },
        { id: "configuration", title: qsTr("Axes and combinations") }
    ]
    readonly property var entries: [
        { id: "chart-ohlc", title: qsTr("OHLC and volume"), group: "extended", modes: [24],
            description: qsTr("Candlesticks, OHLC ticks, trading gaps and original price/volume inspection.") },
        { id: "chart-box", title: qsTr("Box plots"), group: "extended", modes: [25],
            description: qsTr("Raw observations or summaries, interpolated quartiles, whiskers and outliers.") },
        { id: "chart-contour", title: qsTr("Contours"), group: "extended", modes: [27],
            description: qsTr("Isolines and filled bands over grids, with shared colors and missing cells.") },
        { id: "chart-waterfall", title: qsTr("Waterfall charts"), group: "extended", modes: [23],
            description: qsTr("Changes, subtotals and totals with connectors, reset rules and original-value inspection.") },
        { id: "chart-fit", title: qsTr("Fit and residual band"), group: "configuration", modes: [22],
            description: qsTr("Public linear regression with original observations, statistics and a residual band.") },
        { id: "chart-histogram", title: qsTr("Histograms"), group: "extended", modes: [21],
            description: qsTr("Automatic or custom bins, counts, frequency and probability density from raw observations.") },
        { id: "chart-stack", title: qsTr("Stacked areas"), group: "extended", modes: [20],
            description: qsTr("Ordinary and percentage stacks with signed values, shared gaps and original-value inspection.") },
        { id: "chart-line", title: qsTr("Line charts"), group: "2d", modes: [0, 16],
            description: qsTr("Lines, steps, splines and explicit gaps, with stroke and marker options.") },
        { id: "chart-area", title: qsTr("Area charts"), group: "2d", modes: [1, 15],
            description: qsTr("Filled areas and upper/lower ranges, with baselines and boundary alignment.") },
        { id: "chart-bars", title: qsTr("Bar charts"), group: "2d", modes: [2, 3, 13],
            description: qsTr("Grouped, stacked and percentage bars, including horizontal and signed layouts.") },
        { id: "chart-scatter", title: qsTr("Scatter and bubble charts"), group: "2d", modes: [4, 5, 28],
            description: qsTr("Point distributions and bubble sizes, with markers, selection and live data.") },
        { id: "chart-pie", title: qsTr("Pie and ring charts"), group: "2d", modes: [6, 7, 17],
            description: qsTr("Pie, doughnut and concentric rings, with slice labels and separation.") },
        { id: "chart-radar", title: qsTr("Radar charts"), group: "2d", modes: [8, 29],
            description: qsTr("Compare multiple series across labeled dimensions.") },
        { id: "chart-heatmap", title: qsTr("Heatmaps"), group: "2d", modes: [9, 26],
            description: qsTr("Map cell values to colors and inspect the underlying data.") },
        { id: "chart-3d", title: qsTr("3D charts"), group: "3d", modes: [19],
            description: qsTr("Scatter, bars, trajectories and surfaces, with cameras, lighting and slices.") },
        { id: "chart-combination", title: qsTr("Combination charts"), group: "configuration", modes: [10, 14],
            description: qsTr("Combine bars and lines, or scatter points and a fitted trend.") },
        { id: "chart-axes", title: qsTr("Axes and scales"), group: "configuration", modes: [18, 11, 12],
            description: qsTr("Independent axes, time and logarithmic scales, reversal and minor ticks.") }
    ].map(entry => Object.assign({}, entry, {
        name: "CatChart",
        category: "charts", icon: "data",
        searchTerms: entry.id === "chart-ohlc" ? "CatChartOhlc" : entry.id === "chart-box" ? "CatChartBoxPlot" : entry.id === "chart-contour" ? "CatChartContour CatChartGrid" : entry.id === "chart-heatmap" ? "CatChartGrid CatChartColorScale" : entry.id === "chart-scatter" ? "CatChartBubble CatChartColorScale" : entry.id === "chart-waterfall" ? "CatWaterfall CatChartSeries" : entry.id === "chart-fit" ? "CatChartFit CatChartSeries" : entry.id === "chart-3d" ? "CatChartSeries3D CatChartScene3D" : entry.id === "chart-stack" ? "CatChartStack CatChartSeries"
            : entry.id === "chart-histogram" ? "CatHistogram CatChartSeries" : "CatChartSeries CatChartAxis",
        code: Examples.code(entry.modes[0])
    }))
    function canonicalId(id: string): string {
        if (id === "chart") return "chart-line";
        if (id === "chart-axis") return "chart-axes";
        if (id === "chart-series") return "chart-area";
        return id;
    }
    function entryForId(id: string): var {
        const canonical = canonicalId(id);
        return entries.find(entry => entry.id === canonical) || null;
    }
    function entryForMode(mode: int): var {
        return entries.find(entry => entry.modes.includes(mode)) || null;
    }
}
