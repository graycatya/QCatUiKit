# Choosing and composing QCatUiKit controls

## Component selection

| Intent | Public types and distinctions |
| --- | --- |
| Page or panel | CatWindow, CatPane; use Qt Quick Layouts for structure |
| Text and validation | CatTextField / CatTextArea with invalid; CatPasswordField for passwords |
| Numeric input | CatSpinBox for integers, CatNumberBox for decimals; configure bounds |
| Slider | CatSlider for one value, CatRangeSlider for an interval, CatSliderField for slider plus precise numeric input; see SLIDERS.md |
| Choice | CatCheckBox, CatRadioButton with ButtonGroup, CatSwitch, CatComboBox, CatMultiSelectComboBox |
| Navigation | CatNavigationView, CatBreadcrumb, CatTabView, CatStackView; app owns navigation state |
| Collection | CatListView / CatGridView / CatTreeView / CatTableView; use models and bounded views |
| Conversation | ChatModel + CatChatView + CatChatInput; application handles delivery |
| Feedback | CatAlert, CatToast, CatBusyIndicator, CatProgressBar; tie visibility to real state |
| Chart | CatChart + CatChartSeries; configure CatChartAxis through xAxis/yAxis |
| Slide or flip | CatCarousel slides between items; CatFlipView exposes front/back faces |
| File interaction | CatFileDropZone emits file requests; CatFileListView displays an app-provided model |

Consult the library component catalog before adding an unfamiliar property. For example,
CatChart does not use the Qt Charts ChartView/LineSeries API, and CatTableView does not
create a database or supply application selection state automatically.

## Layout and theme

A common page is CatWindow → CatScrollView → ColumnLayout, with a width bound to
the scroll view's availableWidth. Use GridLayout.columns for a responsive form and
Layout.fillWidth on controls. Set a meaningful preferred height on nested data views.
Do not nest an unbounded view inside a scrolling page and expect virtualization to help.

CatScrollView supports mouse dragging, wheel scrolling and touch flicking. Use it for
scrolling pages and panels; text inputs retain mouse selection and nested data views
handle their own scrolling. Avoid full-page MouseArea or DragHandler overlays that
steal input from editors, buttons or charts. Applications should use CatScrollView
directly, without depending on the library's internal gesture helpers.

Theme.tokens provides background, surface, text, textSecondary, border, primary,
onPrimary, danger, radius, fontSize, padding, spacing and controlHeight. Seed properties
include primaryColor, colorScheme, density and reducedMotion. Light/Dark and
Desktop/Touch/Mobile are ThemePalette enums. The standard control heights are 32/48/44
logical pixels respectively. Do not interpret these as fixed pixel heights for every item.

For a local palette, define ThemePalette on the owning page and pass theme to its Cat
children. Passing a palette to one outer CatPane does not theme arbitrary app-owned
descendants automatically. Avoid toggling global Theme from a reusable leaf control.

## Tables

TableModel has columns (unique nonempty field keys), headers and count. Use append(map),
update(row, changes), setCell(row, column, value), get(row), remove(row) and clear().
get() returns a copy; editing that copy does not notify the model. Row and column indices
are zero-based. The columns list determines visual field order.

Bind CatTableView.model to the model. resizableColumns enables column resizing;
columnWidthProvider and rowHeightProvider control geometry. Its view property exposes
the underlying TableView for selection and positioning. Keep model values separate from
widths and cursor state. The starter demonstrates adding a validated row and removing rows.

## Conversation

ChatModel accepts sender, recipient, username, content, timestamp, messageId, type,
status and avatar fields. Use ChatModel.Text/Image/Audio/Video and Pending/Sent/Failed.
The built-in non-text delegate displays attachment information, not a media player.

Connect CatChatInput.onSubmitted to the application's send operation. submit() trims
text, rejects empty submissions and clears a successful local submission. Network success
is separate: append a Pending message, then update its status when the actual operation
finishes. Do not mark a real network request Sent just because the editor emitted a signal.

CatChatView.currentUser matches sender for message direction. Disable followTail when
the user chooses history browsing; call positionViewAtEnd() when they request the latest
message. Preserve unsent drafts across app-controlled navigation or live updates as needed.

## Charts and throughput

CatChart owns its two primary axes; optional series axis references retain application ownership; a CatChartSeries child supplies samples. appendData and setData
accept maps such as {x: 0, y: 12, value: 1, label: "A"}. XY must be finite. For a category
axis, set xAxis.scale to CatChartAxis.Category and supply xAxis.categories. Horizontal bar
types instead use yAxis for categories, while samples retain x = category and y = value.
mapToPosition/mapToValue always use physical axis coordinates. DateTime values are UTC
Unix milliseconds; logarithmic ranges must be positive.

