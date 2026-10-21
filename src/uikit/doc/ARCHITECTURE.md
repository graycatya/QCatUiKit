# 架构与源码组织

[文档索引](README.md)

## 模块组成

```text
应用 QML / C++
  ├─ QCatUiKit QML 模块
  │    ├─ Cat 组件：Qt Quick Controls / Templates 与声明式组合
  │    ├─ Theme / ThemePalette：主题配置 → 语义 Token → 控件
  │    ├─ TableModel / ChatModel / TreeModel：数据模型
  │    ├─ CatChart / 坐标轴 / 系列 / 适配器：二维与三维图表
  │    ├─ LedPreview / Deck / Storage / Mixer：LED 工作流
  │    └─ CatAttitudeIndicator / TiltController：姿态与体感
  └─ QCatUiKit 共享库
       ├─ 主题、表格、消息的公开 C++ 类
       ├─ 图表、统计适配器、LED 管线和体感控制器
       ├─ 私有二维／三维 RHI 渲染实现
       ├─ 运行时版本查询
       └─ 私有实现、容器、采样和布局算法

Gallery 应用
  ├─ 导航、搜索、设置、组件目录与示例
  ├─ 中英文翻译管理
  └─ 图表／数据演示控制器与持续压力测试
```

`qt_add_qml_module` 生成类型注册、插件、资源及工具类型信息。库的 QML URI 为 `QCatUiKit`，资源前缀为 `/qt/qml`。Gallery 使用内部静态模块 `QCatUiKit.Gallery`，由可执行程序和 Gallery 测试复用。

## 目录

| 路径 | 职责 |
| --- | --- |
| `.cmake.conf` | 基础版本数字 |
| 根 `CMakeLists.txt` | 依赖、构建选项、发布渠道和包导出 |
| `cmake/` | 安装包配置模板 |
| `src/uikit/` | 共享库 C++ 公开头、私有头与实现 |
| `src/uikit/qml/` | Cat 组件、Theme、内部视觉实现与变换辅助代码 |
| `src/uikit/doc/` | 当前功能、接入、维护和验证文档 |
| `examples/uikit/gallery/` | Gallery 页面、控制器、翻译及演示资源 |
| `tests/auto/models/` | 主题、消息与表格模型协议 |
| `tests/auto/led/` | LED 帧、管线、输出和真实 GPU 预览 |
| `tests/auto/motion/` | TiltController 输入、校准、授权与超时 |
| `skills/qcatuikit-ui/` | UI 技能、按需参考与安装包应用模板 |
| `tools/` | 文档校验与 Gallery 截图工具 |
| `tests/auto/charts/` | 图表、坐标和序列数据 |
| `tests/auto/version/` | C++／QML 版本身份 |
| `tests/auto/qml/` | 控件行为、视觉状态、动画时序 |
| `tests/auto/gallery/` | 页面、示例、翻译和演示数据控制器 |
| `tests/auto/cmake/` | 安装消费者与动态库替换检查 |
| `LICENSES/` | 所包含代码的附带许可证 |

C++ 文件采用小写名称，私有头以 `_p.h` 结尾且不安装。QML 文件名保持类型名称，例如 `CatButton.qml`。

## 公开接口与安装

C++ 公开状态类采用 Qt 基类加 `const std::unique_ptr<Impl>` 的布局，状态与算法位于实现文件。构造、析构和导出函数在共享库中执行；兼容边界见 [ABI](ABI.md)。

真实公开头位于 `src/uikit`。配置阶段生成构建目录的转发头；安装包提供真实头和相对路径转发头，例如：

- `<QCatUiKit/qcatthemepalette.h>`
- `<QCatUiKit/ThemePalette>`
- `<QCatUiKit/ThemePalette.h>`

CMake 统一链接 `QCatUiKit::QCatUiKit`。QML 公开名称由组件文件、注册宏和模块 CMake 定义。TreeModel 与版本信息单例的 C++ 实现为私有；SurfaceImpl 是内部 QML 类型，应用不依赖其对象树或 id。

## 状态、线程与视图

主题配置生成只读 Token map，控件通过绑定更新。Theme 按 QML 引擎创建；局部 ThemePalette 由调用方持有并显式传入。组合组件把主题继续传给其内部控件。

模型只管理数据与标准 Qt 模型通知，不决定光标、列宽或窗口大小。TableModel 使用 QAbstractTableModel，ChatModel 使用 QAbstractListModel，TreeModel 提供只读层次数据。视图负责布局、选择、滚动与 delegate。

所有 QObject API 在所属线程调用。工作线程准备批次，通过 queued connection 交给模型或图表对象；不要并发读写容器，也不要跨数据修改保留内部样本引用。

图表将数据和样式更新合并到 polish 阶段，在 GUI 线程准备不可变布局与图元快照。私有 QQuickRhiItem 负责数据层的 GPU 缓冲和透明离屏绘制；背景／网格及独立文字／交互覆盖层通过 SceneGraph 合成。software 后端通过图像绘制回退；具体数据、诊断与性能边界见 [图表](CHARTS.md)。

曲线缓存按系列的数据、插值模式／张力、实际类型及两轴 scale 失效。自然样条控制点保存在变换后的数据坐标中；有序阶梯／样条 Line／Area 的层次索引记录连续段、控制多边形 X 边界及真实 Y 极值。平移和缩放只遍历可见节点、生成当前细节层级的不可变快照，固定轴范围不重新扫描全量极值。概览命中会细查原始曲段；这些缓存及诊断计数全部保存在 Impl 和私有助手中。

