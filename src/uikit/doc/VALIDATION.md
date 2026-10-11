# 本地验证记录

日期：2026-10-08。环境：Linux x86_64、GCC 15.2、Qt 6.12.0、CMake + Ninja。QML 测试与截图使用 offscreen 平台和 Qt Quick software 后端。

| 检查 | 结果 |
| --- | --- |
| Debug 完整构建：共享库、QML 插件、gallery、测试 | 通过 |
| Release 最小构建：BUILD_TESTING=OFF、QCATUIKIT_BUILD_EXAMPLES=OFF | 通过 |
| CTest：models、qml_contracts、gallery_smoke | 3/3 通过 |
| C++ 行为测试 | 5 个测试函数通过，覆盖模型协议、越界、通知、主题及缓存失效 |
| Qt Quick Test | 38 项通过（包含初始化与清理）；其中 27 项为所有公开 QML 组件的导入与创建 |
| QML 行为 | 鼠标、键盘、模拟触摸、加载状态、enabled 绑定恢复、IPv4、分页、日期、Escape、局部主题与析构 |
| QCatUiKit_qmllint / qcat-gallery_qmllint | 无警告；.qmllint.ini 将 MaxWarnings 设为 0 |
| Debug 安装包由 Qt 自带 qmltestrunner 加载 | 38 项通过，无源码树或 build 导入路径 |
| Release 安装包由 Qt 自带 qmltestrunner 加载 | 38 项通过 |
| find_package 独立 C++ 消费者 | 从安装包成功配置、编译、链接及执行 |
| 不重编译消费者的共享库替换 | 通过，实际加载候选安装目录中的 libQCatUiKit.so.1 |
| QCatUiKit 导出符号 | 基线 91 个，候选 91 个，删除 0 个 |
| 视觉检查 | 桌面亮色、触摸暗色、390px 移动布局已渲染并查看 |

## 初次迁移的 ABI 实验

先安装不含主题缓存的基线，用其公开头和库编译 `tests/auto/cmake/consumer`。随后只在 ThemePalette 的 Impl 增加 QVariantMap 缓存与失效逻辑，重新构建安装候选库。原消费者在候选库优先的 LD_LIBRARY_PATH 下正常运行，dladdr 输出确认加载的是候选库。公开对象大小在客户端为 24 / 24 / 24 字节；导出符号集合没有删除。此结果是对当前消费者与此次私有实现变化的验证，不替代未来发布时的完整 ABI 审计。

复现入口：`tests/auto/cmake/check_abi_replacement.sh`；平台边界见 [ABI](ABI.md)。

## 截图

- [桌面亮色](images/gallery-desktop.png)
- [触摸暗色](images/gallery-touch.png)
- [移动宽度](images/gallery-mobile.png)

截图命令：

```sh
QT_QPA_PLATFORM=offscreen QT_QUICK_BACKEND=software \
  ./build/examples/uikit/gallery/qcat-gallery --screenshot /tmp/gallery.png
# 可附加 --dark --density touch，或 --density mobile --width 390
```

## 尚未执行的环境验证

上文 2026-10-08 的验证未覆盖 Windows、macOS、Android、iOS 或实体嵌入式设备，也没有在 Qt 6.8 上运行该轮测试。Windows 的后续验证见下文。Qt 6.8 是项目声明的最低版本，本次实际执行版本为 6.12.0。触摸使用 Qt Quick Test 合成事件；实际输入法、屏幕阅读器、GPU 上的 MultiEffect、设备 DPI、功耗与帧率需要目标设备验收。软件后端截图不能证明 GPU 特效效果。

迁移关系图用于旧模型导航，有 4 条悬空端点和 3 组同端点关系合并；关键结论已核对源码。详细说明见 [架构文档](ARCHITECTURE.md)。

## 版本接口新增验证

新增 C++ Version.h 与 QML LibraryInfo 后，分别构建 experimental（Debug）和 stable（Release），结果：

- 实验构建 CTest 4/4 通过，正式构建 3/3 通过（未构建 gallery）。
- version 测试将 C++ 函数和两个 QML 引擎中的只读单例与 CMake 预期配置对比，均通过。
- 两种安装包分别由 Qt qmltestrunner 加载，各 39 项通过。
- qmllint 无警告；非法渠道 preview 在 CMake 配置阶段被拒绝。
- 独立消费者仅用实验安装包编译一次。在固定 Qt 6.12 运行时下，先加载实验库得到 `1.0.0-experimental experimental ABI 1`，再切换到正式库得到 `1.0.0 stable ABI 1`，消费者未重新编译。
- 安装包的 VERSION_STRING、RELEASE_CHANNEL 和 ABI_VERSION 元数据正确。
- 旧 ABI 消费者仍可运行；QCatUiKit 导出符号从 91 增至 97，无删除。

