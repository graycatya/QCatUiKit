# 统计与扩展图表

[文档索引](README.md) · [图表容器、交互与拾取](CHARTS.md) · [实施清单](CHARTS_RHI_PLAN.md)

M4 提供独立的数据适配器。统计计算在对象所属线程执行，派生系列复用 CatChart 的私有 RHI 数据层及二维 software 回退；无需 Qt Graphs。原始输入与绘图结果分开保存，平移、缩放不重新进行统计。

| QML 类型 | C++ 类／公开头 | 输出 |
| --- | --- | --- |
| CatChartStack | `QCatUiKit::ChartStack`；`QCatUiKit/ChartStack` 或 `qcatchartstack.h` | 只读 `series` 列表，每个输入对应一个 RangeArea |
| CatHistogram | `QCatUiKit::ChartHistogram`；`QCatUiKit/ChartHistogram` 或 `qcatcharthistogram.h` | 常量对象属性 `series`，类型 Histogram = 15 |
| CatChartFit | `QCatUiKit::ChartFit`；`QCatUiKit/ChartFit` 或 `qcatchartfit.h` | 常量对象属性 `series`（Line）、`band`（RangeArea） |
| CatWaterfall | `QCatUiKit::ChartWaterfall`；`QCatUiKit/ChartWaterfall` 或 `qcatwaterfall.h` | 常量对象属性 `series`，类型 Waterfall = 16 |

适配器拥有输出系列，其生命周期与适配器一致。应用不要替换输出数据、容量或类型。CatChart 引用输出而不取得所有权。`CatHistogram.series` 的颜色、名称、边框和柱标签可直接设置；堆叠样式设置在输入系列上，重算时复制至对应输出。

## 堆叠面积

```qml
import QtQuick
import QCatUiKit

Item {
    width: 720; height: 420
    CatChartStack {
        id: stack
        percentage: false
        CatChartSeries {
            name: qsTr("A"); color: Theme.tokens.primary; fillOpacity: 0.7
            Component.onCompleted: setData([{x:0,y:2}, {x:1,y:4}, {x:2,y:-2}])
        }
        CatChartSeries {
            name: qsTr("B"); color: Theme.tokens.success; fillOpacity: 0.7
            Component.onCompleted: setData([{x:0,y:3}, {x:1,y:2}, {x:2,y:-3}])
        }
    }
    CatChart { anchors.fill: parent; series: stack.series }
}
```

- `sources` 是默认列表属性，可引用外部 CatChartSeries；也可调用 `addSource()`、`removeSource()`、`clearSources()`。输入对象所有权不变，销毁输入会移除其输出。顺序决定累计顺序。
- `percentage` 默认 false；普通模式累计原值，百分比模式分别将正组归一到 +100%、负组归一到 −100%。没有数值的组为 0，不生成 NaN；两端均为 0 的带段省略。
- 可见输入必须具有相同数量、完全一致且严格递增的 X，以及相同显式 X/Y 轴引用。隐式轴由图表提供；值轴必须使用线性数值空间，显式对数 Y 轴会报错。首版不提供缺失 X 插值对齐，也不支持平滑样条堆叠。
- 在原始 X 区间内按直线插值，在任何系列过零处共同切段，再分别累计正负值。百分比在这些分段端点计算，端点间绘制直线边界。生成的过零点若整组趋于零，使用该段的单侧百分比极限，避免将 100% 区域错误画成渐缩三角形；正负两侧分别断开。原始零值样本仍按零分母为 0 处理。输入 `curveMode`、标记和面积基线不参与堆叠计算。
- 任一输入的 `breakBefore` 会使整个堆叠在对应区间断开。隐藏输入从所有累计和分母中排除；图例操作输出的 `visible` 也会同步到输入并重算。
- 输入数据、样式、显隐与 `percentage` 变化在下一轮事件循环合并计算。C++ 同步读取结果前可调用 `refresh()`，成功返回 true。轴范围变化只更新视图，不重建数据。
- `error` 非空表示配置或数据无效；此时清空所有派生几何、保留输入，修复后自动恢复。最多 64 个可见系列、合计 1,000,000 个生成边界点；过零切段可能使输出数量多于原始点数。

填充、边框、渐变／纹理与线条样式继承输入，边界使用 LinearCurve。C++ 可通过 `count()`／`seriesAt(i)` 访问输出。公开 Sample／RangeSample 布局不变。

`series.at(i)`、`chart.dataAt()`、选择和委托数据包含以下附加字段：

| 字段 | 含义 |
| --- | --- |
| `sourceIndex`、`sourceSeriesIndex` | 原始点下标、输入列表下标；生成的过零端点取 X 区间内最近的原始点，等距取前点 |
| `sourceX`、`sourceY` | 对应原始点坐标，不是累计值 |
| `stackValue`、`percentage` | 此派生端点的插值原值及其有符号占比 |
| `percentageLimit` | 占比是否采用生成过零点的单侧极限 |
| `stackLower`、`stackUpper` | 此系列累计前／后的边界，负组的后边界可能小于前边界 |