Series capacity defaults to 100,000 and overwrites oldest samples. CatChart.displayBudget
samples ordered Line/Area: linear curves without breaks use peak sampling; dense step/spline curves use a cached segment hierarchy that preserves extrema and separate runs. Zooming restores original curve detail; displayBudget 0 disables overview sampling. Raw records remain available for picking. RangeArea, unsorted curves and linear curves with breaks do not use this hierarchy. It does not aggregate arbitrary scatter, bars or heatmaps.
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
percentage labels are omitted; no outside leader labels are implemented yet.

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

## LED 工作流

需要灯点布局、预设、双 Deck 或多通道输出时，阅读 `src/uikit/doc/LEDWORKFLOW.md`。
使用公共 `LedProject` 可选装配或独立注入 LayoutModel/Registry/Repository/Deck/Mixer；不要依赖
`GalleryLedDemo`。`LedDeck`、`LedMixer`、`LedStorageBrowser` 及三个设置面板可直接复用。
预览亮度与 Deck/Place/Master 不同；RGBA alpha 不代表 W。设备分支读取公共 LightingFrame，
不用截图或 renderer 私有头，MemoryOutputBackend 只表示内存验证。

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

M2 additions: series can reference independent `xAxis`/`yAxis` objects; set their orientation,
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

Use CatSlider ticks, valueToolTipVisible, valueFormatter and fillOrigin for scalar parameter feedback. CatSliderField adds CatNumberBox, suffix, decimals and resetVisible without app-owned duplicate editors. Write the backing model in onMoved (CatSlider/CatRangeSlider) or onValueEdited (CatSliderField); programmatic synchronization should not emit user signals. Use valueCommitted / valuesCommitted for expensive work after an edit. SliderField automatically stacks at narrow widths. Keep long units/labels concise and give each slider an Accessible.name; CatRangeSlider names its two handles separately.

For a percentage editor backed by a normalized model, display `value: model.opacity * 100` in a 0–100 field and write `model.opacity = value / 100` in `onValueEdited`; express defaultValue in the displayed units too. A locked chart interval can bind fixedSpan to its current viewport span and use -1 when unlocked; update that span before setValues during external zoom synchronization.

For 3D charts use `CatChart.scene3D.enabled: true` and `scene3D.series: [CatChartSeries3D { ... }]`. Keep XYZ/row-major grid data in this new type; never reinterpret the 2D Sample.value as Z. Read `src/uikit/doc/CHARTS3D.md` for camera, Blinn–Phong / Phong material, grid slicing and polar contracts. 3D requires a hardware RHI backend; inspect scene3D.supported/error rather than treating software rendering as a working 3D preview.

### Statistical charts

Use `CatChartStack` for ordinary/percentage stacked areas and bind `CatChart.series: stack.series`; style its input series. Inputs must share strictly increasing X coordinates and axes. Use `CatHistogram` for raw observations, automatic/custom bin edges and Count/Frequency/Density, with `series: [histogram.series]`. Adapters own their derived series; do not replace their data or type. Inspect `error` and use `sourceIndex` or bin metadata for original-data semantics. See [statistical charts](../../../src/uikit/doc/CHARTSTATISTICS.md).

Use `CatChartFit` for an ordinary least-squares trend over a source CatChartSeries; display its `series` and optional `band` alongside observations. The band is fitted Y ± multiplier × RMSE, not a confidence/prediction interval. Inspect `valid`/`error`, and use `predict(x)` for evaluated values. Use `CatWaterfall` for cumulative changes: Change steps require value; Subtotal/Total calculate their value and must omit it. Bind a Category X axis to `categories`. Optional resetOnSubtotal returns to initialValue after each subtotal; `value`/`delta` preserve the raw change for picking and labels.


M4 adapters: use CatChartBubble for independent radius/area and color encoding; use the owned CatChartColorScale for automatic or fixed bounds. CatChartGrid accepts row-major finite values or null gaps, and supports dense scalar textures, sparse cells, setValue and appendRow. CatChartContour references that grid for isolines and filled bands. CatChartOhlc validates time/open/high/low/close/volume and exposes independent price and volume series; explicitly choose the volume axis. CatChartBoxPlot accepts raw observations or ordered summaries and exposes separate outliers. CatChart.radarRanges normalizes each radar dimension without changing raw observations. Read [M4 extensions](../../../src/uikit/doc/CHARTEXTENSIONS.md) for limits, quantile/contour algorithms and missing-value semantics. GalleryChartM4Preview is not a library API.
