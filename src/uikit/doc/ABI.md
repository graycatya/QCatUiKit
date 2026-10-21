# ABI 与公开接口兼容

[文档索引](README.md) · [版本管理](VERSIONING.md)

## 边界

C++ 公开接口由安装头和导出符号定义。共享库当前数字版本为 1.0.0，SOVERSION 为 1；Windows 使用对应 DLL。ThemePalette、TableModel、ChatModel、ChartItem、ChartAxis、ChartSeries 的对象布局为 Qt 基类加一个 `const std::unique_ptr<Impl>`，状态和容器保存在实现中。

同一主版本的兼容维护要求保留已发布公开接口。替换库的前提为相同架构、平台、编译器 ABI、C++ 运行时、兼容 Qt 运行时及一致的构建模式。Pimpl 不能提供跨工具链或 Windows Debug／Release 混用能力。

QML 维护属性、信号、方法与可观察行为的兼容性。内部视觉树、id、SurfaceImpl、私有 C++ 头和 Gallery 控制器不属于公开接口。

## 变更规则

新增的 `QCatLedSource` 和 `QCatLedPreview` 也使用 Pimpl；`QCatLedSource::Position` 的三个 float 字段与 12 字节布局属于公开接口。现有公开类的对象布局和 ABI 版本保持兼容。

- 保留公开类名、命名空间、基类、导出符号、函数签名、对象大小及虚函数布局。
- 不向公开对象添加状态成员，不将访问 Impl 的实现放到公开内联函数中。
- 保留已发布枚举、模型角色的数值和名称，以及属性和信号语义。
- ChartSeries::Sample 是公开值类型，其字段布局也属于兼容边界。
- 内部容器、算法和缓存可修改；新增非虚函数或 QML 属性仍须检查源码与行为兼容性。
- 打破兼容时提升主版本与 QCATUIKIT_ABI_VERSION；CMake 的 SameMajorVersion 规则本身不检查 ABI。

发布渠道从 experimental 改为 stable 不改变 SONAME。渠道只是运行时身份的一部分，不能替代兼容性检查。

## 部署单元

库、QML 插件、qmldir、类型信息、QML 文件及资源须成套部署，并匹配所用 Qt。QML 编译缓存与 Qt 版本有关。先退出应用再替换发行文件；不支持进程运行中卸载热替换。

版本函数在共享库中执行，返回实际加载库的信息。QML 的 QCatUiKit 和 LibraryInfo 单例提供相同只读属性；其 C++ 实现不作为公开类安装。

## Linux 动态替换检查

脚本 [check_abi_replacement.sh](../../../tests/auto/cmake/check_abi_replacement.sh) 接受：

```sh
bash tests/auto/cmake/check_abi_replacement.sh \
  /absolute/baseline /absolute/candidate /path/to/Qt /tmp/qcat-abi-check
```

流程为：仅用基线安装包编译消费者，保存可执行文件，然后令候选库路径优先，运行同一可执行文件。检查实际加载路径、主题和两种模型行为。

基线应为需要保持兼容的发布安装包，候选为待发布安装包。该脚本验证所用消费者，不覆盖所有符号和调用方式；发布还需检查导出符号、类型信息、公开头与 QML 行为。

图表安装消费者位于 [charts-consumer](../../../tests/auto/cmake/charts-consumer)，版本消费者位于 [version_consumer](../../../tests/auto/cmake/version_consumer)。相关命令见 [验证说明](VALIDATION.md)。

## 当前扩展的兼容边界

二维 Sample、RangeSample 和三维 ChartSeries3D.Point 是独立值类型，保留各自已公开的字段、顺序与含义。XYZ 不借用二维 Sample.value；统计来源、LOD、拾取索引、纹理和 GPU 实例布局留在私有实现。

ChartSeries::Type 的扩展均追加于枚举尾部，保留既有值：PercentBar／横向柱图为 10–13，RangeArea 为 14，Histogram 为 15，Waterfall 为 16，Candlestick／Ohlc／BoxPlot／Grid／Contour 为 17–21。后续扩展继续遵守追加与兼容审查规则。

ChartScene3D／ChartSeries3D、统计与扩展适配器、LED 工作流及 TiltController 使用独立公开类型。新增 scene3D、PNG 导出状态／信号和其他属性保存在既有 Impl 中，避免改变已发布对象布局或虚函数表。具体安装头列表由 `src/uikit/CMakeLists.txt` 的三组头列表维护，新增类型必须同时安装真实头、带扩展名和无扩展名转发头。

