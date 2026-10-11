# 从 QCatGrayQuick 迁移

本次迁移按能力重新设计。旧模块 URI、手动注册入口、别名拼写、内部对象访问和对象指针式模型接口不作为兼容约束。应用改为 `import QCatUiKit`，CMake 链接 `QCatUiKit::QCatUiKit`。

| 旧组件 / 模块 | QCatUiKit 对应能力 | 变化 |
| --- | --- | --- |
| QCatGrayQuickPlugins | 自动生成的 QML 插件 | 删除手动 Init、Debug 绝对 import 路径与 qrc 清单 |
| QCatGrayQuickStyle | Theme / ThemePalette | 全局单例 + 局部主题；Seed 与语义 Token |
| CatButton / CatButtonImage | CatButton | `primary`、`danger`、`loading`、`iconSource`；标准 clicked / checked / pressed |
| CatBasicsText | CatLabel | Token 字体与颜色，默认纯文本 |
| CatBasicsToolTip | CatToolTip | 标准延时、超时与内容定制 |
| CatRectangle / TemplateWidget | CatPane 或 QtQuick.Rectangle | 带主题容器；装饰图形使用 Qt 原生图元 |
| CatMessage | CatAlert | info/success/warning/error，closable 与 closed |
| CatTransArea | QtQuick.HoverHandler / TapHandler | 透明输入区域采用原生 Pointer Handler |
| GeneralInput | CatTextField | 标准文本输入、validator、invalid、输入法支持 |
| IpTextInput | CatIpAddressField | 0–255 IPv4 校验，不保留四个编辑框内部接口 |
| CatPopup | CatPopup | 标准模态／关闭策略／焦点 |
| CatSideColumn | CatDrawer | 原生抽屉的边缘拖动和遮罩协议 |
| CatCalendar + CalendarRelyon/* | CatCalendar | 合并内部日期项；范围校验、跨年、闰日、可访问日期标签 |
| CatSorter + CatSorter/* | CatPagination | total/pageSize/currentPage；有界页码与 pageRequested |
| BarTypeProgressBar | CatProgressBar | Qt 标准 from/to/value、RTL 和不确定进度 |
| ArcProgressBar / ArcProgressBarCanvas | CatCircularProgress | 一个矢量实现；0–100 默认范围，归一化、钳制与零跨度处理 |
| QCatGrayQuickChatModel / QCatGrayChatStruct | ChatModel | 消息值类型 map；append/update/get/remove/clear，安全索引与规范模型通知 |
| CatChatView / CatChatItemBase | CatChatView + 自定义 delegate | 消息列表虚拟化；默认显示文本与附件描述，可自行添加媒体展示 |
| CatChatTextEdit | CatChatInput | 去除空白后提交，Enter / Send；业务服务自行接收 submitted |
| QCatGrayQuickTableViewModel / *Struct | TableModel | QAbstractTableModel；显式列顺序，数据与尺寸分离 |
| CatTableView / *DelegateBase / *ItemBase | CatTableView + delegate / columnWidthProvider / rowHeightProvider | Qt TableView 虚拟化、同步表头、手动列宽；旧多区域冻结属性移除 |
| CatImageBase | CatImage | 异步加载、等比填充与加载指示 |
| ImageMove / ImageScale / ImageFlicker / ImageFlickerGradient | CatImageMotion | source + from/to Offset/Scale/Opacity，统一生命周期与 reducedMotion |
| ImageFlipable | CatFlipView | front/back 任意 Item，flipped 控制翻转 |
| Marquee | CatMarquee | 仅溢出时滚动，静态模式不滚动 |
| TextOneByOneShow | CatTypewriter | fullText、interval、restart；按 Unicode 码点逐字（不保证字素簇） |
| DragWidget | CatDragHandle | Pointer Handler 同时接收鼠标与触摸 |
| DragSizeWidget | CatResizeHandle | 右下角拖动与 min/max 尺寸约束；旧八方向句柄可由应用组合实现 |
| RotateCursor | CatBusyIndicator 或 HoverHandler.cursorShape | 忙碌指示和系统鼠标光标分离 |
| CatFps | CatFpsCounter | FrameAnimation 统计，仅可见时采样；不等价于 GPU 硬件计数器 |
| CatDropShadow / CatOpacityMask | CatEffect | QtQuick.Effects.MultiEffect 的 shadow / mask 属性，无 Qt5Compat |
| CatConicalGradient | CatConicalGradient | QtQuick.Shapes 原生 ConicalGradient |
| QCatGrayQuickQrenCodeQml | 移除 | 不迁移二维码生成及其示例、依赖 |
| lottie | 移除 | 不迁移 lottie 代码、资源与依赖 |
| Qt 5 / qmake 支持 | 移除 | 只维护 Qt 6 CMake 模块 |

## 模型示例

```qml
TableModel {
    id: rows
    columns: ["name", "status"]
    headers: ["名称", "状态"]
    Component.onCompleted: append({name: "设备 A", status: "在线"})
}
CatTableView { model: rows }

ChatModel { id: messages }
CatChatInput {
    onSubmitted: text => messages.append({
        messageId: "local-1", sender: "me", content: text,
        type: ChatModel.Text, status: ChatModel.Pending
    })
}
```

旧的 `getStruct()` 返回可变 QObject 指针；新的 `get()` 返回值副本，通过 `update()` 或 `setCell()` 修改数据以便正确通知视图。ChatModel `timestamp` 使用毫秒。表格列不会再由 JSON 键的排序意外决定。

新项目主协议为 Apache-2.0。旧项目 MIT 文本保留在 `LICENSES/MIT.txt`，并由 `NOTICE` 记录来源。

## 从 QCatUiKit.Charts 测试版迁移

图表已合入同一共享库与 QML 模块，不再单独构建 QtNativeCharts。C++ Namespace 从 `NativeCharts` 改为 `QCatUiKit`；类名 `ChartItem`／`ChartAxis`／`ChartSeries` 保留。小写公开头为 `qcatchartitem.h`／`qcatchartaxis.h`／`qcatchartseries.h`，也支持 `<QCatUiKit/ChartItem>` 等转发头。CMake 改链接 `QCatUiKit::QCatUiKit`。

| 测试版 | 当前入口 |
| --- | --- |
| `import QtNativeCharts` / `NativeChart` | `import QCatUiKit` / `CatChart` |
| QML `ChartAxis` / `ChartSeries` | `CatChartAxis` / `CatChartSeries` |
| `<nativecharts/chartitem.h>` | `<QCatUiKit/qcatchartitem.h>` |
| `QtNativeCharts::Charts` | `QCatUiKit::QCatUiKit` |
| 固定暗色背景、字体和图例颜色 | 全局 Theme 或局部 ThemePalette；显式颜色可覆盖 |
| 默认固定蓝色系列 | 未设置 color 时使用图表的主题调色板 |
| 仅硬件几何渲染 | 硬件 SceneGraph + 软件后端绘制回退 |
| 独立原生演示窗口 | 中英文 gallery 图表分类，保留 15 个示例 |

保留枚举值、批次校验、容量覆盖、采样、原始数据查询、缩放、平移和图例行为；`interactive=false` 同时停用鼠标点击与键盘导航。测试版模块名称和二进制不作为兼容 API，需重新编译原使用方。完整接口、数据格式与边界见 [CHARTS.md](CHARTS.md)。源项目保留原样，迁入代码保留 MIT 归属。
