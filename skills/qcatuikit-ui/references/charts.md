# Chart composition and throughput

[Library chart APIs](../../../src/uikit/doc/CHARTS.md) · [Rendering boundaries](../../../src/uikit/doc/CHART_RENDERING.md)

## Two-dimensional data and interaction

CatChart owns its two primary axes; optional series axis references retain application ownership; a CatChartSeries child supplies samples. appendData and setData
accept maps such as {x: 0, y: 12, value: 1, label: "A"}. XY must be finite. For a category
axis, set xAxis.scale to CatChartAxis.Category and supply xAxis.categories. Horizontal bar
types instead use yAxis for categories, while samples retain x = category and y = value.
mapToPosition/mapToValue always use physical axis coordinates. DateTime values are UTC
Unix milliseconds; logarithmic ranges must be positive.

Series capacity defaults to 100,000 and overwrites oldest samples. CatChart.displayBudget
samples ordered Line/Area/RangeArea in all seven curve modes through a cached segment hierarchy that preserves extrema and separate runs. Zooming restores original curve detail; displayBudget 0 disables overview sampling. Raw records remain available for picking. Unsorted X does not use this hierarchy; breaks isolate runs, and RangeArea preserves both boundary extrema. It does not aggregate arbitrary scatter, bars or heatmaps.
Application-level aggregation must be explicit and must preserve raw data when required.
Keep antialiasing at its default true unless the task specifically requires a comparison.

Use curveMode for step/natural/monotone/Catmull-Rom curves. markerShape and markerSize
configure preset points; markersVisible enables Line/Area nodes. Bubble still takes its
radius from each sample's value. PercentBar and HorizontalPercentBar normalize positive
and negative totals separately and return raw values plus percentage from dataAt. See
src/uikit/doc/CHARTS.md for interpolation, picking and normalization rules.

Use bounded batches on the QObject's owning thread, delivering worker results through
queued connections. A stop button must get an event-loop opportunity to run. For streaming
views, separately define retained capacity, append rate, drawing budget and history policy.


Use breakBefore in data batches or setBreakBefore(index, bool) to separate runs; NaN still
rejects the entire batch. RangeArea uses setRangeData/appendRangeData with paired
{x, lower, upper}; both boundaries share curveMode and splineTension (straight, steps or
splines). Breaks isolate both boundaries and automatic axes include their extrema. Do not put
the lower boundary in Sample.value. strokeStyle/dashPattern/dashOffset configure dashes
in logical screen pixels; fillOpacity controls Area/RangeArea fill independently of the border.
Area offers areaBaselineMode (ZeroBaseline / CustomBaseline / PlotBottomBaseline) and a finite
areaBaseline value. Area/RangeArea share fillColor, borderColor, fillStyle (SolidFill /
VerticalGradient / HorizontalGradient), and fillEndColor. Gradients span the whole plot and
interpolate premultiplied alpha; invalid QColor inherits the series color (end inherits fill).
RangeArea ignores areaBaseline. Nonpositive baselines on a log axis fall back to the plot bottom.

Pie/Doughnut support pieStartAngle/pieEndAngle (degrees clockwise from the top),
pieRadius, holeSize and per-record sliceStyle. Use setPieAngles(start, end) for an atomic
angle change. Doughnut with multiRing:true maps visible nonempty series outer to inner;
each ring normalizes independently. setSliceStyle replaces the entire map; use color,
borderColor, borderWidth, explodeDistance (logical pixels), labelVisible. Empty maps reset.
Chart-local setSliceVisible(seriesIndex, index, visible) controls legend masks independently
for each ring; changing series data restores only that series' masks. Crowded internal
percentage labels are omitted. Outside labels with leaders and budgeted label delegates are available as described below.

For six bar layouts, barGroupWidth (default .8 category units) and barSpacing (default 0,
fraction of each slot left empty) belong to CatChart. Zero width or spacing 1 hides bars.
Series barBorderWidth/barBorderColor style the inside border. barLabelsVisible defaults
false; barLabelPosition offers center, inside end/base, outside end. barLabelFormat accepts
@value (raw Y), @percentage (signed, percent layouts only), @category and @series.
Use barLabelPrecision for significant digits and barLabelMargin for logical-pixel padding.
Only the first 256 eligible visible bars are considered; labels that do not fit are omitted.

CatChart dragMode defaults to PanDrag; RectangleZoomDrag enables a mouse selection overlay.
rectangleZoomAxes selects ZoomBothAxes / ZoomXAxis / ZoomYAxis; the other axis retains
its automatic/manual state. zoomToRect uses chart-local logical pixels (at least 6 per selected
dimension); zoomSelecting and zoomSelection expose pending state, and cancelZoomSelection
or Escape cancels it. Streaming updates that change the mapping cancel pending selection.
Rectangle zoom controls mouse dragging; touch pan/pinch is configured separately with
`touchNavigationEnabled`. See CHARTS.md for cancellation and input rules.

CatChart touchNavigationEnabled defaults to true: one finger pans, two fingers pinch and
translate around their center, independently of mouse dragMode. Disable it inside a
scrolling page when touch drags should scroll the parent; taps still emit pointClicked.
Touch input requires interactive=true. Radial charts do not pan or pinch.

