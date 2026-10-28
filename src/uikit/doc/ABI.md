# ABI 与公开接口兼容

[文档索引](README.md) · [版本管理](VERSIONING.md)

## 边界

C++ 公开接口由安装头和导出符号定义。共享库当前数字版本为 1.0.0，SOVERSION 为 1；Windows 使用对应 DLL。ThemePalette、TableModel、ChatModel、ChartItem、ChartAxis、ChartSeries 的对象布局为 Qt 基类加一个 `const std::unique_ptr<Impl>`，状态和容器保存在实现中。

同一主版本的兼容维护要求保留已发布公开接口。替换库的前提为相同架构、平台、编译器 ABI、C++ 运行时、兼容 Qt 运行时及一致的构建模式。Pimpl 不能提供跨工具链或 Windows Debug／Release 混用能力。

QML 维护属性、信号、方法与可观察行为的兼容性。内部视觉树、id、SurfaceImpl、私有 C++ 头和 Gallery 控制器不属于公开接口。

## 变更规则

新增的 `QCatUiKit::VectorItem` 采用 Qt 基类加 const unique_ptr Pimpl，公开头不依赖 ThorVG。三个 Cat 矢量组件与该公开类型的接口见 [矢量专题](VECTOR.md)。启用 QRhi 时 ThorVG 静态链接到共享库，安装使用者无需包含 ThorVG 头或自行链接其目标；既有类型的布局和 ABI 版本不变。

`QCatLedSource` 和 `QCatLedPreview` 也使用 Pimpl；`QCatLedSource::Position` 的三个 float 字段与 12 字节布局属于公开接口。现有公开类的对象布局和 ABI 版本保持兼容。

- 保留公开类名、命名空间、基类、导出符号、函数签名、对象大小及虚函数布局。
- 不向公开对象添加状态成员，不将访问 Impl 的实现放到公开内联函数中。
- 保留已发布枚举、模型角色的数值和名称，以及属性和信号语义。
- ChartSeries::Sample 是公开值类型，其字段布局也属于兼容边界。
- 内部容器、算法和缓存可修改；新增非虚函数或 QML 属性仍须检查源码与行为兼容性。
- MascotPetController 的 actionCacheLimitMiB 为新增非虚访问方法、QML 属性和通知信号，状态保存在既有 Pimpl 中，不新增公开对象数据成员或虚函数；默认 150 MiB 改变预热和资源占用策略，旧应用可设为 0 禁用。
- 打破兼容时提升主版本与 QCATUIKIT_ABI_VERSION；CMake 的 SameMajorVersion 规则本身不检查 ABI。

发布渠道从 experimental 改为 stable 不改变 SONAME。渠道只是运行时身份的一部分，不能替代兼容性检查。

## 部署单元

库、QML 插件、qmldir、类型信息、QML 文件及资源须成套部署，并匹配所用 Qt。QML 编译缓存与 Qt 版本有关。先退出应用再替换发行文件；不支持进程运行中卸载热替换。

版本函数在共享库中执行，返回实际加载库的信息。QML 的 QCatUiKit 单例提供只读属性；其 C++ 实现不作为公开类安装。

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

具体变更的验证结果记录在对应 OpenSpec change 中；检查方法见[验证说明](VALIDATION.md)。这些结果仅覆盖记录中的工具链与调用范围，不替代候选发布包的检查。


### 姿态与体感输入

TiltController 是独立 Pimpl 类型，注册同名 QML 类型；安装真实头、TiltController.h 和 TiltController 转发头。CatAttitudeIndicator／CatTiltControlPanel 为公开 QML 组件，四元数沿用既有 QtGui 依赖，不引入 Qt Sensors 或协议库。具体变更的验证结果记录在对应 OpenSpec change 中；检查方法见[验证说明](VALIDATION.md)。

### Dock 工作区

新增独立 Pimpl 类型 DockController，安装真实头、DockController.h 和 DockController 转发头，新增两个 QML 组件。既有导出类、元对象和 SOVERSION 不变。DockGeometry.js 为内部实现；布局快照使用显式版本号。

DockController 的浮动窗口属性与非虚方法保留既有方法、信号、枚举和 Pimpl 对象布局。`dockToWorkspace` 是新增非虚方法，不改变已有函数签名或 SOVERSION。DockFloatingWindow、DockDragArea、DockDragPreview、DockDropOverlay、DockTabButton 与 DockGeometry.js 均为内部实现，不作为公共接口。

布局格式写入 v2，同时读取 v1。v2 的可选字符串 `returnNode` 用于整列／整行回停；旧 v2 不含此字段仍可读，类型错误则拒绝整份恢复请求。新增此可选字段未提高格式版本，未来未知版本仍被拒绝。

具体变更的验证结果记录在对应 OpenSpec change 中；检查方法见[验证说明](VALIDATION.md)。检查候选发行包时应重新验证对应平台和工具链。


### 角色播放控制器

新增独立 Pimpl 类型 MascotPetController，安装真实头、MascotPetController.h 与 MascotPetController 转发头。既有类型布局、枚举数值与 SOVERSION 保持不变。MascotSprite QML 与角色包解析结构为内部实现；当前角色接口见 [角色包与动画播放](MASCOTPET.md)。
