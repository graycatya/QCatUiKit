# 原生图表

[文档索引](README.md) · [Gallery 使用指南](../../../examples/uikit/gallery/README.md)

QCatUiKit 图表使用 Qt Quick SceneGraph，提供图表容器、坐标轴和数据序列三个类型，随共享库和 QML 模块一起安装。支持 Qt 6.8+、C++17，无需 Qt Charts、Qt Graphs 或 WebEngine。

![中文桌面图表预览](images/gallery-charts-desktop.png)

[中文暗色窄屏预览](images/gallery-charts-mobile.png)

## 入口

| 类型 | C++ 类 | 公开头 |
| --- | --- | --- |
| CatChart | QCatUiKit::ChartItem | QCatUiKit/qcatchartitem.h 或 QCatUiKit/ChartItem |
| CatChartAxis | QCatUiKit::ChartAxis | QCatUiKit/qcatchartaxis.h 或 QCatUiKit/ChartAxis |
| CatChartSeries | QCatUiKit::ChartSeries | QCatUiKit/qcatchartseries.h 或 QCatUiKit/ChartSeries |

```qml
import QtQuick
import QCatUiKit

CatChart {
    width: 640; height: 360
    title: qsTr("每周订单")
    chartType: CatChartSeries.Bar
    xAxis.scale: CatChartAxis.Category
    xAxis.categories: [qsTr("周一"), qsTr("周二"), qsTr("周三")]
    CatChartSeries {
        id: orders
        name: qsTr("订单")
        Component.onCompleted: setData([{x: 0, y: 12}, {x: 1, y: 18}, {x: 2, y: 9}])
    }
}
```

C++ 调用方链接 `QCatUiKit::QCatUiKit`。可以直接将 ChartItem 加到 QQuickWindow 的 contentItem，数据与计算不需要 QML 引擎。

```cpp
#include <QCatUiKit/ChartItem>

auto *chart = new QCatUiKit::ChartItem(window.contentItem());
chart->setSize({640, 360});
auto *series = new QCatUiKit::ChartSeries(chart);
series->setName(QStringLiteral("Signal"));
chart->addSeries(series);
series->appendSamples({{0, 2, 1, {}}, {1, 4, 1, {}}});
```

## 数据与图表类型

| ChartSeries::Type | 数据解释 |
| --- | --- |
| Inherit = −1 | 使用图表 chartType，序列默认值 |
| Line = 0 | XY 折线；保留可见范围峰谷 |
| Area = 1 | XY 面积填充至零线；多系列重叠，不是堆叠面积 |
| Bar = 2 | 分组柱状；X 为位置，Y 为高度；组宽 0.8 X 单位 |
| StackedBar = 3 | 同一 X 的正值、负值分别累计 |
| Scatter = 4 | XY 散点 |
| Bubble = 5 | XY 位置；value 为逻辑像素半径，限制为 1–64 |
| Pie = 6 / Doughnut = 7 | 第一条可见系列；Y 为非负权重，非正值忽略；label 为扇区名 |
| Radar = 8 | 索引对应维度，最多 32 维；非负值，标签取 xAxis.categories |
| Heatmap = 9 | XY 为单元中心，value 为强度；cellWidth/cellHeight 为数据坐标尺寸 |

单条序列可覆盖笛卡尔图表的默认 type，例如柱状与折线、散点与拟合线。径向类型由图表 chartType 决定，不支持与笛卡尔类型混合。热力图自动统计强度范围并显示色条；数据应由应用准备，不包含音频采集、解码或 FFT。

序列默认容量 100,000，满后覆盖最旧点。QML 提供 `append(x,y,value)`、`appendData([{x,y,value,label}])`、`setData(...)`、`at(index)` 和 `clear()`；value/label 可省略。C++ 还提供 `appendSamples`、`replaceSamples`、`replacePoints`、`sampleAt`、`sortedX`、`lowerBound`。包含 NaN、Infinity 或缺少坐标的批次整批拒绝，保留原数据。数据可以无序；有序时可二分定位。

