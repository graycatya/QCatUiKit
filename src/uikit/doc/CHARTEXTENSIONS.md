# 扩展图表

[文档索引](README.md) · [图表交互](CHARTS.md) · [堆叠、直方图、拟合和瀑布图](CHARTSTATISTICS.md) · [渲染架构](CHART_RENDERING.md)

本页描述气泡、格网、雷达、等值线、OHLC 和箱线扩展。适配器保存原始输入并拥有派生 CatChartSeries，应用可配置输出的名称、颜色、轴、选择和委托，不应直接改写输出数据、类型或容量。计算在 QObject 所属线程执行，绘图使用 CatChart 已有的私有 RHI 数据层；二维 software 回退保留。没有 Qt Graphs 运行时依赖。

| QML 类型 | C++ 类／公开头 | 输出 |
| --- | --- | --- |
| CatChartColorScale | ChartColorScale；`QCatUiKit/ChartColorScale`／`qcatchartcolorscale.h` | 可复用色带与数值映射 |
| CatChartBubble | ChartBubble；`QCatUiKit/ChartBubble`／`qcatchartbubble.h` | `series`：Bubble |
| CatChartGrid | ChartGrid；`QCatUiKit/ChartGrid`／`qcatchartgrid.h` | `series`：Grid = 20 |
| CatChartContour | ChartContour；`QCatUiKit/ChartContour`／`qcatchartcontour.h` | `series`：Contour = 21 |
| CatChartOhlc | ChartOhlc；`QCatUiKit/ChartOhlc`／`qcatchartohlc.h` | `series`：Candlestick = 17 或 Ohlc = 18；`volumeSeries`：Bar |
| CatChartBoxPlot | ChartBoxPlot；`QCatUiKit/ChartBoxPlot`／`qcatchartboxplot.h` | `series`：BoxPlot = 19；`outlierSeries`：Scatter |

上述 C++ 类均在 QCatUiKit 命名空间；带 `.h` 的类名转发头也安装。既有 ChartSeries.Type 数值、Sample／RangeSample 和公开对象布局保持不变。

## 色带与气泡

`CatChartBubble` 与 `CatChartGrid` 各自拥有常量 `colorScale` 对象。`automatic` 默认 true，从当前有效数据确定 `minimum/maximum`；`setRange(minimum, maximum)` 接受有限且 minimum ≤ maximum 的范围，并切换为手动。恢复 `automatic=true` 使用最近的数据范围。`colors` 接受 2–32 个颜色，均匀分布、按预乘 alpha 插值；默认蓝—青—黄。`legendVisible` 默认 true。

`normalized(value)` 返回夹紧到 0–1 的位置，`colorAt(value)` 返回对应颜色。等值范围使用色带中点，空数据自动范围为 0–1；非有限查询返回 NaN／透明色。手动范围之外的值夹紧到端点颜色。绘制色条显示当前范围，不修改原始值。

```qml
import QtQuick
import QCatUiKit
Item {
    width: 720; height: 480
    CatChartBubble {
        id: bubbles
        sizeMode: CatChartBubble.Area
        sizeScale: 64
        points: [{x:0,y:2,size:8,colorValue:-1}, {x:1,y:4,size:16,colorValue:0},
                 {x:2,y:3,size:24,colorValue:1}]
        colorScale.colors: ["#2445a8", "#20b5b1", "#f9d057"]
    }
    CatChart { anchors.fill: parent; series: [bubbles.series] }
}
```

气泡输入为 `points`／`setData(points)`，每点要求有限 x、y、非负 size（默认 1），可选独立 `colorValue`（默认 size）及 label。最多 1,000,000 点；非法输入整批拒绝，保留原值与输出，说明见 `error`。

- `Radius`（默认）：半径 = size × sizeScale，单位为逻辑像素。
- `Area`：半径 = sqrt(size × sizeScale / π)，使面积与 size 成正比。`sizeScale` 默认 1，允许 (0, 10000]；显示半径上限 64 px，达到上限后不再满足比例。
- 零大小不生成可见或可拾取的点。原始 CatChartSeries.Bubble 的 value 仍按既有规则解释为 1–64 px 半径，不因适配器新增而改变。
- `colorMapping` 默认 true，可关闭并使用系列颜色。标记形状、纹理、选择和 pointDelegate 沿用系列接口；映射颜色会调制纹理颜色。

`at()`、拾取和委托返回 `sourceIndex`、`rawSize`、`colorValue`、实际 `radius` 及可选 `mappedColor`；value 为绘制半径。映射色条与普通系列图例分开显示，多组映射建议使用独立图表或仅开启一个色条。

## 格网热力图

`setGrid(rows, columns, values)` 接受按行排列的一维数组，数量须严格相等。各值为有限数字或 null，null 表示缺失且不可拾取；NaN／Infinity 不代表缺失。每维最多 2048，总计最多 1,000,000 单元；`setGrid(0,0,[])` 清空。