开发机同时存在系统 Qt 6.10 与独立 Qt 6.12；独立消费者初次未指定运行时路径时出现 Qt 间接依赖混用，使用与构建一致的 Qt lib 路径后通过。版本管理文档的复现命令已包含该路径。

## Qt 风格目录整理验证（2026-10-11）

环境：Windows 11 x64、MSVC 19.39、Qt 6.12.0、CMake 3.27.7、Ninja。使用独立的 `build-layout-*` 和 `stage-layout-*` 目录，保留原有 Qt Creator 构建目录。

| 检查 | 结果 |
| --- | --- |
| Debug experimental：共享库、插件、gallery、全部测试 | 完整构建通过 |
| Release stable：BUILD_TESTING=OFF、QCATUIKIT_BUILD_EXAMPLES=OFF | 最小构建及安装通过 |
| Debug CTest：gallery_smoke、models、version、qml_contracts | 4/4 通过 |
| QCatUiKit_qmllint / qcat-gallery_qmllint | 两个目标通过，无警告 |
| Release 安装包由 Qt qmltestrunner 加载 | 39 项通过，导入路径只指定安装包 |
| Debug 安装包的独立 C++ 消费者 | 配置、编译、链接、运行通过；保留原有 `.h` 头路径 |
| Release 安装包的独立版本消费者 | 输出 `1.0.0 stable ABI 1`；包渠道检查通过 |
| 无扩展名及小写头路径 | 现有模型、版本测试分别编译通过 |
| 作为 add_subdirectory 子项目配置 | 外部父项目配置及生成通过 |
| 未重编译的旧消费者加载新 DLL | 通过；原二进制哈希不变，候选 DLL 放在其可执行文件目录 |
| Windows DLL 导出符号 | 基线 91 个，候选 91 个，符号集合无差异 |
| 公开模型对象大小 | ThemePalette、ChatModel、TableModel 均为 24 字节 |
| 安装头与 CMake 包配置 | 未包含源码绝对路径，未安装 `_p.h` 私有头 |
| QML 源码与文档路径 | 29 个 QML 文件与迁移前内容一致；本地文档链接全部有效 |

Windows offscreen 插件需要显式指定字体目录；未指定时严格的 `failOnWarning` 会将 Qt 的缺少字体目录警告判为失败。本轮运行环境为：

```powershell
$env:PATH = 'D:\app\Qt\6.12.0\msvc2022_64\bin;' + $env:PATH
$env:QT_QPA_FONTDIR = 'C:/Windows/Fonts'
ctest --test-dir build-layout-debug --output-on-failure
```

独立安装消费者检查发现并修复了原有包配置问题：CMake 3.30 之前，依赖包可能覆盖 `PACKAGE_PREFIX_DIR`，导致 `QCatUiKit_QML_IMPORT_PATH` 错指向 Qt 安装目录。包配置现在在加载依赖前保存自身前缀，随后恢复。用于二进制替换检查的基线安装包也仅修正了这段配置，基线头文件与 DLL 内容保持原样。

本轮验证覆盖 Windows 的构建、安装及软件后端行为；没有重新执行 Linux、macOS、移动端或实体设备验证，也没有新增视觉截图。此次目录整理不改变 QML 组件内容或公开 C++ 对象布局。

## 模块信息单例命名验证（2026-10-11）

公开 QML 信息入口改为 `QCatUiKit`，私有 C++ 实现类为 `QCatUiKitInfo`，文件为 `qcatuikitinfo_p.h/.cpp`。旧名称 `LibraryInfo` 通过 `QML_FOREIGN` 注册为兼容入口。gallery 标题使用 `qsTr("QCatUiKit %1 · Component Gallery").arg(QCatUiKit.version)`，版本继续来自已加载的模块。