面积填充在 GUI 准备阶段按基线／上下界交点拆分并裁剪至绘图区。双色渐变的起终颜色和绘图区坐标保存在不可变批次中；RHI 快照只接收预乘顶点颜色，software 使用同一坐标的线性渐变画刷。边界与标记颜色独立，不给渲染线程增加 QObject 访问。

RangeArea 的上下边界按共同参数递归细分，任一边界误差超标就一起分割；这样每个填充条带的上下端点仍共享 X。自然样条分别缓存两条边界的控制点，但使用相同有效区间。自动范围包含双边界极值；拾取记录保留原始曲段索引和细分参数，返回源记录而非插值点。

框选手势与选框保存在 ChartItem 的 Impl 中，只有松开后的有效矩形才更新所选坐标轴。拖动与取消仅更新私有覆盖层，不标记数据几何为脏。选框记录开始时的绘图区和范围，映射发生变化则取消，避免对过期坐标缩放。场景初始化请求重绘，几何更新由布局／数据失效处理。

触摸输入同样由 ChartItem 处理，私有 RHI 子项不参与事件捕获。Impl 按触点 ID 维护中心与间距，单次坐标变换同时计算平移和缩放后的两轴范围；增减触点时重设基准。手势固定绘图区左边距，结束后恢复刻度布局，避免 DateTime 标签宽度变化引起锚点漂移。轴变化会准备新的绘制快照；轻点／静止触点不上传新数据。

## Gallery 边界

GalleryChartData 保存演示源数据并生成采样／聚合预览；GalleryDataDemo 持有实际表格和消息模型，管理批次、容量及统计。两者都属于应用层，安装库不包含这些演示 API。

页面销毁、离开或隐藏时停止持续测试。运行统计反映数据操作；绘制发生在后续 Qt Quick 渲染阶段。示例容量不能直接推导库在其他应用或设备上的性能上限。

## LED 工作流

LED 工作流由 `qcatledlayoutmodel`、`qcatlightingframe`、`qcatledeffectregistry`、`qcatledstorage`、
`qcatleddeck`、`qcatledmixer`、`qcatledoutput` 组成。实时数据走不可变连续缓冲，低频配置走 QObject / ListModel；
预览 RGBA 转换和设备 Patch 从同一最终 LightingFrame 分支。`qcatledproject` 是可选装配，不是 Deck 的强制依赖。
Gallery 只选择示例配置，旧 `GalleryLedDemo` 保留为旧 RGBA 回归演示 helper。详见 [LED 工作流](LEDWORKFLOW.md)。

## 三维图表数据层

ChartItem 的 Pimpl 持有 ChartScene3D 与私有 ChartRhiItem3D。ChartSeries3D 使用独立 double XYZ／网格契约；GUI 线程完成局部映射、共享网格、曲面索引、法线与拾取 BVH。渲染线程只消费不可变 ChartFrame3D／ChartGeometry3D，使用窗口 QRhi 的深度缓冲和实例绘制；相机和材质变化只更新统一缓冲，几何快照复用。三维 software 后端明确不可用，二维软件回退继续保留。公开入口见 [CHARTS3D.md](CHARTS3D.md)。

## 姿态与体感输入

CatAttitudeIndicator 是纯 QML 飞机姿态仪表，直接绘制圆及圆弓形天地，运动只影响飞机遥测对应的视觉层。
TiltController 是独立 Pimpl QObject，接受应用提供的四元数与单调时间戳，处理校准、滤波、映射、授权和输入超时。
CatTiltControlPanel 只显示状态并发出用户请求。设备传感器、屏幕方向转换、飞控通信和发送端失效策略均由应用负责，
模块及安装包不引入 Qt Sensors/MAVLink。Gallery 的飞机与手机模拟源独立。详见 [姿态与体感](ATTITUDE.md)。

## 图表覆盖、委托与拾取

二维数据层与文字／交互覆盖层独立更新；选择与悬停通过覆盖层和委托载荷反馈，避免无关数据缓冲上传。屏幕 BVH 索引用于二维拾取，三维使用独立深度与几何索引。任意 QML 委托通过私有 ChartDelegates 管理 QQuickItem 生命周期，并受共享预算限制；渲染线程不访问这些对象。详见 [图表渲染架构](CHART_RENDERING.md)。

## 维护入口

新增组件同步维护模块 CMake、组件目录、GalleryCatalog、交互预览、翻译及相关检查。布局使用逻辑像素和主题 Token，输入优先采用 Qt Controls 的焦点、键盘、禁用与可访问性协议。详见 [贡献指南](../../../CONTRIBUTING.md) 和 [验证说明](VALIDATION.md)。


### Dock 工作区

DockController 使用 Pimpl 保存独立布局树和面板 ID 注册表，负责分割、标签操作及事务式 JSON 恢复。
CatDockWorkspace 根据树计算几何并布置已有 CatDockPanel；内容实例不随布局重建，业务状态由应用内容持有。
最低尺寸计算与当前工作区尺寸分离，宿主可据此设置最小窗口或外部滚动区域，避免尺寸绑定循环。
浮动宿主共享控制器和面板注册表，只切换视觉父项；回停提示和窗口几何纳入版本 2 布局。同一工作区内支持主窗口与浮动窗口之间的标签拖动，窗口外的随动预览由独立、输入穿透的预览窗提供。组内分割与工作区外边缘分割分别作用于目标标签组和整个主布局；回停记录相邻子树，避免完整侧列只回到单个面板旁。公开接口和限制见 [Dock 工作区](DOCKING.md)。