所有对象 API 在其所属线程调用。工作线程将完整批次通过 queued connection 提交；追加／替换在其他线程调用会返回 false。`sampleAt` 的引用不能跨修改保留。`addSeries` 不转移 QObject 所有权；C++ 调用方管理系列，或将图表作为父对象。移除、清空和销毁系列会断开连接。

`displayBudget` 默认每条曲线 2,048，0 表示全部可见原始点，其他有效值至少为 8。只对折线／面积做峰谷采样，不删原始数据。柱状、散点、气泡、雷达和热力图保持原始语义；大量散点或单元格应由应用控制数据量。

## 主题与交互

QML 中 theme 默认指向全局 Theme，也可设置局部 ThemePalette；纯 C++ 默认使用内部的 ThemePalette。颜色、字体、图例、网格、提示与坐标文字随主题变化。未设置序列 color 时使用主题调色板；backgroundColor、textColor 和 font 可显式覆盖，并可 reset 恢复主题值。外部主题销毁时安全回退到内部主题；可重新设置 `theme: null` 恢复 QML 全局主题。系列颜色与扇区标签根据主题调整，数值轴始终保持原始数值方向。

xAxis/yAxis 支持 Linear、Logarithmic、DateTime、Category。DateTime 坐标为 UTC Unix 毫秒；Category 坐标从 0 开始。对数轴要求正范围并跳过非正样本。自动范围考虑可见系列以及正负堆叠累计值。`setRange(min,max)` 切换到手动范围；`automatic=true` 恢复自动。

数值刻度按当前区域设置显示完整数字，例如中英文的 `1,000,000`；小数位随刻度间距调整，放大百万级数值附近的小范围时仍能区分相邻刻度。Y 轴按标签宽度预留空间，X 轴在空间不足时减少显示的标签，保留刻度与网格。超过 12 位小数的极小间距仍使用科学计数法，避免非零数显示成零。

- `setViewRange` 同时指定 X/Y；`zoomAt(factor,position)` 以图表内锚点缩放；`panBy(pixels)` 平移；`resetView` 恢复自动跟随。新数据保留手动范围。
- 滚轮缩放，左键拖动平移，双击重置；Tab 可聚焦，+/− 缩放，方向键平移，Home 重置。`interactive=false` 停用这些输入。
- 笛卡尔图表从绘图区开始拖动时保持鼠标捕获，嵌入 Flickable／ScrollView 时不会被父容器中途抢走。松开、取消或关闭交互会清理拖拽状态；径向图和绘图区外仍允许父容器滚动。
- `mapToPosition`／`mapToValue` 进行笛卡尔转换；`plotRect` 返回绘图区。`dataAt` 返回 seriesIndex/index/seriesName/x/y/value/label，折线查询半径为 18 px，返回原始点，不返回采样伪造值。
- `pointHovered`／`pointClicked` 用于应用详情；tooltipsEnabled/crosshairEnabled/legendVisible/gridVisible 可配置。图例点击切换系列或扇区；扇区图例最多 64 项。图例空间不足时截断，更多条目用外部列表管理。

库没有图形过渡动画；reducedMotion 不会停止应用主动追加数据。当前交互为鼠标和键盘操作，不提供双指触摸手势、多 Y 轴、堆叠面积或插件系统；径向图不支持笛卡尔缩放／平移。

## Gallery 数据操作与压力测试

图表、图表序列和坐标轴页面中的 15 个演示均提供添加一批、删除最新一批、清空和恢复演示数据。每批数量、数据量上限与更新间隔可以调整；达到上限时覆盖最旧数据。普通 XY 图按每系列点数计数，分类图同步增删分类，饼图／环形图按扇区、雷达图按维度、热力图按整列（40 个单元格）操作。散点与趋势线演示在数据变化后重新计算最小二乘拟合。

