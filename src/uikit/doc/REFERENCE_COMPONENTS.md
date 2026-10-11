# Fluent-Qt 非 Charts 组件对照

按参考项目 `app/model/GalleryComponentCatalog.cpp` 的组件目录逐项整理，共 **78 种非 Charts 组件**，包含两个可选 Spatial 组件和 FontIcon。每一项都有 QCatUiKit 的 Qt Quick 实现及可操作的 gallery 页面。现有组件继续保留，另加入整数步进框和菜单配套类型，非 Charts 对齐阶段包含 **98 个 Cat 前缀 QML 组件**。之后另从原生图表测试项目迁入 3 个公开类型，当前总计 101 个类型、16 个 gallery 分类；图表不属于此表的 Fluent-Qt 对照范围。

此表对齐组件种类与主要使用场景。API 按 QML 的属性、信号、模型和内容组合方式设计；没有复制 Widgets 的 C++ 类层次、对象所有权或二进制接口。详细公开契约见 [COMPONENTS.md](COMPONENTS.md)。实现未复制 Fluent-Qt 的代码、字体或图标素材。

## 基础输入

| Fluent-Qt | QCatUiKit | 实现入口 |
| --- | --- | --- |
| Button | CatButton | [QML](../qml/CatButton.qml) |
| CompoundButton | CatCompoundButton | [QML](../qml/CatCompoundButton.qml) |
| CheckBox | CatCheckBox | [QML](../qml/CatCheckBox.qml) |
| ColorPicker | CatColorPicker | [QML](../qml/CatColorPicker.qml) |
| ComboBox | CatComboBox | [QML](../qml/CatComboBox.qml) |
| DropDownButton | CatDropDownButton | [QML](../qml/CatDropDownButton.qml) |
| FileDropZone | CatFileDropZone | [QML](../qml/CatFileDropZone.qml) |
| HyperlinkButton | CatHyperlinkButton | [QML](../qml/CatHyperlinkButton.qml) |
| MultiSelectComboBox | CatMultiSelectComboBox | [QML](../qml/CatMultiSelectComboBox.qml) |
| RadioButton | CatRadioButton | [QML](../qml/CatRadioButton.qml) |
| RatingControl | CatRatingControl | [QML](../qml/CatRatingControl.qml) |
| RepeatButton | CatRepeatButton | [QML](../qml/CatRepeatButton.qml) |
| Slider | CatSlider | [QML](../qml/CatSlider.qml) |
| SplitButton | CatSplitButton | [QML](../qml/CatSplitButton.qml) |
| ToggleButton | CatToggleButton | [QML](../qml/CatToggleButton.qml) |
| ToggleSplitButton | CatToggleSplitButton | [QML](../qml/CatToggleSplitButton.qml) |
| ToggleSwitch | CatSwitch | [QML](../qml/CatSwitch.qml) |

## 集合视图

| Fluent-Qt | QCatUiKit | 实现入口 |
| --- | --- | --- |
| DataGrid | CatTableView | [QML](../qml/CatTableView.qml) |
| DrawerView | CatDrawer | [QML](../qml/CatDrawer.qml) |
| FileListView | CatFileListView | [QML](../qml/CatFileListView.qml) |
| FlipView | CatCarousel | [QML](../qml/CatCarousel.qml) |
| FlowView | CatFlowView | [QML](../qml/CatFlowView.qml) |
| GridView | CatGridView | [QML](../qml/CatGridView.qml) |
| ListView | CatListView | [QML](../qml/CatListView.qml) |
| Timeline | CatTimeline | [QML](../qml/CatTimeline.qml) |
| SplitView | CatSplitView | [QML](../qml/CatSplitView.qml) |
| StackView | CatStackView | [QML](../qml/CatStackView.qml) |
| TreeView | CatTreeView | [QML](../qml/CatTreeView.qml) |

## 日期与时间

| Fluent-Qt | QCatUiKit | 实现入口 |
| --- | --- | --- |
| CalendarDatePicker | CatCalendarDatePicker | [QML](../qml/CatCalendarDatePicker.qml) |
| CalendarView | CatCalendar | [QML](../qml/CatCalendar.qml) |
| DatePicker | CatDatePicker | [QML](../qml/CatDatePicker.qml) |
| TimePicker | CatTimePicker | [QML](../qml/CatTimePicker.qml) |

## 对话框与浮层

| Fluent-Qt | QCatUiKit | 实现入口 |
| --- | --- | --- |
| ContentDialog | CatContentDialog | [QML](../qml/CatContentDialog.qml) |
| Dialog | CatDialog | [QML](../qml/CatDialog.qml) |
| Flyout | CatFlyout | [QML](../qml/CatFlyout.qml) |
| Popup | CatPopup | [QML](../qml/CatPopup.qml) |
| TeachingTip | CatTeachingTip | [QML](../qml/CatTeachingTip.qml) |
| CoachMark | CatCoachMark | [QML](../qml/CatCoachMark.qml) |

## 布局

| Fluent-Qt | QCatUiKit | 实现入口 |
| --- | --- | --- |
| Accordion | CatAccordion | [QML](../qml/CatAccordion.qml) |
| Card | CatPane | [QML](../qml/CatPane.qml) |
| Divider | CatDivider | [QML](../qml/CatDivider.qml) |
| Expander | CatExpander | [QML](../qml/CatExpander.qml) |
| Field | CatField | [QML](../qml/CatField.qml) |
| ParticleBackdrop | CatParticleBackdrop | [QML](../qml/CatParticleBackdrop.qml) |

## 空间视图