- Debug experimental 与 Release stable 重新构建均通过。
- CTest 4/4 通过；version 测试分别验证 `QCatUiKit`、命名空间导入下的 `UiKit.QCatUiKit` 和兼容名称 `LibraryInfo` 在多个 QML 引擎中的版本、发布渠道和 ABI，并检查八个公开属性均不可写。
- QML 契约测试覆盖新入口及旧入口属性一致性；Release 安装包由独立 Qt qmltestrunner 加载，40/40 通过。
- QCatUiKit_qmllint 与 qcat-gallery_qmllint 两个目标通过，无警告。

此处的库版本、宿主应用版本、Qt 运行时版本和 QML import 版本分别管理；QML 单例是公开 API，`_p.h` 仅标识其 C++ 实现为私有。

## Loading 转速计时验证（2026-10-11）

`CatBusyIndicator` 原先使用 `RotationAnimator`，配置为 900 ms 一圈。Qt 的线程渲染循环默认按刷新周期推进 Animator 的时间；当实际帧率与假定的刷新周期不一致、垂直同步未生效时，动画可能过快。此机制见 [Qt 场景图的动画驱动说明](https://doc.qt.io/qt-6/qtquick-visualcanvas-scenegraph.html#driving-animations)。改为 `FrameAnimation.frameTime` 的实际帧间隔累计角度，仍保持 900 ms 一圈；不修改应用全局渲染后端或动画驱动。

新增 `tst_busyindicator.qml` 从渲染图像读取圆弧方向，检查实际转速及 running、reducedMotion、隐藏后的停止／恢复行为。`qml_busy_timing` 使用仅在测试中启用的五倍速动画时钟模拟计时漂移，普通 QML 契约测试继续使用默认时钟。

- 修改前：五倍速时钟下 558 ms 旋转 1120.9°，转速回归测试失败；同一时间正常应旋转约 223.2°。
- 修改后：五倍速时钟下独立图像探针测得约 398.98°/s，对应约 902 ms 一圈；停止、隐藏、减少动效与恢复检查通过。
- Windows 11 / MSVC / Qt 6.12.0：Debug 库、gallery、测试构建通过，CTest 5/5 通过，两个 qmllint 目标无警告。
- Release stable 构建及安装通过，独立 Qt qmltestrunner 从安装包加载，46/46 通过。
- 桌面亮色、触摸暗色、390 px 移动密度的 offscreen 截图已渲染并检查，loading 圆弧正常显示。

本轮计时验证使用 Windows offscreen 软件渲染和人工加速动画驱动，未在用户发生问题的 Windows GPU／屏幕组合下复现，也未重新执行 Linux 测试。测试证实控件能够抵抗动画时钟加速，不能据此断定具体显卡或垂直同步设置就是用户环境的触发原因。`FrameAnimation` 在 GUI 线程更新角度，GUI 线程阻塞时画面仍会暂停。
## Gallery 改版与中英文国际化验证（2026-10-11）

参考 Fluent-Qt gallery 的分类导航、组件目录和预览／示例代码组织方式，重构为首页、七个分类、27 个组件详情页及设置页。支持组件搜索、QML 代码复制、示例重置、窄屏抽屉导航、亮暗主题、主题色、交互密度和减少动效。

国际化使用 Qt Linguist：`qcatgallery_en.ts` 与 `qcatgallery_zh_CN.ts` 各有 193 条完成的翻译，QM 嵌入程序。默认跟随系统语言，顶部与设置页可运行时切换；日历 locale、表格样例、聊天状态和欢迎消息同步更新。语言切换保留当前组件、输入、已发送消息、未发送草稿、所选日期与已打开的弹出层。

| 检查 | 结果 |
| --- | --- |
| Windows 11 / MSVC / Qt 6.12.0 Debug experimental | 库、插件、gallery 与测试构建通过 |
| CTest | 6/6 通过；gallery 契约测试 64/64 通过 |
| 所有组件的两种语言 | 27 × 2 个页面的加载、渲染和示例代码检查通过 |
| 运行时切换 | 搜索、输入、消息／草稿、日历日期、弹出层与移动导航检查通过 |
| QCatUiKit_qmllint / qcat-gallery_qmllint | 两个目标通过，无警告 |
| Release experimental / BUILD_TESTING=OFF | 单独构建 qcat-gallery 及 gallery 静态检查通过 |
| Release 从其他目录启动 | 中文首页、英文日历、中文对话、英文暗色触摸、390 px 中文移动布局正常加载 |
| 翻译文件 | 无 unfinished 条目；中英文占位符一致 |
| 文档截图 | 已更新为此次 Windows 软件渲染的中文桌面、英文暗色触摸、中文移动截图 |

单独构建 gallery 时显式依赖 QCatUiKitplugin；程序按可执行文件位置加入相对 QML 导入路径，支持构建树与程序旁的 qml 目录。内部 `qcat-gallery-ui` 静态模块同时供程序与测试使用；库的公开 API 和安装包不包含此内部模块，关闭示例时不引入 LinguistTools。

本轮覆盖 Windows offscreen 软件渲染，未执行 Linux、macOS 或原生 GPU 验收。视觉效果示例在软件后端显示原图，GPU 阴影仍应在目标设备验证。运行参数与翻译维护流程见 [Gallery 说明](../../../examples/uikit/gallery/README.md)。

## Gallery 跑马灯预览验证（2026-10-11）

CatMarquee 仅在文本宽度超过视口时滚动。此前 gallery 的 Loader 把控件撑满预览区，桌面宽度为 856 px，中英文示例分别为 620 px／266 px（测试环境字体），都没有溢出，因此不启动滚动。

示例改用独立场景容器，内部跑马灯居中并限制为最多 200 px；可复制的 QML 同步使用该宽度及 3500 ms 滚动时长。控件原有的溢出判断和减少动效行为保持不变。

- Windows / Qt 6.12.0：Debug、Release gallery 构建通过，gallery qmllint 通过。
- gallery_smoke 与 gallery_contracts 2/2 通过；gallery 契约检查 68/68 通过。
- 新增英文／中文 × 桌面／390 px 窄屏四个行为检查，确认文本溢出、实际位置随时间变化，以及启用减少动效后回到起点并停止。

## 侧栏缩窗后无法展开修复（2026-10-11）

Windows offscreen 复现：从 1200 px 缩到 390 px 后点击导航按钮，抽屉 `opened=true`，但 `position=0`，288 px 面板的背景坐标为 x=-288，仍在窗体外。Qt Drawer 通过 enter／exit 的过渡动作推进 position；CatDrawer 未提供过渡，此前只检查 opened 的测试未发现此问题。

CatDrawer 现在挂载到窗口 Overlay，并通过 position 开关过渡完成展开／收起；减少动效使用零时长过渡，仍完成位置更新。gallery 恢复宽布局时自动收起移动抽屉，较矮窗口的抽屉导航支持滚动。

- Debug／Release gallery 构建、QCatUiKit 与 gallery qmllint 通过。
- 控件八个行为场景通过：四个方向 × 正常／减少动效；在有偏移的父项中验证窗口边缘位置及两次开关。
- 导航八个行为场景通过：中英文 × 390／800 px × 正常／减少动效，高度 540 px；验证点击展开、屏幕内坐标、导航选择、滚动到设置、点击外部／Escape 关闭、恢复宽布局及再次缩窗展开。
- 390 × 540 px 中文导航展开及滚动截图已在 Windows 软件渲染环境检查；修复后 position=1、面板 x=0。
- 完整回归：六个 CTest 测试均通过；gallery 契约检查 75/75 通过。

## 项目标识接入验证（2026-10-11）

导入用户提供的 QCatUiKit 灰猫标识，规范副本保存于 `src/uikit/doc/images/brand/qcat-uikit`。完整 SVG、简化 SVG、ICO 与十个 PNG 共 13 个素材的 SHA-256 均与原始文件一致；没有调整图形、配色或重新生成素材。

- gallery 侧栏与首页使用完整标识，窄屏标题栏使用简化 SVG；统一通过内部 GalleryLogo 组件显示。
- PNG 多尺寸窗口图标随应用嵌入；Windows exe 使用独立 RC 嵌入原始 ICO，已提取并检查实际 exe 图标。
- Debug／Release gallery 构建及 gallery qmllint 通过；gallery_smoke 通过。
- Release 从独立工作目录启动，中文桌面、英文暗色触摸、390 px 中文移动、360 px 英文最窄布局正常加载，截图已检查。
- README 加入完整标识，三张 gallery 文档截图已更新；运行时与构建不依赖 CatCommStudio 的文件路径。

本轮仅涉及品牌资源和展示，未重跑之前已通过的完整控件回归；原生系统窗口标题栏与任务栏的布局未执行 GUI 验收。

## 圆弧抗锯齿验证（2026-10-11）

CatCircularProgress 与 CatBusyIndicator 的 Shape 原先未设置渲染器偏好，gallery 未启用窗口 MSAA；默认 RHI GeometryRenderer 的圆弧边缘存在锯齿。两处 Shape 现在显式优先选择 `Shape.CurveRenderer`，使用其内置抗锯齿；软件 scenegraph 仍由 Qt 自动选择 SoftwareRenderer。相关机制见 [Qt Shape 文档](https://doc.qt.io/qt-6/qml-qtquick-shapes-shape.html#rendererType-prop)。

验证使用只在本地构建的 QQuickRenderControl 探针，在不显示原生窗口的情况下渲染到 D3D11 WARP 纹理，再读取真实像素。白色圆弧、黑色底色、单采样，比较同尺寸控件边缘的中间灰度像素（排除中心文字）：

| 控件 | 修改前：GeometryRenderer | 修改后：CurveRenderer |
| --- | --- | --- |
| 96 px 环形进度 | 0 | 628 |
| 24 px loading 圆弧 | 0 | 113 |

实际渲染器从 1（GeometryRenderer）变为 4（CurveRenderer），两次测试均保持 MSAA=1。[四倍像素放大对比](images/circular-progress-antialiasing.png)已检查。WARP 验证执行了 D3D11 场景图与曲线着色器，不代表已验收用户的物理 GPU 或 Linux 环境。

Debug／Release 库与 gallery 构建通过，QCatUiKit 与 gallery qmllint 通过。qml_contracts、qml_busy_timing、gallery_smoke 3/3 通过，保留此前修复的 900 ms 旋转计时及减少动效行为。未修改进度值、线宽或公开 API。

## 统一完整 Logo 更新验证（2026-10-11）

同步用户更新的标识。新版 16／24／32 px PNG 与 ICO 帧统一使用完整灰猫、蓝色底座和组件角标；原始完整 SVG 及 48–2048 px PNG 未变化。当前 SVG、ICO 与十个 PNG 共 12 个文件的 SHA-256 均与来源一致。

移除旧简化 SVG 及其资源注册，GalleryLogo 在所有位置使用完整标识；compact 仅调整布局尺寸。品牌目录和 gallery 说明已更新。Windows RC 增加 ICO 的 OBJECT_DEPENDS，图标替换后会重新编译；此次 Debug／Release 日志均确认 RC 对象重新生成。

Debug／Release gallery 构建及 gallery qmllint 通过，gallery_smoke 通过。Release 中文桌面、英文暗色触摸、390 px 中文移动及 360 px 英文最窄布局加载成功；最窄标题栏及实际 exe 提取出的 32 px 灰猫图标已检查，三张文档截图已更新。

本轮仅更新品牌素材、显示与构建依赖，未重跑完整控件回归。运行仍使用本项目的嵌入资源，不依赖 CatCommStudio。

## Fluent-Qt 非 Charts 组件对齐验证（2026-10-11）

按用户指定的顺序完成现有控件打磨、基础输入补齐和其他非 Charts 组件扩展。参考目录中的 78 种组件均有 Qt Quick 实现和 gallery 页面，完整映射及使用差异见 [REFERENCE_COMPONENTS.md](REFERENCE_COMPONENTS.md)。当前模块有 98 个 Cat 前缀公开 QML 组件，gallery 有 15 个分类；原有类型及 API 保留。

现有控件调整包括按钮文字和图标空间、禁用输入框、减少动效时的进度显示、圆弧边界、工具提示换行及窄屏小按钮。补齐复选框、单选按钮、开关、滑块和下拉框，再加入集合视图、菜单、日期选择、对话框、导航、状态展示、文本输入和窗口组件。空间视图使用兼容 Qt 6.8 的矩阵透视，显式平面模式会传递到内部面板。

| 检查 | 结果 |
| --- | --- |
| Debug / Release | 库与 gallery 均构建成功；两者 qmllint 无告警 |
| CTest | 六个测试组最终均通过；修正后复跑 qml_contracts 与 gallery_contracts 2/2 通过，其余四组已通过 |
| QML 控件 | 169/169 通过，包含 98 个类型的导入／创建及输入、布局、交互、动画与边界检查 |
| gallery | 419/419 通过，包含英文桌面、中文桌面和 360 px 中文窄屏的 98 × 3 个页面，以及 98 份示例的实际创建检查 |
| 运行时国际化 | 中英文各 525 条文案已完成；保持单行、多行、页签草稿、消息、日期和弹出层状态；搜索覆盖新增日历选择器 |
| 颜色／日期／对话框弹出层 | 中文 390 px 窄屏实际展开截图已检查；RGB／透明度显示标签、选择行为专项通过 |
| 安装与外部消费 | 独立 Release 安装成功；外部应用通过 find_package 构建并加载新增控件、只读 TreeModel、窗口及空间视图，返回 0；SpatialTransform.js 随模块安装 |
| 展示截图 | Release 中文桌面、英文暗色触摸、中文窄屏树视图已检查并更新文档图片；图标继续使用用户提供的完整标识 |

GalleryPages 连续切换组件检查导航与页面生命周期；GalleryContracts 另用独立窗口验证语言切换、输入与草稿保留、抽屉和弹出层。代码示例检查补齐旧片段的根对象，修复图片资源地址，能够在实际 QML 引擎中创建。

额外使用 QQuickRenderControl 和 D3D11 WARP 渲染空间面板：相机距离由 300 改为 1200 时，同一局部点由 (246.455, 222.547) 改为 (239.545, 217.349)；开启 forceFlat 后回到 (240, 220)，内部面板的平面模式及缩放同时恢复。该探针无原生可见窗口，并验证了透视与平面切换的真实场景图路径。

验证环境为 Windows 11、MSVC 2022、Qt 6.12.0。未执行 Linux、物理 GPU、Qt 6.8 运行时或系统窗口移动／缩放的实机验收。组件种类和主要使用场景按 QML 方式对齐；不声明 Widgets C++ API 完全兼容，不包含 Charts、Windows Mica/Acrylic 或 Widgets 嵌入。

## 原生图表迁移验证（2026-10-12）

从 `QCatUiKit.Charts` 测试版合入 ChartItem／ChartAxis／ChartSeries 与私有采样／坐标算法，QML 入口统一为 CatChart／CatChartAxis／CatChartSeries。未增加 Qt Charts、Qt Graphs、WebEngine 或 Widgets 依赖。当前 101 个 Cat 前缀公开类型、16 个 gallery 分类；图表预览包含原有 15 个配置。两种语言各 662 条翻译均完成。

环境为 Windows 11、MSVC 2022、Qt 6.12.0、CMake + Ninja。常规 UI 检查使用 offscreen 与 software，硬件渲染路径用 D3D11 WARP 离屏目标；没有打开可见窗口。

| 检查 | 结果 |
| --- | --- |
| Debug／Release 共享库与 gallery | 均构建通过；QCatUiKit_qmllint／qcat-gallery_qmllint 无警告 |
| 完整 CTest | 7/7 测试组通过，196.32 s；QML 184/184，gallery 465/465 |
| 图表核心收尾复测 | 25/25，包括原有 22 项、主题销毁／单例清理与线程批次校验 |
| 图表 QML 收尾复测 | 15/15，包括 10 种图表的实际图像、主题、键盘、禁用交互、追加与原始值 |
| 图表 gallery 收尾复测 | 34/34，包括 15 × 2 中英文配置、实时追加／暂停／切换、语言保持数据与手动范围 |
| D3D11 WARP | 15 个配置均生成实际图像，渲染目标为单采样；样式收尾后环形图复测通过 |
| 曲线边缘覆盖率 | 在白色环形扇区与黑色背景的内／外边缘检测到 231 个灰色覆盖像素，未依赖 MSAA |
| 安装后独立消费者 | 只用 stage 安装目录的头／导出库／插件，C++ 环形缓冲与 QML 图表创建通过 |
| 最小配置 | BUILD_TESTING=OFF、QCATUIKIT_BUILD_EXAMPLES=OFF 配置通过，无 LinguistTools／Test 要求 |

主题、字体、自动系列调色板、软件绘制回退和键盘交互接入后，原始容量、批次拒绝、峰谷采样、混合序列、正负堆叠、原始点提示、范围保持及对象销毁行为均保留。样式与生命周期收尾后复测了上述图表专项；完整回归结果保留在本地构建目录。新增 [桌面](images/gallery-charts-desktop.png) 与 [窄屏](images/gallery-charts-mobile.png) 实际 gallery 截图。

未执行 Linux、Qt 6.8 运行时、物理 GPU、Vulkan／Metal／OpenGL 后端或触摸双指实机验收。软件后端回退包含整张图表图像，不能用其速度代替原生几何的性能测量；未将旧独立演示的 FPS 记录移作本版本成绩。