各类型都可以配置百万级原始数据：每系列最多 1,000,000 条，热力图最多 25,000 列（共 1,000,000 个单元格）。每批最多每系列 50,000 条，热力图最多 1,250 列。工具栏的“百万数据预设”按整张图合计 1,000,000 条源数据设置容量、每批 50,000 条总源数据和 200 ms 间隔；双系列各占一半，趋势图另含 2 个拟合端点。预设不立即填充数据，点击“开始压力测试”后逐步增长，达到容量后覆盖最旧记录。

设置较大容量时，Gallery 将原始数据保存在独立缓冲中，以有限数量的数据点显示全量概览。曲线每系列最多 512 点，保留各组极值及首尾点；散点／趋势散点等距选取最多 512 条原始记录，气泡最多 128 条；分类图将相邻记录分组求和，正负堆叠分别保留正值和负值；饼图／环形图聚合为最多 64 个扇区；雷达图按相邻维度分组取平均值，最多 32 组；热力图按相邻列逐行取均值，最多 128×40 个预览单元格。趋势线始终根据全部原始观测记录拟合。界面明确显示原始数量、预览数量和概览方式，当前概览不随缩放重新细分；百万数据压力测试不等于同时绘制百万图元或已达到特定 FPS。

原始数据容量与概览容量相互独立，增删、覆盖和缩小容量操作原始缓冲；`GalleryChartData::sourceAt(series, index)` 可读取保留的原始记录。预览不修改原始记录，切换语言不会重建源数据。雷达图少于 3 个维度时显示提示。修改饼图／环形图数据会恢复扇区显示，以免索引变化后沿用错误的隐藏状态。增删保留手动缩放范围；“重置视图”恢复自动范围，“恢复演示数据”恢复初始数据和批次／容量设置，并释放大数据缓冲。

压力测试由用户启动，循环添加 4 批、删除 1 批，持续运行直到手动停止。运行时禁用手动增删和参数编辑；停止后保留当前数据与统计，再次启动会重置统计。切换图表、恢复演示数据、离开页面、隐藏或最小化窗口时停止运行，不会自动恢复。

统计包括实际运行时长、更新次数／频率、添加量、删除及容量覆盖量、最慢数据批次耗时。添加与删除／覆盖量是本次运行的累计操作量，可以超过当前容量；各项计数按当前语言显示带千位分隔符的完整整数，不使用科学计数法。拟合端点计入总数据点数，不计入源数据增删量。最慢批次涵盖数据生成、修改与相关元数据、拟合及概览计算，不包含后续图表渲染；这些数字不是 FPS、GPU 帧耗时或独立性能基准。数据生成和压力测试由 Gallery 应用实现，不增加已安装图表库的公开 API。

## 渲染与验证

数据与样式变化先 polish 合并，GUI 线程准备图元，渲染线程创建 SceneGraph 几何及文字纹理。硬件后端的曲线与圆形使用覆盖率边缘，默认 `antialiasing: true`，不要求窗口开启 MSAA；Gallery 的百万数据概览和持续压力测试也保留该默认设置。文字仍是按内容缓存的小纹理。Qt Quick software 后端用 QPainter 将准备好的图元绘制为图像后显示，并遵循图表的反走样设置；此回退用于可用性，性能不能代表硬件后端。该路径会创建整张图表大小的图像。

`preparationNanoseconds()` 仅为 CPU 数据／布局／几何准备时间，不能当作 GPU 帧耗时或 FPS。性能测量应记录数据量、绘制量、设备、构建模式和图形后端。

自动测试位于 tests/auto/charts、tests/auto/qml/data/tst_charts.qml、tests/auto/gallery/data/tst_charts.qml 和 tests/auto/gallery/tst_gallerychartdata.cpp。安装消费者、测试命令与平台覆盖见 [验证说明](VALIDATION.md)。
