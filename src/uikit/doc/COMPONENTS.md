# 组件与公开契约

所有带主题的组件默认 `theme: Theme`。其 `tokens` 是解析视图。继承自 Qt 控件的类型保留其标准属性、信号和可访问性语义。未列出的内部 id、子对象结构以及 `SurfaceImpl` 不属于公开契约。

| 组件 | 主要 API / 行为 |
| --- | --- |
| CatButton | 标准 Button；`primary`, `danger`, `loading`, `iconSource`；loading 临时禁用并恢复 enabled；primary 决定填充，danger 决定危险边框/普通文字 |
| CatLabel | 标准 Label，纯文本默认；可显式选择其他 textFormat |
| CatPane | 标准 Pane，Token 边框、圆角、padding |
| CatTextField | 标准 TextField；`invalid` 控制错误边框；可配置 validator / inputMethodHints |
| CatIpAddressField | 单输入框 IPv4；使用 `acceptableInput` 判断可提交性；聚焦时允许中间输入状态 |
| CatToolTip | 标准 ToolTip，delay=600ms、timeout=3000ms；应用显式绑定 hovered 或其他可见条件 |
| CatPopup | 标准 Popup；默认模态、focus=true，Escape 或外部按下关闭 |
| CatDrawer | 标准 Drawer；默认左侧，挂载到窗口 Overlay；开关过渡更新 `position`（关闭 0／打开 1），减少动效时立即完成；触摸打开手势采用 Qt 行为 |
| CatAlert | `text`, `severity`（0–3）, `closable`；关闭时隐藏自身并发出 `closed()` |
| CatProgressBar | 标准 ProgressBar；from/to/value 和 indeterminate；镜像布局从右向左填充 |
| CatCircularProgress | `from=0`, `to=100`, `value=0`, `strokeWidth=6`；`position` 钳制为 0–1，零跨度为 0；GPU 后端优先使用内置抗锯齿的 CurveRenderer |
| CatBusyIndicator | `running`；按实际帧间隔以 900 ms 一圈旋转，不可见或减少动效时停止，恢复时不计入停止期间的时间；圆弧优先使用 CurveRenderer 抗锯齿 |
| CatPagination | `total`, `pageSize`, `currentPage`, `pageCount`；`requestPage()` 钳制且仅实际变页发出 `pageRequested`；总数变小会收敛当前页但不发用户请求事件；空数据页数为 1 |
| CatCalendar | `selectedDate`, `month`（0–11）, `year`, `minimumDate`, `maximumDate`；`moveMonth()`、`selectDate()`；只接受范围内日期并发 `dateSelected` |
| CatTableView | `model`, `delegate`, `columnWidthProvider`, `rowHeightProvider`, `resizableColumns`；`view` 可配置标准 TableView 选择与滚动属性 |
| CatChatView | 标准 ListView；`currentUser`, `followTail=true`，delegate 可替换；新增消息默认滚动到底部 |
| CatChatInput | `text`, `placeholderText`，`submitted(string)`；忽略纯空白，提交后清空 |
| CatImage | 标准 Image；异步加载与等比填充，保留 sourceSize 等标准属性 |
| CatImageMotion | `source`, `sourceSize`, `running`, `duration`, `fromScale/toScale`, `fromOpacity/toOpacity`, `fromOffset/toOffset`；往返动画，应用选择移动／缩放／闪烁参数 |
| CatFlipView | 标准 Flipable；`flipped`，front/back 为调用方内容 |
| CatMarquee | `text`, `font`, `running`, `duration`；文本溢出才滚动 |
| CatTypewriter | `fullText`, `running`, `interval`, `revealed`, `restart()`；减少动效时直接显示全文 |
| CatDragHandle | `target`, `active`；为无 anchors 锁定位置的目标提供拖动区域 |
| CatResizeHandle | `target`, `minimumWidth/Height`, `maximumWidth/Height`；应用不要同时用 anchors 或 Layout 强制覆盖目标尺寸 |
| CatFpsCounter | `fps`；只表示 Quick 帧回调速率 |
| CatEffect | 标准 MultiEffect 的 source / mask / shadow；应用按 Qt MultiEffect 约定隐藏源项、设置纹理源，核心控件无效果依赖 |
| CatConicalGradient | `startColor`, `endColor`, `angle`；矩形区域锥形渐变 |

## 基础输入与扩展组件

下面列出新增组件的主要入口；继承类型继续支持对应 Qt 控件的标准 API。完整的 78 项参考对应关系见 [组件对照](REFERENCE_COMPONENTS.md)。

