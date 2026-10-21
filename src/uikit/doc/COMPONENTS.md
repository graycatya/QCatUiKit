# 组件目录与公开行为

[文档索引](README.md) · [Gallery 使用指南](../../../examples/uikit/gallery/README.md)

当前包含 119 个 Cat 前缀 QML 类型，以下按 Gallery 的 16 个分类组织。使用 `import QCatUiKit` 导入；主题和模型等辅助类型另见文末。

所有带主题的组件默认 `theme: Theme`，`tokens` 为解析后的只读视图。继承 Qt 控件的类型支持相应基类的标准属性、信号、方法及可访问性语义；表格列出本库的主要入口。内部 id、子对象树与 SurfaceImpl 不属于公开接口。

## 图表

| 组件 | 主要 API / 行为 |
| --- | --- |
| [CatChart](../qcatchartitem.h) | 图表容器；`chartType`、`series`、`xAxis/yAxis`、`theme`、`interactive`；缩放、平移、单选／多选、预算委托、可选过渡、图例、提示及默认反走样；`renderingStatistics()` 查询渲染与上传诊断。 |
| [CatChartAxis](../qcatchartaxis.h) | `scale`、`minimum/maximum`、`automatic`、`categories`、`orientation`、`side`、`visible`、`reversed`、`minorTickCount`、`setRange()`；线性、对数、UTC 时间与分类轴。 |
| [CatChartColorScale](CHARTEXTENSIONS.md) | 自动／固定范围、预乘颜色插值和色条。 |
| [CatChartBubble](CHARTEXTENSIONS.md) | 独立半径／面积编码、大小倍数与颜色映射。 |
| [CatChartGrid](CHARTEXTENSIONS.md) | 格网、缺失单元、单点更新、滚动行、密集纹理／稀疏实例。 |
| [CatChartContour](CHARTEXTENSIONS.md) | 格网等值线、填色区域、级别和标签。 |
| [CatChartOhlc](CHARTEXTENSIONS.md) | K 线／OHLC、交易间隔压缩与独立成交量。 |
| [CatChartBoxPlot](CHARTEXTENSIONS.md) | 原始观测／摘要、分位数、箱须和离群点。 |
| [CatChartFit](CHARTSTATISTICS.md) | `source`、`series`、`band`、`residualMultiplier`、`valid`；普通最小二乘、斜率／截距／R²／RMSE 和预测值。 |
| [CatWaterfall](CHARTSTATISTICS.md) | `steps`、`initialValue`、`resetOnSubtotal`、`connectorsVisible`、`series`；增减、小计、总计和累计值拾取。 |
| [CatChartStack](CHARTSTATISTICS.md) | `sources`、只读 `series`、`percentage`、`error`；严格 X 对齐、正负分组、过零切段及共享断点的堆叠面积适配器。 |
| [CatHistogram](CHARTSTATISTICS.md) | `values`、`binCount`、`binEdges`、`normalization`、`series`；保留原始观测值，生成频数／频率／密度及箱边界拾取信息。 |
| [CatChartSeries](../qcatchartseries.h) | `type`、`name`、`color`、`visible`、`capacity`、`count`；`curveMode`、`splineTension`、`markerShape`、`markerSize`、`markersVisible`；阶梯／样条、预设／纹理标记、六种柱图布局、多轴关联、多停止点／纹理填充、点／柱体／标签委托；`append()`、`appendData()`、`setData()`、`at()`、`clear()`。 |

曲线外观还支持 `strokeStyle`、`dashPattern`、`dashOffset`；面积支持 `areaBaselineMode/areaBaseline`；Area／RangeArea 共用 `fillColor/borderColor`、`fillStyle/fillEndColor` 和 `fillOpacity`。断点使用批次字段 `breakBefore` 或 `setBreakBefore()`；区间面积使用 `RangeArea` 与 `setRangeData()`／`appendRangeData()`，C++ 对应独立 `RangeSample` 契约。RangeArea 的上下界共用七种 `curveMode` 与 `splineTension`，并支持断点、交叉填充和双边界极值范围。完整语义见 [图表指南](CHARTS.md)。

