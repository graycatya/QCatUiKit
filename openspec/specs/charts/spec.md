# charts Specification

## Purpose

提供二维序列与坐标轴、三维点与网格、统计适配和图像导出。此基线记录已核查的核心数据、轴和导出契约。

该基线用于约束后续变更所涉及的现有对外行为，记录已核查的正常流程、输入边界和错误结果，不以模块名称代表全部接口均已逐项覆盖。

## Dependencies

- Qt Core、Gui、Qml、Quick 和图形后端。
- ChartItem、ChartAxis、ChartSeries、ChartSeries3D、ChartScene3D、统计适配器；二维支持 software 绘制，三维需要硬件图形后端。

## Requirements

### Requirement: R01 AxisRange

ChartAxis SHALL 校验范围并提供显式自动范围设置。

#### Scenario: 有效范围

- WHEN setRange(a,b) 的 a/b 和差值有限、a<b，且对数轴 a>0
- THEN 方法 SHALL 返回 true、更新范围并关闭 automatic。

#### Scenario: 范围失败

- WHEN 范围不满足校验
- THEN 方法 SHALL 返回 false 并保持原范围。

#### Scenario: 切换对数轴

- WHEN minimum 不大于零时切换为 Logarithmic
- THEN 范围 SHALL 调整为 1..10。

#### Scenario: 刻度边界

- WHEN 设置超范围 tickCount 或 minorTickCount
- THEN 系统 SHALL 分别截断为 2..12 和 0..10。

### Requirement: R02 SeriesData

ChartSeries SHALL 支持有限二维样本的追加与替换及容量保留。

#### Scenario: 无效批次

- WHEN 批量输入包含非有限 x/y/value 或在错误线程调用
- THEN 方法 SHALL 返回 false 并保留原数据。

#### Scenario: 容量溢出

- WHEN 接受的数据超过 capacity
- THEN 系统 SHALL 保留容量内最新样本。

#### Scenario: 非法容量

- WHEN 将 capacity 设置为不大于零
- THEN 系统 SHALL 保留原容量。

### Requirement: R03 ThreeDimensionalData

三维序列 SHALL 使用独立 XYZ 数据和完整行优先网格契约。

#### Scenario: 网格接受

- WHEN setGrid 的行列同时为正、乘积等于点数且所有记录有效
- THEN 系统 SHALL 接受完整网格并返回 true。

#### Scenario: 三维失败

- WHEN 超过一百万点、行列不匹配或 XYZ/size/rotation 无效
- THEN 系统 SHALL 拒绝数据并保留原记录。

#### Scenario: 网格追加

- WHEN 对已有网格调用单点 append
- THEN 方法 SHALL 返回 false。

#### Scenario: 缺失网格点

- WHEN 网格点具有有限 XYZ 且 valid=false
- THEN 系统 SHALL 接受该缺失标记，SHALL NOT 要求用非有限坐标表示缺失。

### Requirement: R04 LinearFit

ChartFit SHALL 暴露拟合输出、残差带和有效性。

#### Scenario: 拟合不足

- WHEN refresh 时样本少于两条或 X 全相同
- THEN 拟合 SHALL 无效，输出 SHALL 清空，统计值 SHALL 为 NaN；非空不足数据 SHALL 提供错误说明。

#### Scenario: 无源数据

- WHEN 无源或源为空时 refresh
- THEN 拟合 SHALL 无效且错误说明 SHALL 为空。

#### Scenario: 无效预测

- WHEN 拟合无效或预测 x 非有限
- THEN predict SHALL 返回 NaN。

### Requirement: R05 ImageExport

ChartItem SHALL 支持可见图表的异步 PNG 导出。

#### Scenario: 目标拒绝

- WHEN 目的地址不是绝对本地 .png URL 或含 query/fragment
- THEN 方法 SHALL 返回 false 并发出 imageExportFinished(destination,false,error)。

#### Scenario: 尺寸拒绝

- WHEN 指定像素宽高不在 1..8192 或像素总量超过 32000000
- THEN 系统 SHALL 拒绝导出。

#### Scenario: 并发拒绝

- WHEN 导出尚未结束时再次导出
- THEN 系统 SHALL 拒绝新请求并发出失败结果。

#### Scenario: 环境拒绝

- WHEN 图表不可见、尺寸无效，或 software 下导出三维图表
- THEN 系统 SHALL 拒绝请求并返回错误说明。

#### Scenario: 默认尺寸

- WHEN 有效请求未指定 pixelSize
- THEN 系统 SHALL 使用组件逻辑尺寸乘窗口 DPR 后向上取整的像素尺寸，并执行上限校验。

## Data Model

ChartAxis：scale=Linear、automatic=true、minimum=0、maximum=1、tickCount=6、minorTickCount=0、reversed=false、orientation=Vertical、side=NearSide、visible=true。

ChartSeries：Sample 含 double x/y/value 与标签，默认 capacity=100000。

ChartSeries3D.Point：double x/y/z、size（默认 1，范围 (0,100]）、rotation、label、valid（默认 true）及可选 color；网格行列同时为零表示非网格。

ChartFit：source、输出 series/band、valid、sampleCount、slope/intercept/rSquared/rootMeanSquareError；默认 residualMultiplier=2，范围 0..10。

## Known Deviations / Technical Debt

- 三维图表无 software 回退，二维有独立回退路径。
- 数据方法与属性入口的错误呈现不同，部分返回 false，部分提供 error 文本；没有统一错误码。
- 当前基线未逐项展开所有扩展系列、材质、拾取、触摸、委托及统计适配器的全部参数契约，不能据此认定这些接口不存在。

## Non-Goals

- 本模块 SHALL NOT 负责数据采集、业务持久化或网络传输。
- 本模块 SHALL NOT 将导出成功解释为跨平台像素完全一致或性能保证。

## Assumptions

- 数据与样式 SHALL 在对象所属线程修改。
- 图像导出 SHALL 提供有效窗口、可写目标及持续运行的事件循环。
