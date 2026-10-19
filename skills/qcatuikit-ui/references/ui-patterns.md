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

## Slider edits and chart ranges

Use CatSlider for scalar values, CatRangeSlider for intervals and CatSliderField for a slider with precise entry. Write models in onMoved or onValueEdited, and use valueCommitted / valuesCommitted for expensive final work. Programmatic synchronization should use the public setters without emitting user-edit signals. See [slider contracts](../../../src/uikit/doc/SLIDERS.md).

For a normalized model displayed as percent, bind value to model.value * 100 and convert back in onValueEdited; express defaultValue in displayed units. For chart viewport synchronization, follow [chart range patterns](charts.md), keeping the overview domain separate from the selected interval.
