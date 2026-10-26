# 图表渲染架构与边界

[文档索引](README.md) · [二维 API](CHARTS.md) · [三维 API](CHARTS3D.md) · [验证方法](VALIDATION.md)

CatChart 使用私有 QQuickRhiItem 数据层与 Qt Quick SceneGraph 合成。当前提供二维／三维绘制、交互和当前视图 PNG 导出。具体变更的验证结果记录在对应 OpenSpec change 中；检查方法见[验证说明](VALIDATION.md)。本文维护现有实现和明确边界，具体属性及数据语义以专题 API 为准。

## 图层与线程

| 层 | 职责与源码 |
| --- | --- |
| 公开容器 | `qcatchartitem.cpp` 的 ChartItem／Pimpl 管理系列、坐标轴、布局、主题、输入、选择及导出 |
| GUI 准备 | CPU 计算范围、曲线细分、堆叠、LOD、布局与拾取索引，发布不可变快照 |
| 二维数据 | `qcatchartrhiitem_p.h`／`qcatchartrhiitem.cpp` 同步快照，管理 QRhi 缓冲、纹理、管线与透明离屏绘制 |
| 三维数据 | `qcatchart3d_p.h`／`qcatchartrhiitem3d.cpp` 消费独立 XYZ／网格、法线、共享网格及实例快照，使用深度缓冲 |
| 覆盖与委托 | 背景、网格、文字、图例、提示及选择覆盖层参与 Qt Quick 合成；任意 QML 委托受共享数量预算约束 |
| 着色器 | `src/uikit/shaders/chart*.vert/.frag` 经 qt_add_shaders 打包为 qsb |

所有系列／轴 QObject 在所属线程访问，工作线程准备的批次通过 queued connection 提交。渲染线程只消费不可变快照，不遍历或修改系列 QObject。QRhi、快照、索引和渲染器均为私有实现，不作为安装 API。

## 当前能力与数据契约

- [二维图表](CHARTS.md)：十种基础类型及阶梯／样条、断点、区间带、六种柱状布局、多环与引线标签、多轴、选择、触摸及预算委托。有序 Line／Area／RangeArea 的七种曲线模式支持层次 LOD，原始数据继续用于查询与拾取。
- [统计适配器](CHARTSTATISTICS.md)：堆叠面积、分箱、普通最小二乘与残差带、瀑布图。适配器拥有派生系列，保留原始输入，不应直接改写输出数据或类型。
- [扩展图表](CHARTEXTENSIONS.md)：独立气泡编码、密集／稀疏与滚动格网、等值线／填色、OHLC／成交量、箱线／离群点与雷达维度范围。
- [三维场景](CHARTS3D.md)：柱阵、散点、轨迹、规则曲面、极坐标、相机、Blinn–Phong／Phong、深度拾取及切片。XYZ／网格使用 ChartSeries3D，不将二维 Sample.value 解释为 Z。
- [PNG 导出](CHARTS.md#图像导出)：异步抓取完整当前视图、指定像素尺寸和透明背景；放大输出不提高现有 LOD／纹理细节。配置保存／迁移／同步、原始数据导出由应用实现。

保持既有数据语义：Area 默认填充到零线，饼图忽略非正值；新增独立契约和枚举遵守 [ABI 规则](ABI.md)，不改写既有 Sample／RangeSample 布局。Qt Graphs 仅曾作为功能参考，项目无其运行时依赖，也不承诺同名 API 或像素等价；最低版本仍为 Qt 6.8。

## 缓存、上传与性能

缓冲按容量增长并复用，快照比较后上传变化区间；未变化重绘和仅悬停不上传系列顶点。柱体和预设／纹理标记走实例通道，规则热力图支持数值纹理及变化区域上传。任意 QML pointDelegate／barDelegate／labelDelegate 使用独立 UI 层和 delegateBudget，不等同于任意 QML 的 GPU 实例化。

曲线索引缓存按数据、算法／张力和轴 scale 失效。缩放／平移复用索引，颜色或相机／材质变化可复用相应几何。首次建索引和数据变化仍需处理全量记录，不能将局部 GPU 上传表述为 CPU 全过程 O(1)。`renderingStatistics()` 提供准备、几何及上传诊断，不提供通用 FPS 保证；容量、实际绘制量、CPU 准备和 GPU／呈现时间需分开测量。

## 后端与未覆盖能力

二维保留 software 图像回退；三维需要受支持的 RHI 后端，通过 scene3D.supported／error 报告状态。shader 编译或软件截图不能证明硬件后端可用。实际后端与 DPR 矩阵见 [验证说明](VALIDATION.md)。使用 Qt 私有 GuiPrivate／QRhi 时，Qt 小版本升级后应重新构建和验证。

实时阴影、自动三维 LOD、任意模型文件解析、任意三角网格曲面、透明排序、完整 CPU 增量准备及图表 GPU 性能基准不属于当前完成范围。新增这些能力需分别定义数据契约与验收；其他平台／驱动和设备丢失也需单独验证。