饼／环支持 `pieStartAngle/pieEndAngle`、`pieRadius`、`holeSize`、`multiRing`、`ringGap`、`sliceLabelsVisible` 与 `sliceLabelPosition`（内置百分比／外置引线标签）；`setSliceStyle()` 设置单扇区颜色、边框、分离距离及标签开关，`setSliceVisible()` 独立控制各环扇区。

柱状外观使用图表 `barGroupWidth/barSpacing` 与系列 `barBorderWidth/barBorderColor`。数据标签使用 `barLabelsVisible`、`barLabelPosition`、`barLabelFormat`、`barLabelPrecision`、`barLabelMargin`，六种布局共用。

图表交互新增 `dragMode`（PanDrag／RectangleZoomDrag）、`rectangleZoomAxes`（ZoomBothAxes／ZoomXAxis／ZoomYAxis），并提供 `zoomToRect()`、`cancelZoomSelection()` 以及只读 `zoomSelecting/zoomSelection`。框选取消与坐标约定见 [图表指南](CHARTS.md#框选缩放)。`touchNavigationEnabled` 默认开启单指平移／双指缩放，关闭后保留轻点拾取并允许外层页面接管触摸拖动，详见 [触摸交互](CHARTS.md#触摸平移与双指缩放)。

`CatChart.exportImage(fileUrl, pixelSize)` 导出当前完整视图为 PNG，`exportingImage` 和 `imageExportFinished` 提供状态；支持二维及受支持的三维后端。

### 三维场景

`CatChart.scene3D` 及 `CatChartSeries3D` 提供三维柱阵、散点、轨迹和规则曲面，支持极坐标、相机、Blinn–Phong／Phong 光照、深度拾取及行列切片。`CatChartScene3D` 由图表持有，不直接在 QML 中创建。接口、GPU 要求与限制见 [三维图表](CHARTS3D.md)。

## 输入与容器

| 组件 | 主要 API / 行为 |
| --- | --- |
| [CatAutoSuggestBox](../qml/CatAutoSuggestBox.qml) | `suggestions`, `filteredSuggestions`, `isSuggestionListOpen`, `suggestionAccepted()`；输入时筛选建议，使用方向键或回车选择。 |
| [CatColorPicker](../qml/CatColorPicker.qml) | `selectedColor`, `alphaEnabled`, `colorSelected()`；选择预设颜色，或编辑 RGB、透明度和十六进制值。 |
| [CatCompoundButton](../qml/CatCompoundButton.qml) | `secondaryText`；在操作标题下补充说明文字。 |
| [CatDropDownButton](../qml/CatDropDownButton.qml) | `isOpen`；通过支持键盘操作的按钮打开菜单。 |
| [CatFileDropZone](../qml/CatFileDropZone.qml) | `title`, `description`, `hintText`, `browseText`, `errorMessage`, `acceptedExtensions`, `dragActive`, `filesDropped()`；接收符合筛选条件的本地文件拖放，并向应用请求文件浏览。 |
| [CatHyperlinkButton](../qml/CatHyperlinkButton.qml) | `url`, `showUnderline`；激活文字链接，并可打开指定网址。 |
| [CatMultiSelectComboBox](../qml/CatMultiSelectComboBox.qml) | `model`, `textRole`, `selectedIndexes`, `placeholderText`, `searchEnabled`, `selectedCount`, `selectedTexts`, `selectionEdited()`；选择多个选项，并可通过文字筛选。 |
| [CatNumberBox](../qml/CatNumberBox.qml) | `locale`, `value`, `minimum`, `maximum`, `smallChange`, `largeChange`, `acceptsExpression`, `decimals`, `valueEdited()`；编辑有范围限制的小数，支持步进、本地格式及算术表达式。 |
| [CatPasswordField](../qml/CatPasswordField.qml) | `password`, `revealed`, `revealEnabled`；输入密码，并通过支持键盘操作的开关显示明文。 |
| [CatRatingControl](../qml/CatRatingControl.qml) | `value`, `placeholderValue`, `maxRating`, `isReadOnly`, `isClearEnabled`, `caption`, `stepSize`, `ratingEdited()`；选择或清除评分，支持小数显示及键盘步进。 |
| [CatRepeatButton](../qml/CatRepeatButton.qml) | `delay`, `interval`；在按住鼠标或按键期间重复触发操作。 |
| [CatSpinBox](../qml/CatSpinBox.qml) | 通过文字、按钮或键盘编辑范围内的整数。 |
| [CatSplitButton](../qml/CatSplitButton.qml) | `text`, `iconSource`, `primary`, `checkable`, `checked`, `isOpen`；将主要操作与更多菜单操作分开。 |
| [CatTextArea](../qml/CatTextArea.qml) | `invalid`；编辑、选择多行文本，并显示主题焦点及校验状态。 |
| [CatToggleButton](../qml/CatToggleButton.qml) | `threeState`, `checkState`；保持选中状态，或在三种选择状态之间循环。 |
| [CatToggleSplitButton](../qml/CatToggleSplitButton.qml) | 切换主要操作的选中状态，同时保留独立菜单。 |
| [CatCheckBox](../qml/CatCheckBox.qml) | 标准 CheckBox；支持 `checked`、`tristate` 与 `checkState` |
| [CatRadioButton](../qml/CatRadioButton.qml) | 标准 RadioButton；支持 Qt ButtonGroup 与排他选择 |
| [CatSwitch](../qml/CatSwitch.qml) | 标准 Switch；鼠标、触摸拖动和键盘切换 |
| [CatSlider](../qml/CatSlider.qml) | 单值滑块；刻度、提示、居中填充、默认值恢复；[滑块 API](SLIDERS.md) |
| [CatRangeSlider](../qml/CatRangeSlider.qml) | 双端区间、跨度限制、固定跨度、整段拖动；[滑块 API](SLIDERS.md) |
| [CatSliderField](../qml/CatSliderField.qml) | 滑块与精确数值框、单位、默认值恢复、自适应布局；[滑块 API](SLIDERS.md) |
| [CatComboBox](../qml/CatComboBox.qml) | 标准 ComboBox；可编辑、textRole/valueRole、键盘导航与主题弹出列表 |
| [CatButton](../qml/CatButton.qml) | 标准 Button；`primary`, `danger`, `loading`, `iconSource`；loading 临时禁用并恢复 enabled；primary 决定填充，danger 决定危险边框/普通文字 |
| [CatTextField](../qml/CatTextField.qml) | 标准 TextField；`invalid` 控制错误边框；可配置 validator / inputMethodHints |
| [CatIpAddressField](../qml/CatIpAddressField.qml) | 单输入框 IPv4；使用 `acceptableInput` 判断可提交性；聚焦时允许中间输入状态 |
| [CatLabel](../qml/CatLabel.qml) | 标准 Label，纯文本默认；可显式选择其他 textFormat |
| [CatPane](../qml/CatPane.qml) | 标准 Pane，Token 边框、圆角、padding |

## 状态与反馈

| 组件 | 主要 API / 行为 |
| --- | --- |
| [CatAvatar](../qml/CatAvatar.qml) | `name`, `initials`, `source`, `presence`；展示姓名缩写或圆形照片，并可显示在线状态。 |
| [CatInfoBadge](../qml/CatInfoBadge.qml) | `value`, `iconGlyph`, `severity`, `showZero`；展示数量、状态圆点或简洁符号。 |
| [CatProgressRing](../qml/CatProgressRing.qml) | `isActive`, `indeterminate`, `minimum`, `maximum`, `value`, `strokeWidth`；在加载圆弧与确定进度之间切换。 |
| [CatShimmer](../qml/CatShimmer.qml) | `active`, `cycleDuration`, `shimmerProgress`；展示加载骨架，在减少动效时保持静止。 |
| [CatSplashScreen](../qml/CatSplashScreen.qml) | `title`, `subtitle`, `iconSource`, `progress`, `indeterminate`；使用品牌图、启动说明及进度覆盖窗口内容。 |
| [CatToast](../qml/CatToast.qml) | `title`, `message`, `severity`, `duration`, `actionText`, `pauseOnHoverEnabled`；展示带操作的定时通知，并可在悬停时暂停计时。 |
| [CatAlert](../qml/CatAlert.qml) | `text`, `severity`（0–3）, `closable`；关闭时隐藏自身并发出 `closed()` |
| [CatBusyIndicator](../qml/CatBusyIndicator.qml) | `running`；按实际帧间隔以 900 ms 一圈旋转，不可见或减少动效时停止，恢复时不计入停止期间的时间；圆弧优先使用 CurveRenderer 抗锯齿 |
| [CatProgressBar](../qml/CatProgressBar.qml) | 标准 ProgressBar；from/to/value 和 indeterminate；镜像布局从右向左填充 |
| [CatCircularProgress](../qml/CatCircularProgress.qml) | `from=0`, `to=100`, `value=0`, `strokeWidth=6`；`position` 钳制为 0–1，零跨度为 0；GPU 后端优先使用内置抗锯齿的 CurveRenderer |
| [CatToolTip](../qml/CatToolTip.qml) | 标准 ToolTip，delay=600ms、timeout=3000ms；应用显式绑定 hovered 或其他可见条件 |

## 导航与浮层

| 组件 | 主要 API / 行为 |
| --- | --- |
| [CatBreadcrumb](../qml/CatBreadcrumb.qml) | `model`, `autoTruncateOnItemClick`, `itemClicked()`；沿路径导航，并可在选择后截短路径。 |
| [CatNavigationView](../qml/CatNavigationView.qml) | `model`, `selectedIndex`, `paneOpen`, `expandedPaneWidth`, `compactModeThresholdWidth`, `compact`, `page`, `activated()`；组合导航与页面内容，在紧凑布局下使用抽屉。 |
| [CatPivot](../qml/CatPivot.qml) | 使用带活动下划线的文字选项切换区块。 |
| [CatSelectorBar](../qml/CatSelectorBar.qml) | `model`, `selectedIndex`, `itemCount`, `selectionRequested()`；从一组带标签的选项中选择一个区块。 |
| [CatStepper](../qml/CatStepper.qml) | `model`, `currentIndex`, `orientation`, `stepRequested()`；请求切换步骤，由应用决定流程状态。 |
| [CatTabView](../qml/CatTabView.qml) | `titles`, `selectedIndex`, `tabsClosable`, `addTabButtonVisible`, `pages`, `tabCount`, `closeRequested()`；切换页面时保留状态，并向应用请求新增、关闭页签。 |
| [CatPagination](../qml/CatPagination.qml) | `total`, `pageSize`, `currentPage`, `pageCount`；`requestPage()` 钳制且仅实际变页发出 `pageRequested`；总数变小会收敛当前页但不发用户请求事件；空数据页数为 1 |
| [CatCalendar](../qml/CatCalendar.qml) | `selectedDate`, `month`（0–11）, `year`, `minimumDate`, `maximumDate`；`moveMonth()`、`selectDate()`；只接受范围内日期并发 `dateSelected` |
| [CatPopup](../qml/CatPopup.qml) | 标准 Popup；默认模态、focus=true，Escape 或外部按下关闭 |
| [CatDrawer](../qml/CatDrawer.qml) | 标准 Drawer；默认左侧，挂载到窗口 Overlay；开关过渡更新 `position`（关闭 0／打开 1），减少动效时立即完成；触摸打开手势采用 Qt 行为 |

## 数据与对话

| 组件 | 主要 API / 行为 |
| --- | --- |
| [CatTableView](../qml/CatTableView.qml) | `model`, `delegate`, `columnWidthProvider`, `rowHeightProvider`, `resizableColumns`；`view` 可配置标准 TableView 选择与滚动属性 |
| [CatChatView](../qml/CatChatView.qml) | 标准 ListView；`currentUser`、`followTail=true`；支持替换 delegate，关闭尾部跟随后可持续浏览历史。 |
| [CatChatInput](../qml/CatChatInput.qml) | `text`、`placeholderText`、`submit()`、`submitted(string)`；忽略纯空白，提交去除首尾空白后清空输入。 |

## 动画与文本

| 组件 | 主要 API / 行为 |
| --- | --- |
| [CatFlipView](../qml/CatFlipView.qml) | 标准 Flipable；`flipped`，front/back 为调用方内容 |
| [CatMarquee](../qml/CatMarquee.qml) | `text`, `font`, `running`, `duration`；文本溢出才滚动 |
| [CatTypewriter](../qml/CatTypewriter.qml) | `fullText`, `running`, `interval`, `revealed`, `restart()`；减少动效时直接显示全文 |
| [CatImageMotion](../qml/CatImageMotion.qml) | `source`, `sourceSize`, `running`, `duration`, `fromScale/toScale`, `fromOpacity/toOpacity`, `fromOffset/toOffset`；往返动画，应用选择移动／缩放／闪烁参数 |

## 图像与效果

| 组件 | 主要 API / 行为 |
| --- | --- |
| [CatImage](../qml/CatImage.qml) | 标准 Image；异步加载与等比填充，保留 sourceSize 等标准属性 |
| [CatConicalGradient](../qml/CatConicalGradient.qml) | `startColor`, `endColor`, `angle`；矩形区域锥形渐变 |
| [CatEffect](../qml/CatEffect.qml) | 标准 MultiEffect 的 source / mask / shadow；应用按 Qt MultiEffect 约定隐藏源项、设置纹理源，核心控件无效果依赖 |

## 交互工具

| 组件 | 主要 API / 行为 |
| --- | --- |
| [CatFontIcon](../qml/CatFontIcon.qml) | `glyph`, `iconSize`；使用指定字体、字号及主题颜色显示字形。 |
| [CatDragHandle](../qml/CatDragHandle.qml) | `target`, `active`；为无 anchors 锁定位置的目标提供拖动区域 |
| [CatResizeHandle](../qml/CatResizeHandle.qml) | `target`, `minimumWidth/Height`, `maximumWidth/Height`；应用不要同时用 anchors 或 Layout 强制覆盖目标尺寸 |
| [CatFpsCounter](../qml/CatFpsCounter.qml) | `fps`；只表示 Quick 帧回调速率 |

## 集合视图

| 组件 | 主要 API / 行为 |
| --- | --- |
| [CatCarousel](../qml/CatCarousel.qml) | 滑动切换保留状态的页面，并可配合页码指示器。 |
| [CatFileListView](../qml/CatFileListView.qml) | `fileActivated()`；展示应用提供的文件名称及说明。 |
| [CatFlowView](../qml/CatFlowView.qml) | `model`, `textRole`, `currentIndex`, `itemWidth`, `itemHeight`, `activated()`；在可用宽度内自动换行展示可选条目。 |
| [CatGridView](../qml/CatGridView.qml) | `textRole`, `activated()`；在可滚动网格中浏览、选择条目。 |
| [CatListView](../qml/CatListView.qml) | `textRole`, `multiSelect`, `selectedIndexes`, `activated()`；浏览列表，支持键盘导航及多选。 |
| [CatSplitView](../qml/CatSplitView.qml) | 调整相邻面板大小，并保留最小尺寸限制。 |
| [CatStackView](../qml/CatStackView.qml) | 压入、弹出或替换页面，过渡遵循主题动效设置。 |
| [CatTimeline](../qml/CatTimeline.qml) | `model`；按顺序展示事件、说明及时间信息。 |
| [CatTreeView](../qml/CatTreeView.qml) | 展开、选择 Qt 数据模型提供的树形条目。 |

## 日期与时间

| 组件 | 主要 API / 行为 |
| --- | --- |
| [CatCalendarDatePicker](../qml/CatCalendarDatePicker.qml) | `selectedDate`, `minimumDate`, `maximumDate`, `placeholderText`, `displayFormat`, `calendarOpen`, `dateSelected()`；通过日历选择日期，并限制可选日期范围。 |
| [CatDatePicker](../qml/CatDatePicker.qml) | `selectedDate`, `minimumDate`, `maximumDate`, `placeholderText`, `dateSelected()`；通过月、日、年编辑器确认范围内的日期。 |
| [CatTimePicker](../qml/CatTimePicker.qml) | `selectedTime`, `minuteIncrement`, `placeholderText`, `timeSelected()`；确认小时、分钟，并可配置分钟步长。 |

## 对话框与引导

| 组件 | 主要 API / 行为 |
| --- | --- |
| [CatCoachMark](../qml/CatCoachMark.qml) | 用模态引导浮层突出指定操作。 |
| [CatContentDialog](../qml/CatContentDialog.qml) | `primaryButtonText`, `secondaryButtonText`, `closeButtonText`；展示内容，并配置主要、次要及关闭操作。 |
| [CatDialog](../qml/CatDialog.qml) | 使用统一主题的模态对话框及 Qt 标准确认、取消按钮。 |
| [CatFlyout](../qml/CatFlyout.qml) | `target`, `placement`, `placementMargin`；在目标旁展示上下文内容，并方便关闭。 |
| [CatTeachingTip](../qml/CatTeachingTip.qml) | `title`, `text`, `actionText`；在可关闭的锚定浮层中解释指定操作。 |

## 布局

| 组件 | 主要 API / 行为 |
| --- | --- |
| [CatAccordion](../qml/CatAccordion.qml) | `singleExpanded`, `count`；协调折叠区，支持同时展开一个或多个区块。 |
| [CatDivider](../qml/CatDivider.qml) | `orientation`, `thickness`, `leadingInset`, `trailingInset`；使用水平或垂直线划分区块。 |
| [CatExpander](../qml/CatExpander.qml) | `headerText`, `expanded`, `body`；展开标题下的内容，保留内部控件状态。 |
| [CatField](../qml/CatField.qml) | `labelText`, `required`, `helperText`, `validationMessage`, `invalid`, `editor`；组合标签、编辑器、辅助说明和校验信息。 |
| [CatParticleBackdrop](../qml/CatParticleBackdrop.qml) | `animationEnabled`, `particleCount`, `speed`, `phase`；展示轻量主题粒子动画，并响应减少动效设置。 |

## 菜单与命令

| 组件 | 主要 API / 行为 |
| --- | --- |
| [CatCommandBar](../qml/CatCommandBar.qml) | `primaryActions`, `secondaryActions`；组织主要操作，并将次要操作放入更多菜单。 |
| [CatCommandBarFlyout](../qml/CatCommandBarFlyout.qml) | `actions`；在锚定浮层中展示上下文操作。 |
| [CatMenu](../qml/CatMenu.qml) | 组织操作、勾选项、分隔线及子菜单。 |
| [CatMenuBar](../qml/CatMenuBar.qml) | 通过鼠标及键盘访问应用菜单。 |
| [CatMenuBarItem](../qml/CatMenuBarItem.qml) | 在应用菜单栏中使用统一主题的入口。 |
| [CatMenuItem](../qml/CatMenuItem.qml) | 从菜单触发操作，或切换勾选项。 |
| [CatMenuSeparator](../qml/CatMenuSeparator.qml) | 用分隔线划分相关菜单操作。 |

## 滚动

| 组件 | 主要 API / 行为 |
| --- | --- |
| [CatAnnotatedScrollBar](../qml/CatAnnotatedScrollBar.qml) | `minimum`, `maximum`, `value`, `pageStep`, `annotations`, `annotationActivated()`；通过标签定位到滚动范围中的指定位置。 |
| [CatPipsPager](../qml/CatPipsPager.qml) | `numberOfPages`, `selectedPageIndex`, `maxVisiblePips`, `orientation`, `visiblePages`, `pageRequested()`；通过数量有限的可选指示点切换页面。 |
| [CatScrollBar](../qml/CatScrollBar.qml) | 拖动主题滑块，或将其附加到可滚动视图。 |
| [CatScrollView](../qml/CatScrollView.qml) | 支持鼠标按住拖动、滚轮、触摸滑动及主题水平／垂直滚动条；输入框内拖动仍用于选取文字。 |

## 空间

| 组件 | 主要 API / 行为 |
| --- | --- |
| [CatAttitudeIndicator](ATTITUDE.md) | 飞机姿态球；`pitchDegrees`、`rollDegrees`、`attitudeValid`；固定飞机标记、俯仰刻度、横滚刻度及无效状态。 |
| [CatTiltControlPanel](ATTITUDE.md) | 体感操作面板；状态和参数由应用绑定，校准、使能及参数编辑通过请求信号交接；按住控制、两轴预览、失活取消。 |
| [TiltController](ATTITUDE.md) | 设备无关 C++/QML 控制器；`submitOrientation()`、`calibrate()`、`engage()`、`disengage()`；四元数校准、滤波、死区、曲线、时效及有效杆量。 |

上述类型均不依赖 Qt Sensors 或 MAVLink；飞机遥测、手机采集和通信由应用接入，两套数据源独立。

[LedPreview](../qml/LedPreview.qml) / [LedSource](../qcatledsource.h) 提供 `source`、`viewMode`、`pointSize`、`previewBrightness`、`backgroundColor`、`showStatistics`、`resetView()`；使用 RHI 实例化灯点和独立 RGBA8 实时帧。详见 [LED 预览](LEDPREVIEW.md)。

公共 `LedDeck`、`LedMixer`、`LedStorageBrowser` 和 `LedLayoutSettings` / `LedPreviewSettings` / `LedOutputSettings` 提供独立播放、Storage 预设、共享布局与灯具配置交互。`LedProject` 可选装配多通道管线，支持 RGB、RGBW、RGB＋冷白＋暖白，并通过 `MemoryOutputBackend` 验证设备分支。完整接口和最小示例见 [LED 工作流](LEDWORKFLOW.md)。

| 组件 | 主要 API / 行为 |
| --- | --- |
| [CatSpatialItem](../qml/CatSpatialItem.qml) | `position`, `rotation3d`, `cameraDistance`, `spatialEnabled`, `spatialHost`, `effectiveCameraDistance`, `flatMode`；对实时界面进行空间变换，并提供平面渲染回退。 |
| [CatSpatialView](../qml/CatSpatialView.qml) | `spatialEnabled`, `pointerTrackingEnabled`, `cameraDistance`, `zoom`, `maximumTilt`, `forceFlat`, `flatMode`, `rendererName`, `surfaces`；组合可交互的面板，提供透视及指针倾斜效果。 |

## 窗口

| 组件 | 主要 API / 行为 |
| --- | --- |
| [CatTitleBar](../qml/CatTitleBar.qml) | `targetWindow`, `title`；移动窗口，并执行最小化、最大化及关闭操作。 |
| [CatWindow](../qml/CatWindow.qml) | `frameless`；用主题颜色承载内容，并可启用自定义标题栏。 |

## 组合与边界

- `CatCarousel` 用于滑动翻页，`CatFlipView` 用于内容正反面翻转。
- `CatNumberBox` 编辑小数；按 Enter 或失去焦点（Tab、点击其他控件）时提交修改并触发 `valueEdited`，编辑中的文本保留到提交。`acceptsExpression` 默认关闭，启用后只接受加减乘除、括号和一元正负号。无效输入保留文本和最近有效值并标记 invalid。整数使用 `CatSpinBox`，同样支持 Enter 或失焦提交。`CatColorPicker` 的合法十六进制颜色输入也支持 Enter 或失焦生效。
- `CatCircularProgress.showValue` 控制中心文字，`CatProgressRing` 提供无中心数值的进度环。
- `CatFileListView` 展示调用方模型，`CatFileDropZone` 提供拖放事件；文件访问和持久化由应用处理。
- `CatSpatialItem` / `CatSpatialView` 提供 Qt Quick 内容的透视表现，`forceFlat` 可选择平面显示。
- `CatFpsCounter` 表示 Quick 帧回调速率；Gallery 数据操作统计不等于该帧率。

键盘可通过 Tab 聚焦，标准按钮用 Space 激活。输入控件使用 Qt 的文字编辑和输入法机制。RTL 可通过 LayoutMirroring 配置；屏幕阅读器、输入法和系统窗口行为应在目标平台验证。

## 主题、模型与版本

| 类型 | 说明 |
| --- | --- |
| Theme / ThemePalette | 每个引擎的默认主题与可显式传入的局部主题，见 [主题](THEMING.md) |
| TableModel / ChatModel / TreeModel | 表格、消息和层次数据，见 [模型](MODELS.md) |
| QCatUiKit | 只读运行时版本单例，见 [版本管理](VERSIONING.md) |
| LibraryInfo | 与 QCatUiKit 提供相同版本属性的兼容入口 |

图表的数据格式、坐标和渲染边界见 [图表指南](CHARTS.md)。批量编辑、对话收发、输入提交及百万数据配置见 [Gallery 使用指南](../../../examples/uikit/gallery/README.md)。


## Dock 工作区

| 类型 | 接口与行为 |
| --- | --- |
| CatDockWorkspace | `controller`、`panels`、`theme`、`layoutLocked`、`dragging`、最低尺寸和 `sizeConstrained`；标题栏／扁平标签、随动缩略窗、方向指示与预览、跨窗口拖拽、浮动／回停 |
| CatDockPanel | `dockId`、`title`、`content`、最小尺寸、`closable`、`movable`、`floatable`、`closeRequested()`；内容实例保持 |
| DockController | 面板注册、组内／工作区外边缘停靠、浮动／回停、关闭／打开、激活、排序和版本化 JSON 布局；见 [Dock 工作区](DOCKING.md) |