`origin` 为第 0 行第 0 列的中心，默认 (0,0)；`spacing` 默认 (1,1)，宽高必须有限且为正，所有外边界须有限。单元边界为中心 ± spacing/2。`values`、`rows`、`columns` 只读；原始数组包含缺失位置。`setValue(row,column,value)` 原子替换一个单元；`appendRow(values)` 移除首行并追加一行，行数不变，origin.y 增加 spacing.height，以保留已有行的数据坐标。非法配置或批次拒绝并保留上一结果。

```qml
Item {
    width: 720; height: 480
    CatChartGrid {
        id: grid
        dense: true
        Component.onCompleted: {
            setGrid(2,3,[0,1,null,2,3,4]);
            colorScale.setRange(0,4);
        }
    }
    CatChart { anchors.fill: parent; series: [grid.series] }
}
```

`dense` 默认 true。线性、时间或分类坐标轴下用一个格网矩形和数值纹理，片元着色器查询同一纹理中的 256 色调色表；数值归一化后以 16 位编码，原始 double 始终单独保留。缺失掩码为透明。使用最近邻采样，支持轴反向和视口裁剪。`dense=false` 使用逐单元实例矩形；对数轴自动走稀疏路径以保持几何映射。旧 Heatmap 的非连续单元也改用实例矩形，既有色条和 value 语义不变。

固定大小纹理保留 GPU 资源；数据或色带变化比较像素并仅上传变化的包围矩形。固定色域下单个有效数值变更通常上传 4 字节，自动色域改变、调色板改变、滚动或大量更新可能上传较大区域。数据校验、统计范围、编码及屏幕拾取构建仍在 CPU 上，不宣称单点更新或平移的 CPU 成本为 O(1)。`renderingStatistics()` 新增 `textureUploadedBytes`、`textureAllocations` 可检查实际上传和分配。

稀疏与密集路径使用相同的 256 色表；software 使用对应 RGBA 图像／实例回退。拾取返回 `sourceIndex`（原始 row*columns+column）、row、column、原始 value 和 mappedColor。缺失单元保留在原始数组中，不占输出系列下标；不要用输出 index 代替 sourceIndex。

## 等高线与填色区域

`CatChartContour.source` 引用 CatChartGrid，不接管其所有权；输出继承源系列轴并使用源色带。`levels` 默认为 [-1,0,1]，要求 1–16 个有限、严格递增的值；非法设置被拒绝并通过 error 说明。`filled` 默认 false，开启后填充相邻级别之间的区域，范围之外保持透明。`labelsVisible` 默认 true；每级选取一个可见且不冲突的位置标注，拥挤时省略。

```qml
Item {
    width: 720; height: 480
    CatChartGrid {
        id: grid
        Component.onCompleted: setGrid(3,3,[-1,0,1, 0,1,2, 1,2,3])
    }
    CatChartContour { id: contour; source: grid; levels: [-1,0,1,2,3]; filled: true }
    CatChart { anchors.fill: parent; series: [contour.series] }
}
```

算法约定是每个四边格网单元以四角平均值作为中心值，分为四个线性三角形，在线性标量场上提取交线并裁剪填色多边形。鞍点的连接方式由这个约定确定，所有相邻单元共享边上的线性插值；不是双线性 Marching Squares 的渐近判别结果。等值边按统一的大于／不大于分类处理，全等值三角形不生成重合等值线。填色区间左闭右开，最后一个区间含上端点；恰好位于级别上的常量平台只归属一个区间，避免重复混色。任一角点缺失时整个单元跳过，不跨缺口连线或填色。

源数据变化与参数更新合并到下一轮事件循环；`refresh()` 可同步刷新。空源／源销毁清空输出。最多 65,536 格网节点和 250,000 个生成图元，超过预算时清空派生几何、保留源并设置 error。绘图几何和生成记录随数据更新，导航不重算等值提取。

派生记录包含 `cellRow/cellColumn`、单元左下原始 `sourceIndex`、`vertices`、`filled`、`level` 或 `lowerLevel/upperLevel`、mappedColor。填色拾取检查实际三角形，线拾取检查屏幕线段邻域；返回的是派生单元／级别信息，不伪称为原始观测点。此版本不拼接或导出完整闭合等值路径，线样式使用系列 lineWidth／可选 borderColor。

## OHLC 与成交量

`CatChartOhlc.records`／`setData(records)` 接受 `{time,open,high,low,close,volume?,label?}`。时间必须有限且严格递增；low ≤ open/close ≤ high，volume 默认 0 且非负；所有数值有限，最多 100,000 条。非法批次通过 error 报告，保留之前数据。数据不排序、不合并重复时间。

`candlesticks=true`（默认）绘制 K 线实体和高低影线，false 绘制 OHLC 左开右收刻线；open=close 时绘制横线。risingColor／fallingColor 分别用于 close ≥ open／close < open。可用主题 Token 覆盖默认绿／红，不推断地区涨跌色习惯。lineWidth 与可选 borderColor 控制线条。

`compressedTime=true`（默认）以记录下标布局，分类轴可绑定 categories；关闭后以原始 time 为 X，柱宽以最小正时间间隔为基准。barGroupWidth／barSpacing 调整实体宽度。适配器始终保留原始 time；按时间戳使用时可配置 DateTime 轴（毫秒），无需插入非交易日记录。