| 组件 | 主要 API / 行为 |
| --- | --- |
| CatCheckBox | 标准 CheckBox；支持 `checked`、`tristate` 与 `checkState` |
| CatRadioButton | 标准 RadioButton；支持 Qt ButtonGroup 与排他选择 |
| CatSwitch | 标准 Switch；鼠标、触摸拖动和键盘切换 |
| CatSlider | 标准 Slider；范围、步长、水平／垂直方向及镜像布局 |
| CatComboBox | 标准 ComboBox；可编辑、textRole/valueRole、键盘导航与主题弹出列表 |
| CatAccordion | `singleExpanded`, `count`；协调折叠区，支持同时展开一个或多个区块。 |
| CatAnnotatedScrollBar | `minimum`, `maximum`, `value`, `pageStep`, `annotations`, `annotationActivated()`；通过标签定位到滚动范围中的指定位置。 |
| CatAutoSuggestBox | `suggestions`, `filteredSuggestions`, `isSuggestionListOpen`, `suggestionAccepted()`；输入时筛选建议，使用方向键或回车选择。 |
| CatAvatar | `name`, `initials`, `source`, `presence`；展示姓名缩写或圆形照片，并可显示在线状态。 |
| CatBreadcrumb | `model`, `autoTruncateOnItemClick`, `itemClicked()`；沿路径导航，并可在选择后截短路径。 |
| CatCalendarDatePicker | `selectedDate`, `minimumDate`, `maximumDate`, `placeholderText`, `displayFormat`, `calendarOpen`, `dateSelected()`；通过日历选择日期，并限制可选日期范围。 |
| CatCarousel | 滑动切换保留状态的页面，并可配合页码指示器。 |
| CatCoachMark | 用模态引导浮层突出指定操作。 |
| CatColorPicker | `selectedColor`, `alphaEnabled`, `colorSelected()`；选择预设颜色，或编辑 RGB、透明度和十六进制值。 |
| CatCommandBar | `primaryActions`, `secondaryActions`；组织主要操作，并将次要操作放入更多菜单。 |
| CatCommandBarFlyout | `actions`；在锚定浮层中展示上下文操作。 |
| CatCompoundButton | `secondaryText`；在操作标题下补充说明文字。 |
| CatContentDialog | `primaryButtonText`, `secondaryButtonText`, `closeButtonText`；展示内容，并配置主要、次要及关闭操作。 |
| CatDatePicker | `selectedDate`, `minimumDate`, `maximumDate`, `placeholderText`, `dateSelected()`；通过月、日、年编辑器确认范围内的日期。 |
| CatDialog | 使用统一主题的模态对话框及 Qt 标准确认、取消按钮。 |
| CatDivider | `orientation`, `thickness`, `leadingInset`, `trailingInset`；使用水平或垂直线划分区块。 |
| CatDropDownButton | `isOpen`；通过支持键盘操作的按钮打开菜单。 |
| CatExpander | `headerText`, `expanded`, `body`；展开标题下的内容，保留内部控件状态。 |
| CatField | `labelText`, `required`, `helperText`, `validationMessage`, `invalid`, `editor`；组合标签、编辑器、辅助说明和校验信息。 |
| CatFileDropZone | `title`, `description`, `hintText`, `browseText`, `errorMessage`, `acceptedExtensions`, `dragActive`, `filesDropped()`；接收符合筛选条件的本地文件拖放，并向应用请求文件浏览。 |
| CatFileListView | `fileActivated()`；展示应用提供的文件名称及说明。 |
| CatFlowView | `model`, `textRole`, `currentIndex`, `itemWidth`, `itemHeight`, `activated()`；在可用宽度内自动换行展示可选条目。 |
| CatFlyout | `target`, `placement`, `placementMargin`；在目标旁展示上下文内容，并方便关闭。 |
| CatFontIcon | `glyph`, `iconSize`；使用指定字体、字号及主题颜色显示字形。 |
| CatGridView | `textRole`, `activated()`；在可滚动网格中浏览、选择条目。 |
| CatHyperlinkButton | `url`, `showUnderline`；激活文字链接，并可打开指定网址。 |
| CatInfoBadge | `value`, `iconGlyph`, `severity`, `showZero`；展示数量、状态圆点或简洁符号。 |
| CatListView | `textRole`, `multiSelect`, `selectedIndexes`, `activated()`；浏览列表，支持键盘导航及多选。 |
| CatMenu | 组织操作、勾选项、分隔线及子菜单。 |
| CatMenuBar | 通过鼠标及键盘访问应用菜单。 |
| CatMenuBarItem | 在应用菜单栏中使用统一主题的入口。 |
| CatMenuItem | 从菜单触发操作，或切换勾选项。 |
| CatMenuSeparator | 用分隔线划分相关菜单操作。 |
| CatMultiSelectComboBox | `model`, `textRole`, `selectedIndexes`, `placeholderText`, `searchEnabled`, `selectedCount`, `selectedTexts`, `selectionEdited()`；选择多个选项，并可通过文字筛选。 |
| CatNavigationView | `model`, `selectedIndex`, `paneOpen`, `expandedPaneWidth`, `compactModeThresholdWidth`, `compact`, `page`, `activated()`；组合导航与页面内容，在紧凑布局下使用抽屉。 |
| CatNumberBox | `locale`, `value`, `minimum`, `maximum`, `smallChange`, `largeChange`, `acceptsExpression`, `decimals`, `valueEdited()`；编辑有范围限制的小数，支持步进、本地格式及算术表达式。 |
| CatParticleBackdrop | `animationEnabled`, `particleCount`, `speed`, `phase`；展示轻量主题粒子动画，并响应减少动效设置。 |
| CatPasswordField | `password`, `revealed`, `revealEnabled`；输入密码，并通过支持键盘操作的开关显示明文。 |
| CatPipsPager | `numberOfPages`, `selectedPageIndex`, `maxVisiblePips`, `orientation`, `visiblePages`, `pageRequested()`；通过数量有限的可选指示点切换页面。 |
| CatPivot | 使用带活动下划线的文字选项切换区块。 |
| CatProgressRing | `isActive`, `indeterminate`, `minimum`, `maximum`, `value`, `strokeWidth`；在加载圆弧与确定进度之间切换。 |
| CatRatingControl | `value`, `placeholderValue`, `maxRating`, `isReadOnly`, `isClearEnabled`, `caption`, `stepSize`, `ratingEdited()`；选择或清除评分，支持小数显示及键盘步进。 |
| CatRepeatButton | `delay`, `interval`；在按住鼠标或按键期间重复触发操作。 |
| CatScrollBar | 拖动主题滑块，或将其附加到可滚动视图。 |
| CatScrollView | 使用主题水平、垂直滚动条浏览应用提供的内容。 |
| CatSelectorBar | `model`, `selectedIndex`, `itemCount`, `selectionRequested()`；从一组带标签的选项中选择一个区块。 |
| CatShimmer | `active`, `cycleDuration`, `shimmerProgress`；展示加载骨架，在减少动效时保持静止。 |
| CatSpatialItem | `position`, `rotation3d`, `cameraDistance`, `spatialEnabled`, `spatialHost`, `effectiveCameraDistance`, `flatMode`；对实时界面进行空间变换，并提供平面渲染回退。 |
| CatSpatialView | `spatialEnabled`, `pointerTrackingEnabled`, `cameraDistance`, `zoom`, `maximumTilt`, `forceFlat`, `flatMode`, `rendererName`, `surfaces`；组合可交互的面板，提供透视及指针倾斜效果。 |
| CatSpinBox | 通过文字、按钮或键盘编辑范围内的整数。 |
| CatSplashScreen | `title`, `subtitle`, `iconSource`, `progress`, `indeterminate`；使用品牌图、启动说明及进度覆盖窗口内容。 |
| CatSplitButton | `text`, `iconSource`, `primary`, `checkable`, `checked`, `isOpen`；将主要操作与更多菜单操作分开。 |
| CatSplitView | 调整相邻面板大小，并保留最小尺寸限制。 |
| CatStackView | 压入、弹出或替换页面，过渡遵循主题动效设置。 |
| CatStepper | `model`, `currentIndex`, `orientation`, `stepRequested()`；请求切换步骤，由应用决定流程状态。 |
| CatTabView | `titles`, `selectedIndex`, `tabsClosable`, `addTabButtonVisible`, `pages`, `tabCount`, `closeRequested()`；切换页面时保留状态，并向应用请求新增、关闭页签。 |
| CatTeachingTip | `title`, `text`, `actionText`；在可关闭的锚定浮层中解释指定操作。 |
| CatTextArea | `invalid`；编辑、选择多行文本，并显示主题焦点及校验状态。 |
| CatTimePicker | `selectedTime`, `minuteIncrement`, `placeholderText`, `timeSelected()`；确认小时、分钟，并可配置分钟步长。 |
| CatTimeline | `model`；按顺序展示事件、说明及时间信息。 |
| CatTitleBar | `targetWindow`, `title`；移动窗口，并执行最小化、最大化及关闭操作。 |
| CatToast | `title`, `message`, `severity`, `duration`, `actionText`, `pauseOnHoverEnabled`；展示带操作的定时通知，并可在悬停时暂停计时。 |
| CatToggleButton | `threeState`, `checkState`；保持选中状态，或在三种选择状态之间循环。 |
| CatToggleSplitButton | 切换主要操作的选中状态，同时保留独立菜单。 |
| CatTreeView | 展开、选择 Qt 数据模型提供的树形条目。 |
| CatWindow | `frameless`；用主题颜色承载内容，并可启用自定义标题栏。 |

