# 原生图表

从 QCatUiKit.Charts / QtNativeCharts 测试版迁入，使用同一个 `QCatUiKit` QML 模块、共享库、版本号与安装包。Qt 6.8+、C++17；不依赖 Qt Charts、Qt Graphs、WebEngine、Chart.js 或 QuickJS，不使用 QQuickPaintedItem。原有 MIT 源码许可及归属保存在 LICENSES/QtNativeCharts-MIT.txt。

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

- `setViewRange` 同时指定 X/Y；`zoomAt(factor,position)` 以图表内锚点缩放；`panBy(pixels)` 平移；`resetView` 恢复自动跟随。新数据保留手动范围。
- 滚轮缩放，左键拖动平移，双击重置；Tab 可聚焦，+/− 缩放，方向键平移，Home 重置。`interactive=false` 停用这些输入。
- `mapToPosition`／`mapToValue` 进行笛卡尔转换；`plotRect` 返回绘图区。`dataAt` 返回 seriesIndex/index/seriesName/x/y/value/label，折线查询半径为 18 px，返回原始点，不返回采样伪造值。
- `pointHovered`／`pointClicked` 用于应用详情；tooltipsEnabled/crosshairEnabled/legendVisible/gridVisible 可配置。图例点击切换系列或扇区；扇区图例最多 64 项。图例空间不足时截断，更多条目用外部列表管理。

库没有图形过渡动画；reducedMotion 不会停止应用主动追加数据。保留测试版的桌面交互边界，未新增双指触摸手势、多 Y 轴、堆叠面积或插件系统；径向图不支持笛卡尔缩放／平移。

## 渲染与验证

数据与样式变化先 polish 合并，GUI 线程准备图元，渲染线程创建 SceneGraph 几何及文字纹理。硬件后端的曲线与圆形使用覆盖率边缘，默认 `antialiasing: true`，不要求窗口开启 MSAA；文字仍是按内容缓存的小纹理。Qt Quick software 后端用 QPainter 将准备好的图元绘制为图像后显示，此回退用于可用性，性能不能代表硬件后端。该路径会创建整张图表大小的图像。

`preparationNanoseconds()` 仅为 CPU 数据／布局／几何准备时间，不能当作 GPU 帧耗时或 FPS。原测试版的独立窗口基准结果不能直接作为当前 gallery 的性能结果。

自动测试在 tests/auto/charts、tests/auto/qml/data/tst_charts.qml、tests/auto/gallery/data/tst_charts.qml。安装后独立测试：

```sh
cmake --install build --prefix stage
cmake -S tests/auto/cmake/charts-consumer -B build-charts-consumer \
  -DCMAKE_PREFIX_PATH="/absolute/stage;/path/to/Qt"
cmake --build build-charts-consumer
# Windows 同时把 stage/bin 与 Qt/bin 放入 PATH。
./build-charts-consumer/charts-consumer /absolute/stage/lib/qml
```

本地验证记录及平台边界见 [VALIDATION.md](VALIDATION.md)。