既有 `index` 是派生系列下标，`x/y/lower/upper` 为绘制边界；选择高亮针对派生记录。拾取不将插值点伪称为原始观测值。原始值和派生端点占比在过零切段处可能属于不同 X，应用可按上述字段分别显示。

## 直方图

```qml
import QtQuick
import QCatUiKit

Item {
    width: 720; height: 420
    CatHistogram {
        id: distribution
        values: [0, 0.5, 1, 2, 3, 6, 7]
        binEdges: [0, 1, 3, 6]
        normalization: CatHistogram.Density
        series.name: qsTr("观测值")
        series.color: Theme.tokens.primary
    }
    CatChart { anchors.fill: parent; series: [distribution.series]; barSpacing: 0.04 }
}
```

| 属性／方法 | 约定 |
| --- | --- |
| `values`／`setData(values)` | 保存原始有限数值，最多 1,000,000 条；setData 返回是否成功 |
| `binCount` | 自动等宽分箱数，默认 10，范围 1–4096 |
| `binEdges` | 默认空，使用自动分箱；非空时覆盖 binCount，要求 2–4097 个有限、严格递增的边界，箱宽也必须有限 |
| `normalization` | `Count`（默认）为频数；`Frequency` 为箱内数／入箱总数；`Density` 为频率／箱宽 |
| `sampleCount`、`includedCount` | 原始样本数、位于分箱范围内的样本数 |
| `underflowCount`、`overflowCount` | 小于首边界、大于末边界的样本数；不参与归一化分母 |
| `error` | 无效输入的说明；拒绝整个修改，保留上一次有效参数、原始数据和输出 |

自动分箱覆盖样本最小至最大值。常量数据在该值两边扩展可表示区间；空数据的自动分箱输出为空，自定义边界则保留计数为 0 的箱。单箱为 `[lower, upper)`，最后一箱为 `[lower, upper]`。恰好位于内部边界的数据进入右侧箱。

每箱输出一条记录，`x` 为箱中心、`y` 为所选归一化高度、`value` 为频数、`label` 为区间文本。附加 `binIndex`、`binLower`、`binUpper`、`binCount`、`frequency`、`density`、`upperInclusive` 字段随 `at()`、拾取、选择和委托返回。有入箱数据时频率总和为 1，密度乘箱宽的积分为 1；无入箱数据时全部为 0。任何箱宽或密度无法有限表示时拒绝修改。

柱体宽度来自实际箱边界，因此支持不等宽分箱；`barSpacing` 为箱宽内留白比例，`barGroupWidth` 不影响直方图。同图多个直方图按系列顺序叠绘，不自动分组或累计。使用线性 X 轴比较宽度和密度面积；对数轴的屏幕面积不代表概率。样本更新和分箱配置更改重新计算统计，柱体外观与视图导航仅重新布局／绘制。

## 线性拟合与残差带

```qml
import QtQuick
import QCatUiKit

Item {
    width: 720; height: 420
    CatChartFit {
        id: fit
        source: CatChartSeries {
            id: observations
            type: CatChartSeries.Scatter
            Component.onCompleted: setData([{x:0,y:1}, {x:1,y:2}, {x:2,y:2}])
        }
        residualMultiplier: 2
        series.color: Theme.tokens.danger
        band.color: Theme.tokens.danger
    }
    CatChart { anchors.fill: parent; series: [fit.band, observations, fit.series] }
}
```

`source` 引用输入 CatChartSeries，不接管所有权。使用全部原始 `(x,y)` 点在数据坐标内进行普通最小二乘线性拟合，不使用显示 LOD；输入不必排序，可重复 X，`breakBefore` 不排除观测点。不同 X 至少两个，支持常量 Y；不提供权重、多项式或对数坐标下的重新拟合。

| 属性／方法 | 约定 |
| --- | --- |
| `valid`、`sampleCount` | 拟合是否成功、源数据点数 |
| `slope`、`intercept` | `y = slope * x + intercept` 的系数 |
| `rSquared` | `1 − SSE/TSS`；常量 Y 的精确拟合为 1 |
| `rootMeanSquareError` | `sqrt(SSE/n)`，单位与原始 Y 相同 |
| `residualMultiplier` | 残差带半宽系数，默认 2，允许有限值 0–10 |
| `predict(x)` | 有效拟合的预测 Y；无效输入、无拟合或不可表示的结果返回 NaN |
| `refresh()` | 同步刷新，成功返回 true；通常由源数据变化自动合并调度 |
| `error` | 点数不足、X 完全相同或结果超出有限数值范围的说明 |

拟合线与残差带都在原始 X 的最小／最大值处各输出一个端点。带边界为 `拟合 Y ± residualMultiplier × RMSE`，**不是置信区间或预测区间**。默认带透明度 0.2、无边界描边；可独立配置两个输出系列的样式、名称和可见性。输出继承源的显式轴引用，不继承源的其他样式。