```qml
Item {
    width: 720; height: 480
    CatChartOhlc {
        id: prices
        records: [{time:1,open:10,high:15,low:8,close:12,volume:80},
                  {time:4,open:12,high:14,low:7,close:9,volume:100}]
        volumeSeries.yAxis: CatChartAxis {
            side: CatChartAxis.FarSide
            Component.onCompleted: setRange(0,500)
        }
    }
    CatChart {
        anchors.fill: parent
        series: [prices.volumeSeries, prices.series]
        xAxis.scale: CatChartAxis.Category
        xAxis.categories: prices.categories
    }
}
```

成交量是独立 Bar 系列，应用显式添加并关联独立 Y 轴，可叠绘或放在另一个 CatChart；适配器不自动创建分面。价格系列使用 barDelegate 自定义实体，内置影线保留。拾取、选择与委托数据包含 sourceIndex、time、open/high/low/close、volume、barColor；价格记录的 lower/upper 为 low/high，value 为 close。

## 箱线图与离群点

`CatChartBoxPlot.groups`／`setData(groups)` 的每组可选 label，按组下标确定 X；categories 可绑定分类轴。支持两种互斥输入：

- 原始观测 `{label, values:[...]}`：非空、有限数字，按排序后的 `(n−1)*p` 位置线性插值求 Q1（p=.25）、中位数（.5）、Q3（.75），即常用 Type 7 定义。重复和常量观测允许。
- 统计摘要 `{label,minimum,q1,median,q3,maximum,outliers?:[...]}`：要求五个有限数值依次非递减；outliers 必须严格位于 [minimum,maximum] 外。不从摘要推测样本数或重算分位数。

原始数据的 `whiskerFactor` 默认 1.5，允许 0–10。以 Q1−factor*IQR、Q3+factor*IQR 为围栏，范围外为离群点；须端取围栏内观测极值，但不向箱体内部收缩，至少延伸到 Q1／Q3。factor=0 且分位数落在观测间时可能使用插值分位数作为须端。摘要模式的须长与离群点完全使用输入，factor 不覆盖摘要。

最多 4096 组、合计 1,000,000 个原始观测／摘要离群点。非法批次或 factor 保留上次有效输入、配置及输出，error 提供说明。空 groups 清空图表。`series` 绘制箱体、中位线和须，`outlierSeries` 独立绘制离群点，可隐藏或设置形状／大小。

```qml
Item {
    width: 720; height: 480
    CatChartBoxPlot {
        id: boxes
        groups: [{label:"A",values:[1,2,3,4,10]}, {label:"B",values:[2,3,4,5,6]}]
        series.color: Theme.tokens.primary
        series.borderColor: Theme.tokens.text
        outlierSeries.color: Theme.tokens.danger
    }
    CatChart {
        anchors.fill: parent; series: [boxes.series, boxes.outlierSeries]
        xAxis.scale: CatChartAxis.Category; xAxis.categories: boxes.categories
    }
}
```

记录返回 groupIndex、source（observations／summary）、sampleCount（摘要为 0）、minimum/q1/median/q3/maximum、outlierCount。离群点还返回 outlier=true 和原始组内 sourceIndex；摘要离群点无原始观测下标，取 −1。箱体 value 为中位数，lower/upper 为须端；自动范围包含独立可见的离群点系列。barDelegate 可替换箱体填充，中位线与须保留。

## 雷达增强

CatChart.chartType 为 Radar，继续最多使用 32 维，维度标签来自 xAxis.categories。数据顺序决定维度，显示维数为可见系列最大点数（截断到 32）；不足该维数的系列省略，避免伪造缺失维度。更多点保留在源数据中，但不参与雷达几何和自动最大值。

- 系列的 lineWidth、strokeStyle、dashPattern、dashOffset、线端／连接用于闭合边；fillOpacity=0 显示线型，其他填充参数沿用面积填充。markersVisible、markerShape、markerSize 和 pointDelegate 控制节点。
- `CatChart.radarRanges` 是新增可选列表，每项 `{minimum,maximum}` 为该维的有限递增范围，最多 32 项；空列表使用共享轴范围。自动共享范围为 0 至可见数据最大值（至少 1），手动 yAxis 使用 minimum/maximum。未提供独立范围的维度回退到共享范围。
- 值归一化后夹紧到圆心至外圈；所有系列在同一维使用相同范围。拾取返回原始 y，不覆盖输入。图例显隐、主题和选择仍使用公共图表能力。

## Gallery

新增独立 `chart-ohlc`（24）、`chart-box`（25）、`chart-contour`（27）页面。格网热力图（26）加入 `chart-heatmap`，气泡映射（28）加入 `chart-scatter`，雷达样式（29）加入 `chart-radar`，保留旧编号 0–23。

各页提供实际控件、主题／密度适配及跟随参数变化的独立 QML 示例；GalleryChartM4Preview 仍是 Gallery 内部帮助组件。例子使用少量固定数据，不是完整工程保存或参数导出。统计与扩展适配器的既定范围已完成；当前视图 PNG 导出见 [图像导出](CHARTS.md#图像导出)，配置保存／恢复、同步和原始数据导出由应用实现。