光照默认值曾从 Phong 改为 BlinnPhong，属于可观察外观变化；显式 Phong 保留相应光照计算，不保证新网格逐像素复现旧图像。ABI 兼容也不能替代 QML 行为、默认值及实际交互审查。

各阶段的符号、安装和历史消费者执行结果见 [兼容性演进记录](VALIDATION_HISTORY.md#abi-兼容性演进记录)。这些结果仅覆盖记录中的工具链与调用范围，不替代候选发布包的检查。


M2 第三批新增径向布局属性、扇区可见性与样式方法，均为非虚接口，复用 appearanceChanged／styleChanged；状态仅进入 Impl 和私有 SeriesRecord，既有 Sample、RangeSample 及导出对象布局不变。新安装消费者与 M2 第二批、首批、M1、迁移前四份历史消费者的库替换检查均通过。

M2 第四批的柱组宽度／间距、边框与标签属性全部使用非虚访问方法和既有 appearanceChanged／styleChanged 信号；BarLabelPosition 为独立新枚举，既有枚举数值不变。新增状态只在 Impl 中，Sample、RangeSample 以及对象布局不变。新安装消费者及 M2 第三批至迁移前共五份历史消费者的库替换运行检查通过。

M2 第五批新增 AreaBaselineMode／FillStyle 独立枚举和六个系列属性，非虚访问方法复用 styleChanged，状态全部在 Impl 内。既有 Type 数值、Sample／RangeSample 和对象布局不变。新安装消费者及 M2 第四批至迁移前共六份历史消费者的库替换检查通过。

M2 第六批在 ChartItem 新增 DragMode／ZoomAxes 枚举、拖动与框选状态属性、zoomToRect／cancelZoomSelection 非虚函数，以及追加的 zoomSelectionChanged 信号。状态仅在 Impl 中，原有信号、虚函数槽、Sample／RangeSample 与对象布局不变。新安装消费者及 M2 第五批至迁移前共七份历史消费者均完成新库替换检查。

M2 第七批在 ChartItem 新增 touchNavigationEnabled 属性及非虚访问方法，复用 appearanceChanged。touchEvent／touchUngrabEvent 覆盖 QQuickItem 已有虚函数槽，不追加新的虚函数；手势状态保存在 Impl，公开对象及 Sample／RangeSample 布局不变。新安装消费者及 M2 第六批至迁移前共八份历史消费者完成新库替换检查。

M2 第八批让 RangeArea 使用既有 curveMode／splineTension，未新增公开接口、枚举或信号。双边界自然样条缓存、共同细分点及拾取参数均为私有实现；公开对象、Sample 与 RangeSample 布局不变。新安装消费者与 M2 第七批至迁移前共九份历史二进制完成库替换检查。

M2 第九批在 ChartItem 新增独立 SliceLabelPosition 枚举及 sliceLabelPosition 非虚访问方法，复用 appearanceChanged。状态仅进入 Impl；原有枚举、虚函数槽、对象与 Sample／RangeSample 布局均不变，默认仍为内部标签。私有标签布局助手不安装。新安装消费者及 M2 第八批至迁移前共十份历史二进制完成新库替换运行检查。

密集曲线拖动优化仅增加 Impl 缓存、私有层次索引及 renderingStatistics() 的诊断字段；公开类、函数签名、枚举、Sample／RangeSample 与对象布局不变。新安装消费者及十一份历史二进制通过新库替换检查。

M2 坐标轴批次在 ChartAxis 末尾追加 reversed、minorTickCount 属性，新增访问方法均非虚函数，复用既有 changed 信号；既有属性顺序、枚举、导出对象和 Sample／RangeSample 布局不变。新增字段仅在 Impl 中，图表的映射、刻度和曲线适配仍为私有实现。新安装消费者及十二份历史二进制完成新库替换检查。

M2 多轴在 ChartAxis 尾部追加 orientation／side／visible，在 ChartSeries 尾部追加 xAxis／yAxis；全部访问方法非虚，复用 changed／styleChanged。映射采用新增重载，保留原函数符号；轴引用、连接及布局存于 Impl。新安装消费者和十三份历史消费者通过库替换检查。

本次 M2 线条外观在 ChartSeries 的私有实现中保存 lineCap、lineJoin、miterLimit，追加枚举、非虚方法与元属性；不改变导出对象布局、Sample 或既有虚函数。描边三角化使用已有 GuiPrivate 依赖，随 Qt 小版本重新构建。

M2 填充新增元属性与非虚方法，TextureFill 追加于 FillStyle 枚举末尾；状态保存在 ChartSeries::Impl。ChartVertex、绘制命令及纹理快照均属私有类型，新增 UV 不影响公开对象 ABI。

M2 区间对齐仅添加 RangeAlignment 枚举和非虚 setRangeBoundaries 方法；既有 Sample／RangeSample、对象布局及元属性顺序保持不变。

M2 实例化／委托／选择新增枚举、非虚访问方法及末尾元属性／信号，状态留在 Impl；既有对象、Sample、RangeSample、虚函数槽不变。ChartInstance、委托管理与拾取索引均为不安装的私有类型。

M2 最终安装复核：新消费者与 18 个历史二进制共 19 个全部通过。animationsEnabled／transitionRunning 以及选择／委托信号均追加到元对象末尾，动画和屏幕拾取索引继续保存在私有实现。具体记录见 VALIDATION.md 的 M2 最终验收。

### M3 三维接口

ChartItem 末尾追加 scene3D 元属性及非虚访问方法，既有基类、对象布局、枚举和信号保持不变。ChartSeries3D／ChartScene3D 是新导出 Pimpl 类型，XYZ Point 与旧 ChartSeries.Sample 完全分离。真实头、带扩展名和无扩展名转发头均安装，QML 类型随模块注册。qcatchart3d_p.h 和 RHI 类型不安装、不作为应用 API；私有测试入口的导出符号不承诺 ABI。M2 完成版消费者纳入后续动态库替换验证。

三维光照优化在 ChartScene3D 末尾追加 lightingModel、ambientGroundRatio、exposure 元属性及 LightingModel 枚举，在 ChartSeries3D 末尾追加 meshDetail 元属性及 MeshDetail 枚举；RoundedCube 追加到 Shape 末尾，原有值不变。访问方法均非虚，复用既有信号，状态保存于 Impl，公开对象与 Point 布局保持不变。默认光照改为 BlinnPhong 是外观行为变化；显式选择 Phong 可保留旧版光照计算。几何精度变化独立于光照模式，不承诺逐像素恢复旧网格。

### M5 图像导出

ChartItem 末尾追加 exportingImage 元属性、exportingImageChanged／imageExportFinished 信号，以及 exportImage／exportingImage 非虚方法。抓取句柄、目标 URL、尺寸和定时器保存于 Impl；既有属性／信号、类大小、虚函数、枚举和 Sample／RangeSample 均不变，ABI 版本保持 1。图像抓取沿用 Qt Quick，不增加公共类型或运行时模块依赖。

### 姿态与体感输入

新增独立 Pimpl 类型 TiltController，注册同名 QML 类型；安装真实头、TiltController.h 和 TiltController 转发头。
CatAttitudeIndicator/CatTiltControlPanel 是新增 QML 类型，既有公开类、元属性、枚举、布局与 SOVERSION 保持不变。
未增加 Qt Sensors 或协议库依赖；新接口的四元数使用既有 QtGui 依赖。

本机 Linux / Qt 6.12 Release 的姿态安装消费者与历史图表二进制替换检查通过；此前 745 个 QCatUiKit 函数符号全部保留。该验证不代替其他平台的 ABI 检查。

### Dock 工作区

新增独立 Pimpl 类型 DockController，安装真实头、DockController.h 和 DockController 转发头，新增两个 QML 组件。既有导出类、元对象和 SOVERSION 不变。DockGeometry.js 为内部实现；布局快照使用显式版本号。

DockController 的浮动窗口属性与非虚方法保留既有方法、信号、枚举和 Pimpl 对象布局。`dockToWorkspace` 是新增非虚方法，不改变已有函数签名或 SOVERSION。DockFloatingWindow、DockDragArea、DockDragPreview、DockDropOverlay、DockTabButton 与 DockGeometry.js 均为内部实现，不作为公共接口。

布局格式写入 v2，同时读取 v1。v2 的可选字符串 `returnNode` 用于整列／整行回停；旧 v2 不含此字段仍可读，类型错误则拒绝整份恢复请求。新增此可选字段未提高格式版本，未来未知版本仍被拒绝。

2026-10-21 Windows／Qt 6.12 MSVC Release 检查保留原 1384 个导出符号，新增 1 个；独立安装消费者和旧测试程序加载新 DLL 均通过。Linux 的既有替换检查早于此次新增方法；各次平台、工具链和调用范围见[历史记录](VALIDATION_HISTORY.md)，不据此推断其他平台已验证。