源数据更新在下一轮事件循环合并重算；颜色、标记和视图导航不触发拟合。内部先居中缩放再累计协方差，避免大偏移坐标的直接平方求和。空源／空数据无错误且 `valid=false`；不满足拟合条件时保留源、清空输出，统计数值及 `predict()` 返回 NaN。销毁源会清空输出。

`at()`、拾取、选择和委托返回 `fitted`、`observationCount`、`predictedY`、`residualHalfWidth`。这些是派生端点的信息，不生成伪造的原始点索引。需要某个 X 的精确拟合值时使用 `predict(x)`。

## 瀑布图

```qml
import QtQuick
import QCatUiKit

Item {
    width: 720; height: 420
    CatWaterfall {
        id: waterfall
        increaseColor: Theme.tokens.success
        decreaseColor: Theme.tokens.danger
        subtotalColor: Theme.tokens.warning
        totalColor: Theme.tokens.primary
        series.barLabelsVisible: true
        steps: [{value:120, label:qsTr("收入")}, {value:-45, label:qsTr("成本")},
            {kind:CatWaterfall.Subtotal, label:qsTr("小计")},
            {value:-25, label:qsTr("调整")}, {kind:CatWaterfall.Total, label:qsTr("总计")}]
    }
    CatChart {
        anchors.fill: parent
        series: [waterfall.series]
        xAxis.scale: CatChartAxis.Category
        xAxis.categories: waterfall.categories
    }
}
```

| 属性／数据 | 约定 |
| --- | --- |
| `steps`／`setData(steps)` | 最多 100,000 个步骤；setData 返回是否接受 |
| 步骤 `{kind, value, label}` | kind 默认为 `Change`（0），value 为有限增减值；label 默认从 1 开始的序号 |
| `Subtotal`（1）／`Total`（2） | 从零画到当前累计值，不再次累计；必须省略 value |
| `initialValue` | 初始累计值，默认 0，必须有限 |
| `resetOnSubtotal` | 默认 false；开启后每个小计之后将累计值恢复为 initialValue，并断开下一条连接线；Total 不重置 |
| `connectorsVisible` | 默认 true，相邻柱之间显示当前累计高度的水平连接线 |
| `increaseColor`／`decreaseColor`／`subtotalColor`／`totalColor` | 四类柱色；默认无显式颜色，回退到输出系列颜色／图表调色板 |
| `categories`、`finalValue` | 步骤标签列表、处理完全部步骤后的累计值；最后一步为重置小计时等于 initialValue |
| `error` | 无效步骤、非法初始值或累计溢出的说明；整个修改被拒绝，保留上次有效输入、配置和输出 |

首版为纵向瀑布图，各步骤占一个分类位置。增减柱从上一步累计值画到下一累计值，支持正负和跨零；小计、总计均以零为基线。`barGroupWidth`／`barSpacing`、柱边框、标签、选择和委托沿用 CatChartSeries 的能力。连接线使用 `series.borderColor`（未设置时使用系列颜色）和 `series.lineWidth`。累计量使用线性 Y 轴；首版不提供横向布局、自动多系列分组或重置组间的独立总累计。

派生记录 `x` 为步骤下标，`lower/upper` 为柱起止值；减量柱的 upper 可以小于 lower。`value` 及标签 `@value` 对增减步骤返回原始增减量，对小计／总计返回累计值。零高度步骤仍保留在数据和分类中，但没有可拾取的柱体面积。

附加字段 `sourceIndex`、`waterfallKind`、`delta`、`cumulativeBefore`、`cumulativeAfter`、`resetAfter`、`connectorBefore`、`barColor` 随 `at()`、拾取、选择和委托返回；`cumulativeAfter` 是当前柱的结束值，发生小计重置时仍是重置前的小计值。统计发布与元数据更新同时完成，应用不需重建系列。

## Gallery

“图表 → 扩展图表”下的 `chart-stack`（编号 20）和 `chart-histogram`（编号 21）为独立页面。支持普通／百分比、正负值、共享断点、显隐、不透明度，以及箱数、不等宽边界、异常值、归一化和柱间距。数值操作使用 CatSliderField。QML 示例随当前参数更新，可脱离 Gallery 使用。

“图表 → 坐标轴与组合”下 `chart-fit`（22）展示观测值、拟合和残差带，提供噪声、带宽、无效／空输入控件；“扩展图表”下 `chart-waterfall`（23）展示初始值、成本、柱宽、小计重置、连接线和标签。既有组合示例的趋势线也使用公共 CatChartFit。以上示例均跟随参数更新。

M4 的 E01–E10 已完成明确范围；其他六项接口见 [扩展图表](CHARTEXTENSIONS.md)。M5 仅提供当前图表的 PNG 图像导出；配置保存、同步及原始数据导出由应用负责。