| Fluent-Qt | QCatUiKit | 实现入口 |
| --- | --- | --- |
| SpatialView | CatSpatialView | [QML](../qml/CatSpatialView.qml) |
| SpatialItem | CatSpatialItem | [QML](../qml/CatSpatialItem.qml) |

## 菜单与命令

| Fluent-Qt | QCatUiKit | 实现入口 |
| --- | --- | --- |
| Menu | CatMenu | [QML](../qml/CatMenu.qml) |
| MenuBar | CatMenuBar | [QML](../qml/CatMenuBar.qml) |
| CommandBar | CatCommandBar | [QML](../qml/CatCommandBar.qml) |
| CommandBarFlyout | CatCommandBarFlyout | [QML](../qml/CatCommandBarFlyout.qml) |

## 导航

| Fluent-Qt | QCatUiKit | 实现入口 |
| --- | --- | --- |
| Breadcrumb | CatBreadcrumb | [QML](../qml/CatBreadcrumb.qml) |
| NavigationView | CatNavigationView | [QML](../qml/CatNavigationView.qml) |
| Pivot | CatPivot | [QML](../qml/CatPivot.qml) |
| SelectorBar | CatSelectorBar | [QML](../qml/CatSelectorBar.qml) |
| Stepper | CatStepper | [QML](../qml/CatStepper.qml) |
| TabView | CatTabView | [QML](../qml/CatTabView.qml) |

## 滚动

| Fluent-Qt | QCatUiKit | 实现入口 |
| --- | --- | --- |
| AnnotatedScrollBar | CatAnnotatedScrollBar | [QML](../qml/CatAnnotatedScrollBar.qml) |
| PipsPager | CatPipsPager | [QML](../qml/CatPipsPager.qml) |
| ScrollBar | CatScrollBar | [QML](../qml/CatScrollBar.qml) |
| ScrollView | CatScrollView | [QML](../qml/CatScrollView.qml) |

## 状态与信息

| Fluent-Qt | QCatUiKit | 实现入口 |
| --- | --- | --- |
| Avatar | CatAvatar | [QML](../qml/CatAvatar.qml) |
| InfoBadge | CatInfoBadge | [QML](../qml/CatInfoBadge.qml) |
| InfoBar | CatAlert | [QML](../qml/CatAlert.qml) |
| ProgressBar | CatProgressBar | [QML](../qml/CatProgressBar.qml) |
| ProgressRing | CatProgressRing | [QML](../qml/CatProgressRing.qml) |
| SplashScreen | CatSplashScreen | [QML](../qml/CatSplashScreen.qml) |
| Shimmer | CatShimmer | [QML](../qml/CatShimmer.qml) |
| Toast | CatToast | [QML](../qml/CatToast.qml) |
| ToolTip | CatToolTip | [QML](../qml/CatToolTip.qml) |

## 文本输入

| Fluent-Qt | QCatUiKit | 实现入口 |
| --- | --- | --- |
| AutoSuggestBox | CatAutoSuggestBox | [QML](../qml/CatAutoSuggestBox.qml) |
| Label | CatLabel | [QML](../qml/CatLabel.qml) |
| LineEdit | CatTextField | [QML](../qml/CatTextField.qml) |
| NumberBox | CatNumberBox | [QML](../qml/CatNumberBox.qml) |
| PasswordBox | CatPasswordField | [QML](../qml/CatPasswordField.qml) |
| TextEdit | CatTextArea | [QML](../qml/CatTextArea.qml) |

## 窗口

| Fluent-Qt | QCatUiKit | 实现入口 |
| --- | --- | --- |
| TitleBar | CatTitleBar | [QML](../qml/CatTitleBar.qml) |
| Window | CatWindow | [QML](../qml/CatWindow.qml) |

## 基础图标

| Fluent-Qt | QCatUiKit | 实现入口 |
| --- | --- | --- |
| FontIcon | CatFontIcon | [QML](../qml/CatFontIcon.qml) |

## 使用差异

- `FlipView` 对应滑动翻页的 `CatCarousel`；原有 `CatFlipView` 保留正反面翻转行为。`NumberBox` 对应支持小数的 `CatNumberBox`，整数使用 `CatSpinBox`。密码与多行编辑入口分别是 `CatPasswordField`、`CatTextArea`。
- 集合视图接收应用提供的模型。树视图支持标准 Qt 层次模型，另提供 QML `TreeModel` 便于声明只读树；文件列表不隐式读取磁盘。文件拖放区发出 `browseRequested`，由应用选择原生文件对话框或其他浏览方式。
- `CatStepper` 请求变更步骤，`CatTabView` 请求新增或关闭页签；应用负责决定流程是否允许继续及创建、销毁内容。切换页签保留已创建页面的状态。
- 颜色选择器提供色板、RGB、透明度和十六进制编辑；日期编辑器按月份、日期、年份确认，时间编辑器使用 24 小时制。额外的颜色空间编辑和平台日期样式可由应用组合。
- Spatial 使用 Qt Quick 实时内容、矩阵透视及鼠标倾斜，不依赖 Qt Quick 3D。软件后端、减少动效或 `forceFlat` 使用平面回退，保留内容与交互；不提供 Widgets 嵌入或三维网格渲染。
- `CatWindow` 默认保留系统边框，可启用自定义标题栏、系统移动与八向缩放。未加入 Windows Mica/Acrylic 等原生背景能力；系统移动、缩放、任务栏及输入法仍需在目标平台实机验收。

验证范围和实际结果见 [VALIDATION.md](VALIDATION.md)。