`CatCircularProgress.showValue` 控制中心数值文字；`CatProgressRing` 使用不带文字的小尺寸进度环。`CatNumberBox.acceptsExpression` 默认关闭；打开后接受加减乘除、括号和一元正负号，不执行 JavaScript 或函数调用。无效输入保留最后的有效值并设置 `invalid`，成功编辑后发出 `valueEdited`。

## 库身份

`QCatUiKit` 公开只读单例（旧名称 `LibraryInfo` 保留兼容）提供实际加载库的 `version`、`versionMajor/Minor/Patch`、`releaseChannel`、`isStable`、`isExperimental` 和 `abiVersion`。C++ 使用 `<QCatUiKit/Version.h>` 中的导出查询函数。详见 [版本管理](VERSIONING.md)。

## C++ / QML 模型

ChatModel 公开 role（从 `Qt::UserRole+1` 顺序分配）：`messageId`, `username`, `sender`, `recipient`, `timestamp`, `content`, `type`, `status`, `senderAvatar`, `recipientAvatar`。`type`：Text=0、Image=1、Audio=2、Video=3；`status`：Pending=0、Sent=1、Failed=2。缺省文本内容为空、时间戳为当前毫秒、状态 Pending。未知字段不加入消息；非法枚举使 append/update 返回 false。