For pie/doughnut labels, use `sliceLabelPosition: CatChart.SliceLabelOutside` to show names and percentages with leaders. The default `SliceLabelInside` preserves internal percentages. Outside mode reserves side columns and can omit crowded labels; `sliceLabelsVisible: false` restores the original radius. Labels do not change `dataAt()` hit regions.

Chart axes expose `reversed` (default false) and `minorTickCount` (0–10, default 0).
Keep minimum < maximum even when reversed; do not reverse the source arrays.
Map/pick/zoom/pan and touch use the axis direction. `resetView()` preserves these styles.
Minor ticks subdivide major intervals in linear or log10 axis space; Category ignores them.
`PlotBottomBaseline` stays at the physical plot bottom when Y is reversed.

Additional composition: series can reference independent `xAxis`/`yAxis` objects; set their orientation,
side and visibility explicitly. `fillGradientStops` and local/qrc `fillTexture` customize area
fills. Bars and preset/texture markers use the GPU instance path. Arbitrary `pointDelegate`,
`barDelegate` and `labelDelegate` components use the chart's shared `delegateBudget` (128 by
default); excess records retain built-in rendering. Delegate roots are QQuickItems with optional
`required property var chartData`; the chart owns root geometry and reuses visible records.

Use `selectionMode`, `selectedPoints`, `selectPoint()` and `clearSelection()` for explicit selection.
A series data mutation clears its selection; style and visibility preserve it. Selection and hover
update the independent overlay and delegate payloads without data-buffer uploads. Optional
`animationsEnabled` fades appearance/selection changes using the theme motion duration;
`reducedMotion` immediately settles transitions and never stops incoming data.

For external range sliders, use public CatRangeSlider and write to the corresponding axis with `setRange()` only on its aggregate `moved` signal, which includes whole-range dragging. Sync back with `setValues()` on chart `viewChanged` and series `dataChanged`, coalescing notifications. Read the actual viewport using `plotRect()` and `mapToValue()`; axis minimum/maximum retain manual settings while automatic. Keep the overview domain separate from the selected viewport. Set a positive minimumSpan no greater than the actual viewport span, so deep external zooms still synchronize. The Gallery line examples contain standalone binding code; GalleryChartRangeControls remains a private axis adapter.

A locked chart interval can bind fixedSpan to its current viewport span and use -1 when unlocked; update that span before setValues during external zoom synchronization. Scalar/percentage edit patterns are in [UI patterns](ui-patterns.md).

## Three-dimensional scenes

For 3D charts use `CatChart.scene3D.enabled: true` and `scene3D.series: [CatChartSeries3D { ... }]`. Keep XYZ/row-major grid data in this new type; never reinterpret the 2D Sample.value as Z. Read `src/uikit/doc/CHARTS3D.md` for camera, Blinn–Phong / Phong material, grid slicing and polar contracts. 3D requires a hardware RHI backend; inspect scene3D.supported/error rather than treating software rendering as a working 3D preview.

## Statistical and extended charts

Use `CatChartStack` for ordinary/percentage stacked areas and bind `CatChart.series: stack.series`; style its input series. Inputs must share strictly increasing X coordinates and axes. Use `CatHistogram` for raw observations, automatic/custom bin edges and Count/Frequency/Density, with `series: [histogram.series]`. Adapters own their derived series; do not replace their data or type. Inspect `error` and use `sourceIndex` or bin metadata for original-data semantics. See [statistical charts](../../../src/uikit/doc/CHARTSTATISTICS.md).

Use `CatChartFit` for an ordinary least-squares trend over a source CatChartSeries; display its `series` and optional `band` alongside observations. The band is fitted Y ± multiplier × RMSE, not a confidence/prediction interval. Inspect `valid`/`error`, and use `predict(x)` for evaluated values. Use `CatWaterfall` for cumulative changes: Change steps require value; Subtotal/Total calculate their value and must omit it. Bind a Category X axis to `categories`. Optional resetOnSubtotal returns to initialValue after each subtotal; `value`/`delta` preserve the raw change for picking and labels.


Extended adapters: use CatChartBubble for independent radius/area and color encoding; use the owned CatChartColorScale for automatic or fixed bounds. CatChartGrid accepts row-major finite values or null gaps, and supports dense scalar textures, sparse cells, setValue and appendRow. CatChartContour references that grid for isolines and filled bands. CatChartOhlc validates time/open/high/low/close/volume and exposes independent price and volume series; explicitly choose the volume axis. CatChartBoxPlot accepts raw observations or ordered summaries and exposes separate outliers. CatChart.radarRanges normalizes each radar dimension without changing raw observations. Read [chart extensions](../../../src/uikit/doc/CHARTEXTENSIONS.md) for limits, quantile/contour algorithms and missing-value semantics. GalleryChartM4Preview is not a library API.

## Export the current chart image

Use `chart.exportImage(fileDialog.selectedFile)` for a PNG of the complete current chart, including axes, title, legend and child overlays. Bind the action's enabled state to `!chart.exportingImage`; display success or the error from `imageExportFinished(destination, success, error)`. The return value only reports request acceptance. An optional `Qt.size(width, height)` specifies exact pixel dimensions; omit it for display pixels. Larger dimensions scale the current view without increasing its LOD or RHI texture detail. The chart must remain in a visible window; 3D requires a graphics backend. Keep the save dialog and configuration/data persistence in the application. See CHARTS.md for limits and lifecycle behavior.