ChatModel：`append(map)`, `update(row,map)`, `get(row)`, `remove(row)`, `clear()`, `count`。非法行返回 false/空 map，不发虚假的增删通知；update 只通知实际修改角色，clear 对空模型不发 reset。

TableModel：`columns`, `headers`, `count`, `append(map)`, `update(row,map)`, `get(row)`, `setCell(row,column,value)`, `remove(row)`, `clear()`。列不能为空字符串且不可重复；非法配置被忽略，空列列表合法但不可 append。改变列顺序保留行数据并 reset 模型。表头不足时使用列键。角色为 Qt 标准 `display` 和 `edit`，支持标准 `setData(EditRole)`。

TreeModel 是 QML 公开的只读树模型辅助类型，C++ 实现头保持私有。`items` 接受字符串叶子或 `{text, enabled, children}` 对象数组，`children` 使用相同结构；修改整个 `items` 会重建模型并发出 `itemsChanged`。角色遵循 Qt 的 `display` 与 `ItemIsEnabled` 标记，不通过数组元素原地修改来更新模型。

ChatModel 与 TableModel 都保留 Qt 的 `dataChanged`、rowsInserted/rowsRemoved、modelReset 等标准信号。`countChanged` 仅在条目数量变化时发出。get 返回副本，不通过外部对象指针暴露 Impl。

## 平台交互

桌面可以 Tab 聚焦、Space 激活按钮；触摸密度使用更大的命中区域，并默认关闭 hover 样式。输入控件使用 Qt 原生文本编辑与输入法。组合布局使用 Qt 镜像机制；应用可在根 Item 设置 `LayoutMirroring.enabled` 和 `childrenInherit`。实际屏幕阅读器、输入法和平台窗口行为应在目标设备验收。

## 原生图表

`CatChart`（C++ `ChartItem`）、`CatChartAxis`（`ChartAxis`）和 `CatChartSeries`（`ChartSeries`）均在 `QCatUiKit` 模块中导出。图表支持折线、面积、分组柱状、正负堆叠柱状、散点、气泡、饼图、环形、雷达及热力图，笛卡尔图表可按序列混合类型。轴支持线性、对数、UTC 时间和分类。

数据序列提供有限容量的环形缓冲，按批次验证、追加和替换；显示预算仅降采样折线／面积，不改原始数据。图表采用统一 Theme／ThemePalette、默认反走样、缩放／平移、数据提示、图例开关及键盘导航。详见 [图表文档](CHARTS.md)。
