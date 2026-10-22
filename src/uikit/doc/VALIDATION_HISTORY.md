# 历史验证记录

[文档索引](README.md) · [当前验证方法与最近结果](VALIDATION.md)

以下保留各次执行时的平台、结果、失败和范围限制。记录中的“当前”“待实现”“本轮”仅指对应日期，不代表最新接口或待办；按日期和检查范围阅读，不将不同平台结果合并。忽略目录中的日志路径是当时的本地产物位置，仓库不保证携带这些文件。

## Dock 原生 Wayland 独立路径（2026-10-22）

以用户还原后的代码为基线，按实际 QPA 插件选择平台路径：原生 Wayland 使用独立的原生拖放／系统移动适配器，XCB（包括 XWayland）、Windows、macOS 保留既有 MouseArea、全局坐标命中和随动缩略窗。Gallery 未添加平台强制设置。Wayland 图标在原生拖放开始前同步捕获，停靠命中改用接收窗口的局部坐标；支持顶层拖动扩展时临时浮动并交给合成器移动，取消时恢复布局。

新增适配器没有 Qt Wayland Compositor／Qt Wayland Client 模块链接。可选 Wayland C 开发头用于协议探测，MIT 许可 `libwayland-client` 仅在 Wayland 进程动态加载。`readelf -d` 检查库的 NEEDED 项，没有新增 Wayland 传输库或 Qt Wayland 模块依赖。Qt 的 dock-window 私有 MIME 桥接与无扩展时的操作差异已在 DOCKING 记录。

实际环境：Ubuntu 26.04、Qt 6.12.0 GCC Release，现有 Desktop Release 构建目录，窗口测试均使用 software 后端。

- 构建、`QCatUiKit_qmllint`、`qcat-gallery_qmllint` 通过；最终 `ctest --output-on-failure --timeout 300 --parallel 2` 为 18/18，耗时约 124 秒。离屏浮动专项为 25/25。新增原生专属检查在 offscreen 下跳过，不能将 CTest 全通过解释为原生拖动验收。
- 直接使用 `QT_QPA_PLATFORM=wayland` 运行 `tst_docknative`：5/5，日志确认 `toplevel-drag: true`；检查源面板像素已进入缩略图、窗口局部事件、有效／无效落点与跨工作区令牌隔离。事件检查注入 Qt 拖放事件，没有执行完整的 compositor 拖动。
- 同一 Wayland 会话运行 `tst_dockwayland.qml`：6/6，检查主窗偏移后的局部坐标转换、浮窗合并、取消恢复、面板内容保留和系统移动权限。Qt 在测试启动前有 text-input-v3 `disableSurface ... but ... is focused` 警告；未修改输入法环境，测试函数中的严格警告检查通过。
- 另建未定义协议探测编译宏的独立适配器测试构建，在 Wayland 下 5/5，日志为 `toplevel-drag: false`。这验证缺少可选探测代码时适配器仍可编译与路由，不证明系统移动降级手势已完成真机验收。曾尝试全局禁用 PkgConfig 来模拟缺失依赖，但本机 Qt GuiPrivate 的 XKB 查找依赖 PkgConfig，配置即失败，故未把该尝试当作组件降级构建结果。
- XCB 平台隔离测试为 3 通过、2 跳过：QPA 为 `xcb`、协议支持为 false，不安装原生事件过滤器，也不发起系统移动或原生拖放。原有 XCB 浮动全组最终对照为当前代码 24/25、独立 HEAD 基线 23/25；两者均在既有 `test_pointerMergeIntoFloating` 的浮动源场景出现命中断言或 y 坐标相差 34 的失败。更早一次当前全组为 22/25，不将这些不稳定结果宣称为 XCB 全通过。对照基线用 `git archive HEAD` 导出到临时目录独立配置构建，未改动工作区或 Git 历史。
- `python tools/check_docs.py` 与 `git diff --check` 通过；公开 C++ 头和已有控制器 ABI 未修改。

本次没有真实鼠标输入序列，因此尚未验收原生 `QDrag::exec()` 的完整生命周期、拖放图标在真实拖动中的显示、扩展附加窗口的实际移动以及系统移动降级手势。Windows/macOS、最低 Qt 6.8、其他合成器、GPU 后端和混合 DPI 本次也未实机复测。重复方法与手工清单见 VALIDATION 的 Dock 专项。

## 全库文档核对与整理（2026-10-21）

检查仓库中的全部 34 份 Markdown，包括根入口、专题、Gallery、技能参考和品牌资源说明。按下表核对现行文档的用途、接口入口、过期内容、重复说明和阅读顺序；历史文件保留原有日期及平台范围，不把旧结果改写为本次执行。

| 范围 | 文档 | 数量 |
| --- | --- | --- |
| 根入口与约定 | README.md、README.en.md、CONTRIBUTING.md、AGENTS.md | 4 |
| 控件和数据接口 | COMPONENTS、THEMING、MODELS、SLIDERS、ATTITUDE、DOCKING | 6 |
| 图表 | CHARTS、CHARTS3D、CHARTEXTENSIONS、CHARTSTATISTICS、CHART_RENDERING | 5 |
| LED | LEDPREVIEW、LEDWORKFLOW | 2 |
| 构建与维护 | BUILDING、ARCHITECTURE、ABI、VERSIONING、MAINTENANCE | 5 |
| 验证与截图 | VALIDATION、VALIDATION_HISTORY、SCREENSHOTS | 3 |
| 文档导航与说明 | 文档 README、AGENT_UI、品牌 README | 3 |
| Gallery | examples/uikit/gallery/README.md | 1 |
| UI 技能 | SKILL.md 与四份 references | 5 |

对照模块 CMake 的 32 个安装头，以及对应公开声明、QML 文件和相关实现，补充坐标轴 tickCount、LED 布局／仓库／播放器／混合器／设备输出接口，澄清同步调用、后台准备、停止发送、尺寸与线程边界。删除组件目录重复三维段落；ABI 的逐批演进说明已在本历史文件存在，当前指南改为规则与边界。2026-10-12 的 LED 圆片基准从预览指南原样移至本文件末尾。图表与 Gallery 的追加说明归回对应章节，补上任务导航、术语解释、片段上下文和 Dock 技能入口。未修改库运行代码或公开接口。

实际检查环境为 Windows 11、Qt 6.12.0 MSVC Release，复用 `build/dock-validation`，测试使用 offscreen/software：

- `python tools/check_docs.py` 通过：34 份文档的本地链接、锚点、围栏、重复正文与 119 个 Cat 类型覆盖；对应专题包含 360 个 Q_PROPERTY 和 168 个 Q_INVOKABLE 声明的名称。这是名称覆盖，不等于所有签名、信号和行为均已自动验证。
- 从当前指南提取 35 个 QML 对象示例，逐项创建并等待事件循环；Qt Quick Test 37/37（含初始化／清理），没有 QWARN、ReferenceError 或 TypeError。无 import 的片段补标准导入，并列对象加 Item 容器；处理器片段不当作独立文件加载。
- 文档默认值专项 6/6（含初始化／清理）：主题三种密度／减少动效、模型副本及消息默认值、轴刻度默认值与限制、LED 播放／混合／发送默认参数。
- 检查工具的重复正文正例会被拒绝，围栏内重复代码不被误报；临时屏蔽主题专题时，公开接口名称缺失会被拒绝。`git diff --check` 通过。
- 临时提取脚本、专项和日志位于忽略的 `build/doc-examples/`、`build/check_doc_examples.py`、`build/audit_api_mentions.py`；重复检查步骤见 VALIDATION 的“文档与示例检查”。

本次不重新构建产品或重跑完整 CTest、GPU、安装部署及其他平台；QML 示例创建不代表全部交互、像素或性能验收。外网链接未联网检查；带日期截图仍按原采集环境标注。全库已有产品测试失败继续保留在最近结果摘要中。

## Dock 接口与文档整理（2026-10-21）

核对 DockController 头文件、实现、CatDockWorkspace／CatDockPanel 和相关测试。专题文档覆盖 16 个可调用方法、3 个控制器属性和 3 个信号；补充工作区／面板属性表、默认值、关闭确认、布局快照字段、右侧停靠比例及内部实现边界。公开头补充 ratio 方向说明，未修改接口签名或运行行为。

清理 ARCHITECTURE 的“跨窗口拖拽待实现”和 VALIDATION 中重复的旧阶段摘要；旧执行证据已在本历史文件保留。同步中英文首页、组件目录、ABI 和贡献指南。Dock 专题按接入、操作、接口、保存恢复、浮动回停、限制的顺序编排。

- Windows 11、Qt 6.12.0 MSVC Release，复用 `build/dock-validation`；从专题文档提取最小 QML 示例，用 Qt Quick Test 实际创建并检查初始分割、保存／关闭／恢复、右侧 0.75 比例。offscreen/software 3/3（含初始化／清理）通过。
- 本地辅助检查确认上述 16 个方法、3 个属性、3 个信号在文档中均有说明；这属于 Dock 范围核对，不代表全库每个接口均逐项复核。
- `python tools/check_docs.py` 及 `git diff --check` 通过。全库产品测试沿用前次记录，本次未重跑构建、完整 CTest 或其他平台。
- 临时提取脚本、测试和日志位于忽略的 `build/verify_dock_docs.py`、`build/tst_documented_dock.qml`、`build/dock-doc-example.txt`；可重复步骤见 VALIDATION 的“文档与示例检查”。

## Dock 外边缘整列停靠与子树回停（2026-10-21）

用户复现：属性面板浮动后拖回编辑器右方向，属性只占编辑器高度，日志仍横跨其下。原因是命中和控制器均只处理标签组目标。
新增 `dockToWorkspace(id, position, ratio = 0.5)`，四周外边缘指示器相对于整棵主布局分割；组内十字继续只拆分标签组。
交互参考源面板尺寸设置比例，右侧形成完整高列，日志保留在左侧子布局内。

另修复回停定位：浮动时保存可选 `returnNode` 子树锚点，回停优先恢复到整个相邻子树旁，锚点消失时回退到原面板。v2 旧布局无此字段仍可读，字段类型错误则原子拒绝；未增加布局版本。

- Windows 11、Qt 6.12.0、MSVC 2022 Release，`build/dock-validation` 构建及库／Gallery qmllint 通过。
- 控制器专项 16/16：四外边缘、根节点为空、唯一面板原地操作、主布局源的移除与折叠、比例／无效请求、保存恢复、子树锚点缺失与旧 v2 兼容。
- 完整 Gallery Dock Windows 原生窗口 15/15：属性拖到四周外边缘、Esc 取消、左右整列／上下整行的几何、保存恢复，以及回停保留原宽高／位置。
- 工作区 12/12、浮动 25/25 通过。截图复核右侧预览覆盖全高，释放后属性为整列，日志右边界止于该列左侧；本机临时加载 Segoe UI 进行截图。
- 全库 CTest 15/17；失败仍为此前 LED 和下拉框宽度两组。全套 QML offscreen 383 通过、3 失败，仍为三个 `SelectionControls::test_compactComboDisplayFits` 密度用例；文档与差异空白检查通过。
- 安装后独立 Dock 消费者编译／运行通过，覆盖新 API 的 C++ 和 QML 调用及三种公开头。原 Release 的 1384 个导出符号全部保留，候选增加 1 个；从无 DLL 的目录运行原 Release 旧测试 EXE，候选 bin 置于 PATH 首位，10/10 通过。公开对象布局和已有函数签名保持不变。
- 日志：`build/dock-outer-controller.txt`、`build/dock-outer-gallery.txt`、`build/dock-outer-floating.txt`、`build/dock-outer-workspace.txt`、`build/dock-outer-consumer.txt`、`build/dock-outer-old-abi.txt`；截图 `build/dock-outer-preview-right.png`、`build/dock-outer-result-right.png`。

用户的常用 Release Gallery 仍在运行并占用 DLL，未终止程序；修复已在独立构建目录完成，常用目录需关闭 Gallery 后重新构建。原生测试使用 QtTest 合成鼠标及 software 渲染，不替代多屏 DPI／Wayland 验收。

## Dock 跨窗口随动缩略预览（2026-10-21）

原拖动名称浮层受主工作区裁剪，鼠标离开主窗口后没有持续可见反馈。本轮增加内部 `DockDragPreview.qml` 原生 ToolTip 窗口，使用输入穿透／不接收焦点标志，按全局坐标跟随鼠标；源标签淡化，浮动目标标签栏高亮。
预览最多捕获一张 480×320 的等比内容图，保留捕获结果对象直到取消／释放；通过拖动代次忽略异步迟到结果，销毁工作区同时销毁预览窗。原布局仍只在释放时提交。

- Windows 11、Qt 6.12.0、MSVC 2022 Release；常用 Release 及独立验证目录均构建成功，库／Gallery qmllint 通过。
- 浮动专项 Windows 原生窗口 25/25，包含全局位置连续跟随、真实缩略图加载、源焦点项保持、窗口标志、目标标签栏反馈、Esc／释放／销毁和两种来源跨窗口合并；亮暗与三种密度均覆盖。
- 浮动专项 offscreen 检查通过，新增测试等待原生窗口初始几何同步后再比较取消布局，避免将离屏平台的两像素边框定位归因于拖动。
- Gallery Dock Windows 原生窗口 10/10；截图复核亮色／暗色和 Mobile 预览的标题、内容比例、边框以及目标标签栏高亮。本机临时加载 Segoe UI 字体用于截图，未打包字体。
- 工作区 Windows 原生窗口 12/12；全库 CTest 15/17，仍为此前 LED 与下拉框宽度两组失败。全套 QML offscreen 383 通过、3 失败，失败仍为三个 `SelectionControls::test_compactComboDisplayFits` 密度用例。
- 日志：`build/dock-drag-preview-native.txt`、`build/dock-drag-preview-offscreen.txt`、`build/dock-drag-preview-gallery-native.txt`、`build/dock-drag-preview-lint.txt`；示例截图 `build/drag-ghost-floating-density-0.png`、`build/drag-target-floating-density-0.png`。

本轮采用随动缩略预览与释放后提交，不在拖动中重新挂载业务内容。系统原生标题栏移动、浮动窗内部拆分、多屏不同 DPI 和 Wayland 的行为不由本轮截图或合成鼠标测试证明。

## Dock 分隔条鼠标高亮复位（2026-10-20）

截图中的分隔条使用 `activeFocus` 保持整条蓝色，导致鼠标点击后移出仍像选中；同时 MouseArea 在尺寸位置更新后可能保留悬停状态。
改用 Control 的悬停状态，区分鼠标焦点和键盘焦点提示；鼠标点击不留下持续高亮，方向键调整仍显示键盘提示。

- Windows 11、Qt 6.12.0、MSVC 2022 Release，`build/dock-validation` 构建通过。
- 工作区专项 Windows 原生窗口与 offscreen 各 12/12，新增点击／移出、键盘聚焦／调整及再次鼠标操作回归；Gallery Dock Windows 原生窗口 10/10。
- 日志：`build/dock-splitter-native.txt`、`build/dock-splitter-test.txt`、`build/dock-splitter-gallery.txt`。
- 常用 Release DLL 被运行中的 Gallery 占用，链接失败（LNK1104）；未终止用户程序，需退出后重新构建才能更新该目录。

## Dock 标题、标签与停靠指示器（2026-10-20）

环境：Windows 11、Qt 6.12.0、MSVC 2022 Release；software，Windows 原生平台及 offscreen。
将通用按钮式标签改为双行标题／扁平标签，增加标题栏浮动／关闭工具、标签选中下划线、内容焦点边框、分隔条悬停反馈、拖动名称浮层和五方向停靠指示器。浮动目标仅展示中央合并。
方向格命中优先于组边缘区域，预览与实际提交位置一致；控制器和版本 2 布局结构保持不变。

- 工作区专项 Windows 原生窗口 11/11，含五方向格的选择、预览与提交后面板几何关系。
- 浮动专项 Windows 原生窗口 23/23，覆盖亮暗／三种密度下跨窗口迁移、内容、焦点、弹层与关闭生命周期。
- Gallery Dock Windows 原生窗口 10/10，包含完整 Main 嵌套滚动、标题拖出、标题栏浮动工具及跨窗口合并。
- 常用 `Desktop_Qt_6_12_0_MSVC2022_64bit_Release` 目录已重建；库／Gallery qmllint、文档检查及 `git diff --check` 通过。全库 CTest 15/17，仍为此前 LED／下拉框宽度失败，Gallery 全套及 Dock 控制器组通过。
- 检查亮色／暗色 Desktop、暗色 420 像素 Mobile 实际 GalleryDockPreview 截图，包含标题／工具、长标题省略、标签滚动、分隔条、五向指示与半区预览。截图程序从本机加载 Segoe UI 字体文件以绕过测试环境系统字体方框；未将字体文件加入仓库。
- 全套 QML offscreen 380 通过、3 失败，失败仍为已有 `SelectionControls::test_compactComboDisplayFits` 的三个密度用例。
- 本地截图：`build/dock-chrome-light.png`、`build/dock-guides-dark.png`、`build/dock-chrome-narrow.png`；执行日志 `build/dock-ui-workspace.txt`、`build/dock-ui-floating.txt`、`build/dock-ui-gallery.txt`、`build/dock-ui-all-qml.txt`、`build/dock-ui-ctest.txt`。

本轮改善标题／标签与拖动反馈，不代表与其他 Dock 框架功能对等。浮动窗内部拆分、边缘自动隐藏、跨独立工作区停靠和多屏不同 DPI／Wayland 验收仍未完成。

## Dock P3 跨窗口标签合并（2026-10-20）

环境：Windows 11、Qt 6.12.0、MSVC 2022 Release；`build/dock-validation`。
原生窗口使用 Windows 平台插件及 software 渲染，鼠标事件由 QtTest 合成；另跑 offscreen/software。

- 主工作区→已有浮动宿主、浮动宿主→其他浮动宿主均支持标签合并；保留目标几何和回停提示，清理源空宿主。
- 补齐整窗目标预览、跨窗口 Esc、空主工作区拖回单个面板、目标失效／权限检查；修复布局更新覆盖尚未保存的原生移动／缩放。
- 控制器专项 10/10，涵盖原子拒绝浮动目标四方向分割、保存恢复、宿主清理和回停。未增加导出符号或改变公开对象布局，布局格式仍为版本 2。
- 浮动专项 Windows 原生窗口 23/23，覆盖两种来源、亮暗主题、Desktop／Touch／Mobile 密度、预览／Esc、内容保留和生命周期。
- Gallery Dock 专项 Windows 原生窗口 8/8，包含完整 Main 页面的两种跨窗口来源和外层滚动位置保持。
- Release 构建、库／Gallery qmllint、文档检查及 `git diff --check` 通过；全库 CTest 15/17，两个失败组为下述已知区域。
- 全套 QML offscreen 日志 375 通过、3 失败；失败仍为 `SelectionControls::test_compactComboDisplayFits` 的三个密度用例。LED 专项仍有 `layoutMappingAndValidation` 的 `m.applyNow(c)` 失败；均为本轮之前已记录且未修改的区域。
- 预览截图检查亮色 Desktop 和暗色 Mobile 的 320×240 浮动目标，整窗覆盖正确；离屏字体方框问题仍存在，不作为文字视觉验收证据。
- 日志：`build/dock-controller-result.txt`、`build/dock-floating-native-p3.txt`、`build/dock-gallery-native-p3.txt`、`build/dock-all-qml-p3.txt`、`build/dock-led-p3.txt`。

浮动宿主仍为单个标签组，不支持内部四方向分割；跨独立工作区拖动、边缘自动隐藏、人工原生鼠标、多屏不同 DPI／热插拔和 Wayland 尚未验证或实现。接口和复现方法见 [Dock 工作区](DOCKING.md) 与 [验证说明](VALIDATION.md)。

## Dock 浮动标签拖回主工作区（2026-10-20）

同日追加修复：主工作区标签拖出边界并释放可创建浮动宿主；原先该路径仅取消拖动。
浮动专项更新为 16/16，工作区专项 6/6，覆盖真实鼠标事件路径、释放前不改变布局、Esc、禁止浮动、内容保留及回停。
验证环境仍为下述 offscreen/software，原生系统鼠标抓取尚未实机验收。

再次追加 Gallery 集成修复：完整 `Main` 页面的外层滚动容器会抢占标签鼠标抓取，独立预览测试未覆盖该嵌套。
为主工作区及浮动标签 MouseArea 设置 `preventStealing`；新增完整 Main 页面和独立预览拖出测试，断言页面不滚动且创建浮动宿主。
完整页面测试修复前失败、修复后通过；Gallery Dock 在 Windows 原生窗口平台和 offscreen 下均 6/6，浮动专项 offscreen 16/16，构建与两项 qmllint 通过。
原生测试使用 QtTest 合成鼠标事件及 software 渲染，不等同于人工鼠标、多屏 DPI 或 Wayland 验收。日志 `build/dock-gallery-native.txt`、`build/dock-gallery-drag.txt`。

环境：Windows、Qt 6.12.0、MSVC 2022 Release，offscreen/software；独立构建目录 `build/dock-validation`。
原 Release DLL 被运行中程序占用，未终止该程序，另行配置并完成全量构建。

- 浮动 QML 专项 15/15：中央合并、四边分割、全局坐标命中、宿主遮挡、鼠标拖动、Esc、锁定／移动权限、工作区外释放、内容和窗口生命周期。
- Gallery Dock 专项 4/4；检查左侧分割预览截图，覆盖目标半区。离屏环境字体显示为方框，未据此宣称文字视觉验收完成。
- 库和 Gallery 的 qmllint 通过；文档检查通过，同时修正组件目录总数 117 → 119。
- 全库 CTest 15/17 通过。`led_pipeline` 的 `layoutMappingAndValidation` 在 `m.applyNow(c)` 失败；`qml_contracts` 的三个 `SelectionControls::test_compactComboDisplayFits` 密度用例失败。直接运行 QML 日志为 366 通过、3 失败；这两个失败区域未在本次修改。
- 日志：`build/dock-floating-result.txt`、`build/dock-gallery-result.txt`、`build/dock-ctest.log`、`build/dock-all-qml.txt`、`build/dock-led-result.txt`。

未验证原生 Windows 跨窗口鼠标抓取、Wayland、多显示器和不同 DPI；当前目标仅限原主工作区，拖入浮动宿主尚未实现。复现及手工验收见 [验证方法](VALIDATION.md)。

## 姿态球与设备无关体感输入（2026-10-19）

环境：Ubuntu 26.04 x86_64、Qt 6.12.0 Release；software/offscreen，以及 XCB 上的实际 Vulkan/OpenGL。
Vulkan 日志确认 AMD Radeon 780M Graphics / RADV PHOENIX，Mesa 26.1.6；未开启 Khronos validation layer。
新增公开 CatAttitudeIndicator、CatTiltControlPanel 和 TiltController，接口与输入坐标约定见 [姿态与体感](ATTITUDE.md)。

| 检查 | 结果 |
| --- | --- |
| 全库 CTest | 16/16 通过，包含新增 tilt_controller |
| TiltController 专项 | 23/23；校准、任意零点、四元数符号、前倾方向、死区/限幅/曲线、采样间隔、非法输入、事件循环延迟、超时及授权撤销 |
| 全套 QML 契约 software | 349/349；含最终参数边界回归 |
| 新增姿态/体感 UI Vulkan DPR 1 / DPR 2 / OpenGL DPR 1 | 各 20/20；实际像素方向、圆形边界、固定标记、数据无效、鼠标/触摸/键盘、销毁释放及窄屏 |
| Gallery 新增流程 software / Vulkan DPR 2 / OpenGL DPR 1 | 各 4/4；独立飞机/手机数据源、模拟校准与控制、超时、翻译及参数绑定 |
| Gallery 全量 software / Vulkan DPR 1 | 各 811/811，0 失败、0 跳过；14 个 QML 文件、120 个目录页面的英文/中文/窄屏覆盖及既有交互 |
| 构建、静态检查与翻译 | Release、库和 Gallery 的 qmllint、git diff --check 通过；两种语言各 1542 条翻译，无 unfinished |
| 安装与兼容性 | 新独立安装消费者成功创建三种新类型、调用 C++ 控制器；历史图表消费者不重新编译加载新库成功；此前 745 个 QCatUiKit 函数符号全部保留 |
| 依赖边界 | 安装包配置与链接依赖没有 Qt Sensors 或 MAVLink；新增代码保持 Apache-2.0 |

全库回归后，面板参数范围和到达时钟启动的局部修正分别以全套 QML、相关后端、控制器和 Gallery 新增流程补跑。
最终全量 Vulkan 为 811/811；期间有一次全量复跑在既有 `GalleryScrolling::test_nestedDataView(table)`
列宽拖拽断言失败（810/811）。该文件和表格产品代码本轮未修改；保留失败日志，单独复查滚动 10/10，随后全量 811/811。
这记录一次未稳定复现的测试波动，不据此宣称已消除该问题。

实际截图检查覆盖飞机/手机两个页面、英文亮色桌面和中文暗色 Mobile，共四个 software 场景及四个 Vulkan 窗口场景。
XCB 窗口高度受桌面工作区限制，Vulkan 截图代表实际可见视口；software 长窗口检查完整操作区。
检查了角度读数底板、圆形天地、触摸目标和窄屏折行。截图不代表 Android 真机或实际传感器验证。

本地日志位于忽略目录 `build-release/attitude-validation/`；安装与历史消费者使用临时安装前缀。
未测试 Android 实机、Qt 6.8、Windows/macOS、Wayland、其他 GPU、真实传感器或飞控。
传感器采集、屏幕坐标转换、MAVLink 发送和最终控制权/失效策略均由应用实现，不属于本次库交付。

复现入口（替换构建目录和 Qt 运行环境）：

```sh
cmake --build build-release --parallel
cmake --build build-release --target QCatUiKit_qmllint qcat-gallery_qmllint
ctest --test-dir build-release --output-on-failure

QT_QPA_PLATFORM=offscreen QT_QUICK_BACKEND=software \
  build-release/tests/auto/qml/tst_qml -import build-release/qml \
  -input tests/auto/qml/data/tst_attitude.qml

env -u QT_QUICK_BACKEND QT_QPA_PLATFORM=xcb QSG_RHI_BACKEND=vulkan QSG_INFO=1 QT_SCALE_FACTOR=1 \
  build-release/tests/auto/gallery/tst_gallery -import build-release/qml
```

独立安装消费者见 `tests/auto/cmake/attitude-consumer`，通过安装包的 `QCatUiKit::QCatUiKit` 链接，
不引用源码树或 Gallery 模块。

## Gallery 全量 Vulkan 回归（2026-10-19）

环境与下一节相同：Ubuntu 26.04 x86_64、Qt 6.12.0 Release、XCB、AMD Radeon 780M／RADV、Mesa 26.1.6。此前 Gallery 的 Vulkan 检查仅有三个图表视觉场景；本次直接运行 `tst_gallery` 的全部 13 个 QML 测试文件（含新增 LED 切页回归），日志确认实际窗口使用 Vulkan 和 AMD 硬件。

| 检查 | 结果 |
| --- | --- |
| Gallery 全量 Vulkan DPR 1 | 799 通过，0 失败，0 跳过 |
| Gallery 全量 Vulkan DPR 2 | 799 通过，0 失败，0 跳过 |
| Gallery 全量 software／offscreen | 799 通过，0 失败，0 跳过 |
| OpenGL 定向回归 | 数值输入 4、滚动 10 项全部通过 |
| LED 生命周期专项 | Vulkan DPR 2 连续切页 45 次，三个场景及初始化／清理共 5 项通过，无委托卸载警告；修复前同脚本为 4 通过、1 失败。原 LED UI 契约 software 22 项通过 |
| 构建与检查 | Release 构建、库及 Gallery 的 qmllint、git diff --check 通过 |

完整目录包含 118 个页面，每页检查英文桌面、中文桌面及中文 360px 窄屏，共 354 个页面用例；另检查全部 118 个可复制示例。其余覆盖图表交互、M4 扩展、PNG 导出、图表导航与区间、数据示例、密度、语言与弹出层、滚动及滑块。新增 `tst_lednavigation.qml` 在三个场景下反复于首帧后卸载 LED 页面，纳入以上全量运行。总数包含每个测试文件的初始化／清理。

首轮 DPR 1 为 791 通过、3 失败。两项数值输入断言在单独及图表整组复跑中均通过，测试现显式激活 Gallery 并确认编辑器焦点。DPR 2 诊断进一步确认：窗口 `active` 已为 true 时，`activeFocusItem` 仍可能为空；相关键盘测试增加焦点就绪前置检查。首轮 DPR 2 含输入／Escape 断言失败，并在 300 秒超时后中止，该次不计为完整验证。

表格列宽拖动在等待窗口悬停帧完成后通过；测试补齐这一同步，并在重新选择已显示的静态页面或设置原有尺寸时显式请求更新帧。保留实际键入、失焦提交、列宽增量与嵌套拖动断言。

完整 DPR 2 运行还复现了 LED 页面卸载警告 `DelegateModel::cancel: index out range`，与此前历史记录中的现象一致；独立连续切页用例再次复现。`LedStorageBrowser` 现于销毁时先解除 GridView 模型，取消仍在异步创建的缓冲委托，避免创建上下文先被销毁。此修复不改变公共 API 或正常浏览／选择行为；最终全量及专项运行均未再出现该警告。本轮没有重跑整库 CTest，同日 M5 的整库记录仍按其原有范围解释。

真实图形会话下复现（按本机 Qt 安装设置库搜索路径和 GPU ICD）：

```sh
env -u QT_QUICK_BACKEND \
  QT_QPA_PLATFORM=xcb QSG_RHI_BACKEND=vulkan QSG_INFO=1 QT_SCALE_FACTOR=1 \
  VK_DRIVER_FILES=/usr/share/vulkan/icd.d/radeon_icd.json \
  build-release/tests/auto/gallery/tst_gallery -import build-release/qml -maxwarnings 100000
```

增加 `QT_SCALE_FACTOR=2` 运行高 DPI 全量组合。CTest 中的 `gallery_contracts` 默认强制 offscreen／software，不能用其结果代替上述 Vulkan 验证。日志、首轮失败记录及矩阵脚本保存在忽略目录 `build-release/chart-vulkan/`，最终全量日志为 `gallery-full-dpr1-final.txt`、`gallery-full-dpr2.txt`、`gallery-full-software-final.txt`；汇总见 `gallery-validation-summary.json`。

“全量通过”指现有 Gallery 自动测试及目录页面均通过，不表示穷举每页的所有状态或逐像素人工验收；人工视觉检查仍为下一节记录的三个图表场景。本次未扩大到其他操作系统、GPU／驱动、Wayland 或 Qt 版本。

## Vulkan 图表复测与裁剪修复（2026-10-19）

环境：Ubuntu 26.04 x86_64、Qt 6.12.0、Release、XCB，AMD Radeon 780M Graphics（RADV PHOENIX）、Mesa 26.1.6。固定 Radeon ICD，日志确认实际创建 Vulkan QRhi 并选中该硬件；设备支持 Vulkan 1.4.354，Qt 请求 Vulkan 1.4。

首次实测发现二维／三维数据层为空，PNG 也缺少数据。原因是两个私有渲染器在绑定图形管线之前设置 viewport，Qt Vulkan 后端因缺少当前管线而未初始化默认 scissor。现改为先绑定管线再设置 viewport；二维逐绘制命令覆盖普通和实例化管线。此修复不修改公共 API、ABI 或着色器。

| 检查 | 结果 |
| --- | --- |
| 修复前复现 | Vulkan DPR 1：二维 29 通过／163 失败，三维 3 通过／19 失败，PNG 导出 5 通过／2 失败 |
| Vulkan DPR 1 | 二维 192、三维 22、PNG 导出 7 项全部通过 |
| Vulkan DPR 2 | 二维 192、三维 22、PNG 导出 7 项全部通过 |
| OpenGL 回归 | 同机 DPR 1 二维 192、三维 22 项全部通过 |
| Gallery 视觉与导出 | 实际 Vulkan 的 1200px 浅色英文、420px 深色中文和三维场景导出成功，临时视觉脚本 5 项通过；检查三个导出 PNG 及三个界面截图，数据、颜色、三维光照、保存入口和状态换行正常 |
| 构建与差异 | Release 增量构建、git diff --check 通过 |

计数包含初始化／清理，最终各套件均无失败或跳过。二维全量覆盖 M4 扩展图表，三维全量覆盖已有光照／网格等能力；PNG 专项覆盖完整子树像素、尺寸、透明度、失败路径与三维输出。此次只修改两个私有渲染器，没有重复整库 CTest；同日 M5 完整 CTest 记录见下一节。

真实图形会话下复现（按本机 Qt 安装设置库搜索路径；其他 GPU 应使用对应 ICD）：

```sh
env -u QT_QUICK_BACKEND \
  QT_QPA_PLATFORM=xcb QSG_RHI_BACKEND=vulkan QSG_INFO=1 QT_SCALE_FACTOR=1 \
  VK_DRIVER_FILES=/usr/share/vulkan/icd.d/radeon_icd.json \
  LD_LIBRARY_PATH=/home/catgray/app/Qt/6.12.0/gcc_64/lib \
  build-release/tests/auto/charts/tst_chartrender -maxwarnings 100000
```

分别换为 `tst_chart3drender`、`tst_chartexport`，再设置 `QT_SCALE_FACTOR=2` 运行高 DPI 组合。应核对日志中的 `backend Vulkan` 和物理设备，不能以 software 回退代替。日志、修复前失败记录、矩阵脚本及 PNG 位于忽略目录 `build-release/chart-vulkan/`。

本次结论限定于上述 Linux／XCB／AMD RADV／Qt 6.12 组合；未启用 Khronos validation layer，也未新增其他 GPU、Wayland、Windows／macOS、Qt 6.8、Metal／D3D 或设备丢失验证。三维阴影、三维 LOD 等后续增强仍按 [实施清单](CHART_RENDERING.md) 单独推进；本次不是性能基准验收。

## M5 图像导出及基线复测（2026-10-19）

按本次范围调整，M5 仅包含当前图表 PNG 导出；配置保存／恢复、迁移、同步、数据导出及额外可访问数据表由应用负责。环境为 Ubuntu 26.04 x86_64、Qt 6.12.0、Release，真实图形检查使用 XCB / OpenGL、AMD Radeon 780M、Mesa 26.1.6。

| 检查 | 结果 |
| --- | --- |
| 改动前基线 | 完整 CTest 14/14 组通过；真实 OpenGL 二维 192、三维 22 项通过。此前 Windows 记录中的 LED 管线失败在此环境未重现 |
| 最终构建和静态检查 | Release 构建、QCatUiKit_qmllint、qcat-gallery_qmllint、git diff --check 通过；中英文 TS 无 unfinished。编译仍有既有 QDateTime 废弃接口警告，未增加新警告 |
| 最终完整 CTest | 15/15 组通过，包含新增 chart_export、整个 qml_contracts 和 gallery_contracts |
| PNG 导出专项 | software、真实 OpenGL DPR 1、OpenGL DPR 2 各 7 项通过（含初始化／清理）；检查完整视图与窗口像素一致、标题／坐标／图例／子项覆盖层、透明度、精确尺寸、中文路径、忙碌拒绝、文件替换、写入失败、销毁取消及三维图像；software 明确拒绝三维导出 |
| Gallery 导出 | 12 项通过；十组二维／三维／统计示例的目标图表关联、错误反馈、二维两倍尺寸文件、按钮状态、运行时中英文和 Mobile 布局 |
| 视觉检查 | 实际 OpenGL Gallery 的 1200px 浅色英文、420px 深色中文和三维场景导出成功；检查三个图表 PNG 及桌面／窄屏保存入口、换行与状态反馈，临时视觉脚本 5 项通过 |
| 安装与兼容 | 独立安装的新消费者成功调用公共导出接口并读取 320×240 PNG；使用改动前头文件编译的图表消费者不重编译，加载新库及 QML 模块成功。675 个既有 Chart 类导出函数符号无删除，新增 5 个 |

产物位于忽略目录 `build-release/chart-m5/`，包括基线／最终测试日志、软件与 OpenGL 导出专项、安装消费者、符号对比及 `export-*.png`、`gallery-*.png`。

复现普通测试：`ctest --test-dir build-release --output-on-failure`。真实图形会话下运行 `QT_QPA_PLATFORM=xcb QSG_RHI_BACKEND=opengl build-release/tests/auto/charts/tst_chartexport`，增加 `QT_SCALE_FACTOR=2` 检查高 DPI；按本机 Qt 安装配置库搜索路径。`QCATUIKIT_TEST_CHART_GPU=ON` 另注册 `chart_export_render`，不能使用 software 回退代替真实 RHI 验证。

Vulkan 后续复测见上一节；Windows／macOS、Qt 6.8、Metal／D3D、真机触屏或设备丢失仍未新增验证，既有其他平台历史记录不代表新导出接口在这些平台已验证。导出是当前视图快照，增大像素尺寸不增加当前 LOD／网格或 RHI 纹理的细节；PNG 编码／写入在对象线程执行，不提供连续逐帧导出性能承诺。

## M4 完成验收（2026-10-18）

环境：Windows 11 x64、MSVC 2022、Qt 6.12.0 Release。E01–E10 的既定范围全部完成。本批补齐独立气泡映射、格网／滚动热力图、雷达样式与维度范围、等值线／填色、OHLC／成交量、箱线／离群点；接口、算法、容量与输出约定见 [扩展图表](CHARTEXTENSIONS.md)，前两批见 [统计图表](CHARTSTATISTICS.md)。

| 检查 | 结果 |
| --- | --- |
| 构建与静态检查 | Release 构建、库及 Gallery 的 qmllint、git diff --check 通过；中英文 TS 无 unfinished 条目 |
| 数据与统计 | 扩展专项 7、统计专项 17 通过；覆盖数据拒绝与保留、独立大小／颜色范围、透明颜色、格网缺失／滚动／原值拾取、等值鞍点／平台／来源销毁／轴同步、OHLC 时间与价格、分位数／须端／离群点、雷达原值和极小范围 |
| 完整 GPU 回归 | D3D11 二维 192、三维 22 通过；最后的等值平台端点与雷达极小范围修正另通过下列 M4 专项及最终数据测试 |
| M4 渲染专项 | 最终 D3D11、D3D11 DPR 2、OpenGL、software 各 10 通过，覆盖本批能力及此前堆叠面积／直方图／拟合／瀑布图 |
| Gallery | 新增六模式交互 8、导航与动态示例 66、既有图表 149、独立示例 120、双语／窄屏页面 356、坐标轴区间联动 10 通过 |
| 安装与 ABI | 当前消费者及 24 个历史二进制共 25 个通过；六类的 18 条公共头路径齐全，私有头未安装，115 个 Cat 前缀 QML 类型与模块导出相符 |
| 全仓库 CTest | 13/14 组通过，包括整个 gallery_contracts 和 qml_contracts；仅既有 led_pipeline 布局校验失败，见下文 |

格网像素测试比较密集纹理与稀疏实例，覆盖缺失单元、轴反向和原值拾取。固定色域下单格修改只上传 4 字节且不增加纹理分配；这是 GPU 资源行为，不是 CPU O(1) 或 FPS 保证。颜色映射保留旧纹理标记的着色语义。K 线验证涨跌、影线、OHLC 刻线及 1e15 数值偏移下的非零实体；箱须只绘制在箱体外，中位线和离群点单独检查。等值填色的区间端点有明确归属，平台不会被相邻两带重复覆盖。

实际 Gallery 检查了 1440×1000 桌面浅色、420 像素 Mobile 深色的六种新增配置及英文雷达控件，共 19 张截图。测试实际鼠标拖动、参数编辑、切换与生成的独立 QML；新模式归入相应页面，图表目录为 17 个入口、30 个演示配置。气泡示例设置轴留白；箱线须线与窄屏控件显示已复核。截图脚本无警告。单独页面回归首次遇到从 LED 页面卸载时的 DelegateModel 警告，随后完整 Gallery 回归及单独页面复跑均通过；本轮没有修改 LED 页面生命周期。

全仓库唯一失败为 `LedPipelineTests::layoutMappingAndValidation` 的 `m.applyNow(c)`（tests/auto/led/tst_ledpipeline.cpp:189），单独运行复现为 14 通过、1 失败；同一失败已在此前三维光照验收中用旧构建确认。本轮没有修改该管线，不能将全仓库结果描述为全部通过。早前记录的选择控件文字宽度失败在本次环境中未重现。

本机复查产物位于忽略目录 `build-led-release/chart-m4-complete/`：`ctest-final.txt`、`led-known-failure.txt`、`extended-final.txt`、`statistics-final.txt`、`render-final.txt`、`render-3d-final.txt`、`m4-*-final.txt`、Gallery 的 `*-final.txt`、`capture-final.txt`、PNG、`consumers.json` 和构建／lint 日志。当前消费者另归档为 `build-led-release/charts-consumer-m4-complete.exe`。后续运行 CTest 可使用 `ctest --test-dir build-led-release --output-on-failure`，GPU 测试需真实窗口及相应后端环境。

本轮未新增其他操作系统、Qt 6.8、D3D12／Vulkan／Metal、真机触屏或设备丢失验证。M5 的配置保存／恢复、同步、导出尚未开始；M4 完成不代表与 Qt Graphs 所有 API 或像素结果完全一致。

## M4 第二批：公共拟合与瀑布图（2026-10-18）

环境：Windows 11 x64、MSVC 2022、Qt 6.12.0 Release。完成 E06 的 CatChartFit 普通最小二乘／残差带与 E10 的 CatWaterfall 纵向累计柱、增减／小计／总计、重置和连接线。公开契约及限制见 [统计与扩展图表](CHARTSTATISTICS.md)。

- 构建、库及 Gallery 的 qmllint 通过，中英文 TS 无 unfinished 条目，差异空白检查通过。新增两个公共 QML 类型及三种头文件路径；私有统计发布及 RHI 类型不安装。
- software/offscreen：统计专项 15、原图表核心 165、Gallery 数据 27、导航及动态示例 60、既有图表交互 149、可复制示例 117、双语／窄屏页面 347 项通过。计数含初始化与清理。部分 offscreen C++ 测试打印本机 Qt 包缺少 lib/fonts 目录的提示，不影响断言；实际窗口检查使用系统字体。
- 统计检查覆盖已知 OLS 系数、R²／RMSE、残差带、1e15 坐标偏移与 ±1e308 有限输入、常量 Y／相同 X、空源／销毁、异步更新和样式不重算；瀑布图覆盖跨零、初始值、小计重置、原始增减值、来源元数据及溢出／非法步骤的整批拒绝。验证中修复区间发布丢失原始 value 的问题，标签和拾取保留增减量。
- D3D11 全套渲染 188 项通过；最终 value 与 software 柱色修复后拟合、瀑布、堆叠与直方图像素专项 6 项通过，software 同组 6 项、D3D11 DPR 2 新增两类专项 4 项通过。software 原先按批次统一着色，现支持不同实例颜色并正确还原预乘透明度；新增半透明柱色检查。专项还覆盖连接线、小计断线、残差带与拟合线、边界外背景及原始记录拾取。
- 实际 D3D11 Gallery 在 1440 px 桌面及 420 px 暗色 Mobile 操作、拖动并检查 10 张截图，覆盖正常拟合／无效输入、跨零瀑布、小计重置及中英文控件。窄屏输入换行，桌面瀑布显示六个分类；QML 示例跟随参数并可独立创建。
- 独立前缀安装后的当前消费者与 23 个历史二进制共 24 个通过。新消费者验证两个公共 C++ 类、统计输出和 QML 创建；历史可执行文件不重编译，使用同一候选 DLL 与安装 QML 模块。
- 本地证据位于 `build-led-release/chart-m4b/`，包含 statistics-final、charts、data-final、chartnavigation-final、charts-final、examples-final、pages-final、render、render-final、software-final、dpr2、capture、consumers.json、构建日志和 PNG。本轮未运行整库 CTest，也未新增其他平台或 GPU 后端验证。M4 尚余 E01／E02／E03／E07／E08／E09；M5 未开始。

## M4 首批：堆叠面积与直方图（2026-10-18）

环境：Windows 11 x64、MSVC 2022、Qt 6.12.0 Release。新增 CatChartStack 与 CatHistogram 数据适配器，派生系列复用二维 RHI／software 路径。API 和端点、正负分组、缺失值规则见 [统计图表](CHARTSTATISTICS.md)。

- 库、Gallery 及测试构建、两个 qmllint 目标通过；中英文文案已补齐。新增真实头及两种转发头可从独立安装目录使用。
- software/offscreen：统计专项 11 项、原图表核心 165 项、Gallery 导航及动态示例 56 项、独立示例 115 项、双语及窄屏页面 341 项通过。专项覆盖原始数据保留、共享断点、严格 X 对齐、图例隐藏后重算、同时过零的单侧百分比极限、正负组分别缩放避免溢出／下溢、分箱端点、频率总和和密度积分、无效配置原子拒绝、空／常量输入和对象销毁。
- D3D11 图表渲染全套 186 项通过；百分比过零修正后重跑两类新增渲染检查，DPR 1 与 2 各 4 项通过（含初始化与清理）。像素检查覆盖正负区域、百分比和不等宽箱宽／密度高度，拾取返回原值映射及分箱信息。
- D3D11 实际 Gallery 窗口在 1440 px 桌面和 420 px 暗色 Mobile 下操作和拖动两种图表，检查 8 张控件与图表截图；窄屏数值输入换行。底部代码跟随主要参数更新，可独立创建。原 Gallery 图表交互 149 项通过，命令行 `--chart-demo 20`／`21` 中英文启动通过。
- 独立前缀安装消费者与 22 个历史二进制共 23 个通过。当前消费者覆盖新 C++ 头、统计输出及两个 QML 类型；历史二进制不重新编译，加载候选 DLL／QML 安装包。
- 本地日志、截图与安装检查位于 `build-led-release/chart-m4/`。本轮未运行整库 CTest，未验证其他操作系统、Qt 6.8 或其他 GPU 后端；M4 其余增强项及 M5 未包含在本批验收中。

## 折线标记与范围滑块平移稳定性（2026-10-18）

环境：Windows 11 x64、MSVC 2022、Qt 6.12.0 Release。修复有序直线使用视口相对峰谷分桶导致内部标记更换，以及范围读数改变两组滑块列宽的问题。直线／面积概览复用原始数据层次索引，Gallery 区间等宽排列并固定读数行数。

- 修复前回归复现：平移 5 px 后，直线内部 16,334 个着色像素中有 14,629 个偏离整体平移结果；Gallery X 区间宽度从 453 px 变为 416 px。
- 构建、库与 Gallery qmllint、`git diff --check` 通过；software/offscreen 图表核心 165 项、Gallery 区间 10 项、滑块 17 项、导航 53 项通过。核心新增无断点直线的缓存复用、峰值原始索引及深度缩放验证。
- D3D11 图表渲染 184 项通过，包含直线／Catmull-Rom 与正反向坐标轴的平移像素对比。Gallery 在 1440 px 桌面和 420 px 暗色 Mobile 下运行连续鼠标按下／移动／释放，验证数据映射跟随拖动位移、滑块尺寸不变，并检查前后截图。
- 日志与截图：`build-led-release/chart-pan-stability/`。本轮未运行完整 CTest，未测其他 GPU 后端或平台；缓存首次构建与数据更新仍需处理全量记录。

## 滑块在现有 Gallery 中的应用（2026-10-18）

环境：Windows 11 x64、MSVC 2022、Qt 6.12.0 Release。图表参数、进度预览、LED Deck／Mixer 与颜色选择器接入滑块扩展；坐标轴区间增加锁定跨度，相关独立示例同步更新。

- 库、Gallery、测试目标构建及两个 qmllint 目标通过；中英文翻译无未完成条目，`git diff --check` 通过。
- software/offscreen：库滑块 16 项、基础组件 113 项、LED UI 22 项、Gallery 滑块 17 项、坐标轴范围 8 项、图表 149 项、导航与动态示例 53 项、双语及窄屏页面 335 项、独立示例 113 项、Gallery 通用契约 25 项通过。计数包含初始化与清理。
- 新增验证覆盖真实数值键入、百分比回写、默认值恢复后继续接收模型更新、窄屏输入换行、锁定跨度下键盘平移和外部缩放；LED 验证预设异步更新、速度／亮度恢复及混合器居中恢复。
- D3D11 实际窗口运行 Gallery 滑块 17 项通过；检查 1440 px 桌面柱图、三维光照、坐标轴范围、LED 混合器、进度，以及 420 px 暗色 Mobile 的柱图、范围和 LED 面板，共 8 张截图。
- 日志和截图位于 `build-led-release/slider-adoption/`。本轮未运行完整 CTest，未验证其他平台或图形后端。

## 滑块第一批扩展（2026-10-18）

环境：Windows 11 x64、MSVC 2022、Qt 6.12.0 Release。增强 CatSlider，新增公开 CatRangeSlider、CatSliderField；图表 X／Y 控件与可复制代码迁移到公共区间滑块。

- 库、Gallery、测试目标构建通过，两个 qmllint 目标通过；中英文翻译无未完成条目，`git diff --check` 通过。
- software/offscreen：滑块专项 16 项、基础组件契约 113 项、原滑块方向测试 4 项、Gallery 滑块变体与图表整段拖动 11 项、图表范围双向同步 8 项、图表导航与动态示例 53 项、全部 Gallery 双语及窄屏页面 335 项、独立代码示例 113 项、原图表回归 149 项、Gallery 通用契约 25 项通过。计数包含初始化与清理。
- 专项覆盖实际鼠标／合成触摸／键盘输入，水平、垂直、镜像与反向范围，最小／最大／固定跨度、整段拖动到边界、释放吸附、延迟提交、默认值恢复、无效数值输入、编辑后外部绑定及原 padding 行为。图表额外验证单轴整段平移、深度外部缩放及原始数据不变。
- D3D11 实际窗口运行滑块专项 16 项通过；检查 1440 px 桌面的居中填充、垂直、区间提示、精确输入，以及 420 px 暗色 Mobile 的三个页面。刻度按实际文字宽度避让，数值框窄屏换行。
- 安装到独立前缀，将不链接本库的 QtTest 启动器复制到独立目录，只配置安装目录的 DLL 与 QML 导入路径，滑块专项 16 项通过；两个新类型及内部视觉助手随安装包可解析。
- 本地日志与截图：`build-led-release/slider-expansion/`。本批未运行整库 CTest，也未验证 Qt 6.8、其他操作系统或其他 GPU 后端。

## Gallery 折线图双滑块（2026-10-17）

环境：Windows 11 x64、MSVC 2022、Qt 6.12.0 Release。折线与断点曲线增加 X／Y 两组范围双滑块，复用公开坐标轴／映射接口；没有修改库 API。

- Gallery 构建与 qmllint 通过。software/offscreen：范围控件 8 项、原图表回归 149 项、图表导航和动态示例 53 项、全 Gallery 独立示例创建 111 项通过，计数含初始化与清理。
- 范围检查覆盖真实鼠标拖动和键盘调整、双向平移／缩放同步、单轴操作保留另一轴自动状态、反向轴、超域扩展、端点碰合、全图恢复、清空／追加和语言切换。两种折线的可复制 RangeSlider 绑定代码实际创建并验证双向同步。
- D3D11 检查 1440 px 桌面、420 px 暗色 Mobile 和空数据禁用态。滑块不复制原始样本，通过合并通知读取图表映射；本轮未测独立 FPS 或其他图形后端。
- 本地日志与截图：`build-led-release/chart-ranges/`。

## Gallery 图表目录与动态示例（2026-10-17）

环境：Windows 11 x64、MSVC 2022、Qt 6.12.0 Release。本轮仅调整 Gallery 组织、示例生成和导航，不修改图表库 API 或渲染实现。

- 原三个重复预览入口整理为十个图表用途页面，按二维、三维、坐标轴与组合分组；保留原 20 个演示编号和旧页面别名。每页仅显示所属变体，底部代码与复制内容跟随当前图表及主要样式。
- 构建与库／Gallery 两个 qmllint 目标通过。software/offscreen：原图表回归 149 项、新导航与动态示例 53 项、全 Gallery 双语及窄屏页面 329 项、独立代码示例 111 项、通用导航／语言等契约 25 项通过。计数含各套件初始化和清理。
- 新检查实际创建全部 20 种演示、六种柱图布局、四种饼环变体以及三维类型／形状对应的 QML；覆盖分组入口、别名、搜索、复制反馈、保留所选类型的重置、切换页面停止压测和语言切换保留样式。
- D3D11 实际窗口检查桌面分组目录、区间面积预览及代码、三维曲面、420 px 暗色 Mobile 中英文目录及代码。命令行独立 `--chart-demo 19`、旧地址组合 `--page chart --chart-demo 15` 与 `--page chart-axis` 启动通过。
- 日志与截图位于本地 `build-led-release/chart-gallery/`。本轮没有重跑整库 CTest、安装消费者或其他 GPU 后端矩阵。

## CatChart M2 收尾：已验证子项（进行中，2026-10-17）

环境仍为 Windows 11 x64、MSVC 2022、Qt 6.12.0 Release。以下按子项记录，**不表示 M2 全部完成**；剩余工作见 [收尾清单](CHART_RENDERING.md)。

### C05 多轴与轴侧

- 新增轴 orientation／side／visible、系列 xAxis／yAxis 关联和带系列参数的映射 API。轴共享与销毁回退、同侧布局、按轴组合的柱分组／堆叠／百分比隔离以及全轴导航已实现。
- C++ 147 项、QML 69 项、Gallery 137 项、D3D11 133 项通过。覆盖四组独立轴反向组合的真实双指缩放、隐藏轴后仍保留映射、无效方向恢复、销毁／解绑及旧行为回归。
- 两个 qmllint 目标通过；实际 Gallery 的桌面四边轴、同侧轴、420 px 暗色 Mobile 图表和控件四张截图已检查。
- 新安装消费者和十三份历史消费者加载新库通过；公开对象布局、Sample／RangeSample 与既有函数签名不变。
- 本地记录在 `build-led-release/chart-m2-completion/multiaxis-*`。DPR 2／OpenGL 全量矩阵将在 M2 收尾交付时统一运行；此前的单轴验证不代替本次多轴覆盖。

## CatChart M2 坐标轴：次刻度与反向显示（2026-10-17）

环境：Windows 11 x64、MSVC 2022、Qt 6.12.0 Release。按次刻度、反向轴两个步骤实施；次刻度契约、software 与 D3D11 定向检查通过后继续接入反向轴，最终统一验证交付。

- 新增 `CatChartAxis.minorTickCount`（0–10，默认 0）和 `reversed`（默认 false）。分类轴不产生分数次刻度；对数轴按 log10 空间等分。显示反向不改变数据顺序、minimum < maximum 契约及默认外观。
- C++ 契约 145 项、QML 图表 68 项、Gallery 图表 136 项、Gallery 数据 26 项通过。覆盖四种 scale × 四种轴方向的往返映射、拾取、锚点缩放、平移、框选和触摸；20 万记录的六种阶梯／样条反向平移保留原始命中，缓存不重建、准备路径点数有界。
- D3D11 DPR 1、DPR 2、OpenGL DPR 1 全量各 129 项通过。新增七种曲线 × Line／Area／RangeArea、六种柱形布局、散点／气泡／热力图的双轴反向几何检查，四种柱形标签位置、物理底部面积基线、次刻度网格开关及四种方向的双指缩放。对应 software／offscreen 定向检查 38 项通过。
- 库与 Gallery 两个 qmllint 目标通过，构建无新增警告；三个 Catalog 示例和完整文档示例创建／绘制共 6 项通过。实际 Gallery 的 1440 px 亮色桌面、对数轴和 420 px 暗色 Mobile 控件／图表共四张截图已检查；真实鼠标拖动保持数据锚点和 LOD 缓存。
- 新安装消费者及十二份历史二进制加载新安装库通过；新增属性追加到原有属性后，公开对象与数据结构布局不变，qmltypes 和安装头包含两个配置。最终属性顺序整理后重新构建并通过 C++／QML／Gallery、示例、实际窗口捕获及全部安装消费者检查。

本地日志与截图在 `build-led-release/chart-axes/`。本轮完成 C05 的反向轴与次刻度，轴侧选择、多轴及系列关联仍待实现；其余阶段见 [实施清单](CHART_RENDERING.md)。未验证 Qt 6.8、D3D12、Vulkan、Metal、其他操作系统或真机触屏。

## CatChart 密集曲线切换后拖动卡顿修复（2026-10-17）

环境：Windows 11 x64、MSVC 2022、Qt 6.12.0 Release。复现 Gallery 折线图的两条信号，每系列 100,000 条原始记录，默认 displayBudget = 2048。旧路径在阶梯／样条模式绕过预算，每次平移重新处理 200,000 点、扫描自动极值，自然样条还重新求解控制点。

修复使用保留连续段与真实峰谷的私有曲段层次索引；视口变换复用缓存，放大后展开原始曲段。固定轴范围跳过全量极值扫描；原始记录、公开对象布局与函数签名不变。

相同数据和 1440 × 360 无窗口图表，完成初次准备后往返平移 20 次，测量 `panBy + plotRect` 的 CPU 时间中位数（包含几何准备，不包含 RHI 上传／绘制）：

| 曲线模式 | 修复前 | 修复后 |
| --- | --- | --- |
| 直线 | 6.05 ms | 2.74 ms |
| 左阶梯 | 77.24 ms | 0.54 ms |
| 右阶梯 | 77.27 ms | 0.51 ms |
| 中心阶梯 | 103.53 ms | 0.54 ms |
| 单调样条 | 68.98 ms | 0.54 ms |
| Catmull-Rom | 92.69 ms | 0.51 ms |
| 自然样条 | 96.24 ms | 0.56 ms |

相同 200,000 条记录、1440 × 480 D3D11 窗口、默认反走样，等待每次提交完成再发起下一次平移，共 30 次；从发起平移到对应版本 frameSwapped 的中位耗时，Catmull-Rom 为 360.70 → 16.70 ms，自然样条为 276.04 → 16.76 ms。当前视图两系列合计准备约 5,368 个代表路径点，原始记录仍为 200,000 条。这是本机的 GUI／渲染／帧交换链路测量，不是 GPU 时间戳，也不是其他设备的帧率保证；首次切换／更新数据仍需建立索引。

- C++ 契约 128 项、QML 图表 55 项、Gallery 图表 131 项通过。新增检查六种非线性模式的 200,000 点平移工作量、缩放裁剪、颜色／线宽／虚线不重建缓存、数据／张力／断点／轴 scale／类型变更失效，以及覆盖、缩容、移除／重新添加和销毁。
- D3D11 DPR 1、DPR 2、OpenGL DPR 1 各 86 项通过。新增六种阶梯／样条 × Line／Area 的孤立峰值、断点空隙、拖动缓存复用与局部放大测试；放大细节与 displayBudget = 0 的绘图区像素一致。另检查端点在视野外、控制多边形进入视野的自然／Catmull-Rom 曲段。对应 software／offscreen 检查 16 项通过。
- 库和 Gallery 两个 qmllint 目标通过。实际 Gallery 鼠标拖动保留两系列各 100,000 条数据，缓存构建次数不增加；检查 1440 px 亮色桌面概览、放大自然样条及 420 px 暗色 Mobile 三张截图。
- 新安装消费者及 M2 第九批至迁移前共十一份历史消费者直接加载新安装库通过；私有索引头未安装。

本地基准源码、前后日志、测试结果与截图位于 `build-led-release/chart-curve-pan/`。无序曲线、断点线性曲线、RangeArea 专项 LOD 和持续数据更新的增量索引仍未实现；本次没有更改 Gallery 百万预设的独立概览策略。未验证 Qt 6.8、D3D12、Vulkan、Metal 或其他操作系统。

## CatChart M2 第九批：饼／环外置引线标签（2026-10-17）

环境：Windows 11 x64、MSVC 2022、Qt 6.12.0 Release。新增 sliceLabelPosition，默认保留内部百分比；外置模式显示名称、百分比和彩色引线，支持分离扇区、局部饼图和多环。

- C++ 契约 115 项、QML 图表 55 项、Gallery 图表 131 项、Gallery 数据 26 项通过。新增验证两侧标签排列、拥挤时按占比取舍及稳定顺序、属性信号与非法值、预留空间和半径恢复、分离后的原始记录拾取，以及极小绘图区保留数据。
- D3D11 DPR 1、DPR 2、OpenGL DPR 1 各 72 项通过。新增 Pie、Doughnut、多环的文字与引线像素断言，验证单扇区标签开关、清空、内外切换及引线区域不参与拾取；对应 software／offscreen 检查 5 项通过。
- 库与 Gallery 的 qmllint 均通过。三个 Gallery 代码片段及完整同心环文档示例创建／绘制通过，共 6 项。语言切换和恢复示例的标签位置通过稳定枚举值绑定保持一致。
- 实际 D3D11 截图检查：1440 px 亮色桌面双环与 12 扇区饼图，420 px 暗色 Mobile 控件、外置双环与内部标签。窄屏外置标签保留百分比、缩短名称；数据圆盘因预留标签列而缩小，切换内部模式可恢复原始半径。
- 新安装 C++／QML 消费者以及 M2 第八批至首批、M1、迁移前共十份历史二进制，均通过直接加载新安装库检查。QML 从安装目录 lib/qml 导入；新增私有标签助手不安装。

日志、代码片段检查与五张截图在本地 `build-led-release/chart-m2i/`。本批补齐 D10／D11 的外置引线标签；最多尝试前 256 个可见且启用标签的扇区，拥挤标签有界省略，尚未提供任意标签委托或引线交叉消除。M2 其他能力、专项 LOD 及三维范围仍见 [进度清单](CHART_RENDERING.md)。未验证 Qt 6.8、D3D12、Vulkan、Metal 或其他操作系统。

## CatChart M2 第八批：阶梯与样条区间面积（2026-10-17）

环境：Windows 11 x64、MSVC 2022、Qt 6.12.0 Release。RangeArea 开始使用既有 curveMode／splineTension，支持直线、三种阶梯和三种样条；默认行为与成对上下界数据入口保持不变。

- C++ 契约 113 项、QML 图表 55 项、Gallery 图表 131 项、Gallery 数据 26 项通过。新增检查上下界原值拾取、原始曲段归属、阶梯平台归属、DateTime 大坐标、双对数轴插值、双边界极值、无效／缺失下界隔离、清空与模式切换。
- 上下边界使用共同参数细分；自动范围包含两条边界的样条实际极值。测试覆盖原始端点都在视野外、曲线仍过冲进入绘图区的情况，避免可见节点筛选误删曲段。
- D3D11 DPR 1、DPR 2 和 OpenGL DPR 1 各 69 项通过。新增七种模式的像素断言，覆盖一条直边与一条曲边、双边框、细分条带透明度、虚线、断点、曲线交叉留白、渐变裁剪、清空及视野外端点。software／offscreen 对应 11 项通过。
- 两个 qmllint 目标通过；三个 Gallery 代码片段与完整区间面积文档示例创建／绘制通过，共 6 项。Gallery 曲线模式采用稳定值绑定，验证语言切换、张力、增删／清空与恢复设置。
- 检查实际 D3D11 截图：1440 px 桌面的自然样条与 Catmull-Rom 区间带，420 px 深色 Mobile 的曲线／张力控件、交叉曲线与中心阶梯。主题切换截图在过渡完成后采集。
- 新安装消费者验证曲线区间带原始下界拾取成功；M2 第七批至首批、M1、迁移前共九份历史消费者直接加载新安装库通过。

日志与截图在本地 `build-led-release/chart-m2h/`，GPU 全量日志在 `build-led-release/chart-qa/gpu-*.txt`。本批完成 D07 的曲线区间带，独立上下界数组插值对齐、专项 LOD 及其他 M2／三维余项仍见 [进度清单](CHART_RENDERING.md)。未验证 Qt 6.8、D3D12、Vulkan、Metal 或其他操作系统。

## CatChart M2 第七批：触摸平移与双指缩放（2026-10-17）

环境：Windows 11 x64、MSVC 2022、Qt 6.12.0 Release。新增默认开启的 touchNavigationEnabled，单指平移、双指中心锚点缩放与平移；鼠标框选模式独立生效。触摸测试使用 QtTest 注入事件，未验证物理触屏设备。

- C++ 契约 97 项、QML 图表 55 项、Gallery 图表 130 项、Gallery 数据 26 项通过。覆盖四种坐标轴、手指数变化、重合触点、取消后重新轻点、固定范围中的实时数据更新，以及 Flickable 捕获和关闭导航后的父容器滚动。
- 取消覆盖 Esc、失焦、触摸取消／捕获丢失、跨窗口、隐藏／禁用、交互／导航开关、尺寸、外部轴变化、类型、重置、自动范围变化和第三个触点；同步 viewChanged 回调关闭交互后不会重新保持触摸捕获。
- 首轮发现 DateTime 刻度变宽导致触点锚点偏移；修正为手势期间保持绘图区左边距，结束后恢复刻度布局。四种轴的锚点和缩放比例断言全部通过。
- D3D11 DPR 1、DPR 2 和 OpenGL DPR 1 各 60 项通过。新增触摸测试检查缩放后的散点像素、锚点、原始数据不变，以及按下未移动时几何版本和顶点上传次数不变。software／offscreen 对应 3 项通过。
- 两个 qmllint 目标通过；三个 Gallery 片段与完整交互示例创建／绘制通过，共 6 项。D3D11 Gallery 截图检查了 1440 px 桌面、420 px 深色 Mobile 的双指缩放前后，以及关闭导航的提示和窄屏布局。
- 新安装 C++／QML 消费者和 M2 第六批、第五批、第四批、第三批、第二批、首批、M1、迁移前八份历史二进制均成功加载新库。

日志与截图在本地 `build-led-release/chart-m2g/`，GPU 全量日志在 `build-led-release/chart-qa/gpu-*.txt`。C10 的鼠标／键盘／触摸功能已交付；真机触屏与其他系统验证、M2 其余能力及三维仍见 [进度清单](CHART_RENDERING.md)。未验证 Qt 6.8、D3D12、Vulkan、Metal 或其他操作系统。

## CatChart M2 第六批：二维框选缩放（2026-10-17）

环境：Windows 11 x64、MSVC 2022、Qt 6.12.0 Release。新增 PanDrag／RectangleZoomDrag、双轴／仅 X／仅 Y 框选、zoomToRect、只读选框状态及取消接口；原始记录保持不变。

- C++ 契约 93 项、QML 图表 32 项、Gallery 图表 130 项、Gallery 数据 26 项通过。覆盖四种坐标轴类型、反向／越界／过窄矩形、未选轴自动状态、取消后的点击、Flickable 拖动协调和语言／恢复行为。
- 取消检查包含 Esc、失焦、鼠标捕获取消、禁用／隐藏、交互开关、尺寸／模式／轴／类型变化、重置、API 取消、自动范围变化，以及跨窗口迁移；手动范围内数据追加保留选择。同步状态通知中取消也不会重新保持捕获。
- D3D11 DPR 1、DPR 2 和 OpenGL DPR 1 各 59 项通过；新增三种框选轴的像素、覆盖顺序、取消后恢复、几何版本及零额外顶点上传断言。software／offscreen 对应 5 项通过。
- 场景初始化改为请求重绘，不重复准备数据几何。跨窗口测试以迁移前帧计数为基准，允许窗口曝光时已完成绘制；像素与渲染版本仍须正确。
- 两个 qmllint 目标通过；三个 Gallery 片段与完整框选 QML 示例创建／绘制通过，共 6 项。实际 D3D11 截图检查了 1440 px 桌面双轴框选、420 px 深色 Mobile 单轴框选，以及各自松开后的缩放结果。
- 新安装 C++／QML 消费者和 M2 第五批、第四批、第三批、第二批、首批、M1、迁移前七份历史二进制均成功加载新库。

日志与截图位于本地 `build-led-release/chart-m2f/`，GPU 全量日志在 `build-led-release/chart-qa/gpu-*.txt`。本批完成 C10 的鼠标框选部分，触摸平移／双指缩放、三维及其他余项仍按 [进度清单](CHART_RENDERING.md) 推进。未验证 Qt 6.8、D3D12、Vulkan、Metal 或其他操作系统。

## CatChart M2 第五批：面积基线与填充（2026-10-17）

环境：Windows 11 x64、MSVC 2022、Qt 6.12.0 Release。新增三种 Area 基线模式，以及 Area／RangeArea 的独立填充色、边界色、水平／垂直双色渐变。默认零基线与纯色透明度保持不变。

- C++ 契约 65 项、QML 图表 25 项、Gallery 图表 127 项、Gallery 数据 26 项通过。覆盖自动／手动范围、对数轴回退、空系列、非法设置、语言切换、增删和恢复外观。
- D3D11 DPR 1、DPR 2 与 OpenGL DPR 1 各 56 项通过。新增像素断言检查渐变方向、裁剪后的颜色、不同端点 alpha、透明末端、跨基线填充、断点、独立边界／标记颜色与系列叠加。
- software／offscreen 的新增面积像素测试 10 项通过。测试通过图表实际 backend 判断软件就绪；QQuickWindow 的静态 graphicsApi 不能代替当前图表的后端判断。
- 库与 Gallery qmllint 通过。三个 Gallery 代码片段及完整面积文档示例创建、绘制通过，共 6 项。
- 实际 D3D11 Gallery 截图检查通过：1440 px 桌面面积／区间面积、420 px 深色 Mobile 区间面积；检查交叉、断点、渐变和窄屏控件布局。
- 安装后的新 C++／QML 消费者通过；M2 第四批、第三批、第二批、首批、M1 和迁移前共六份历史二进制直接加载新安装库通过。

日志与截图位于本地 `build-led-release/chart-m2e/`，GPU 全量日志位于 `build-led-release/chart-qa/gpu-*.txt`；构建产物不提交。双色渐变不代表任意停止点／纹理填充已实现，曲线区间带及 M2 其他余项仍见 [进度清单](CHART_RENDERING.md)。未验证 Qt 6.8、D3D12、Vulkan、Metal 或其他操作系统。

## CatChart M2 第四批：柱状宽度、边框与标签（2026-10-17）

环境：Windows 11 x64、MSVC 2022、Qt 6.12.0 Release。

- C++ 图表契约 63 项、QML 契约 24 项、Gallery 图表交互 125 项、数据控制器 26 项通过（含初始化／清理）。新增覆盖六种布局的宽度／间距和命中、正负值与零值、隐藏系列后的重新分组、零宽与全间距，以及样式参数边界。
- D3D11 DPR 1／2、OpenGL DPR 1 各 48 项通过。像素检查覆盖六种布局的柱内边框、间距与宽度更新、清空、正负横纵柱的四种标签位置、占比和原值模板输出、关闭标签后的恢复，以及裁剪边缘没有伪造边框。
- 库、Gallery 和两个 QML lint 目标通过。修复了语言切换后新增下拉框显示重置、实际样式仍保留的不一致；控件按稳定值恢复选择。检查中文 1440px 桌面正负百分比图、横向图和 420px Mobile 暗色页面；横向标签启用时 Gallery 增加图表高度，过密文字仍按公共规则省略。
- 三个 Gallery 代码片段与图表文档的完整柱图示例均成功创建和绘制，共 6 项检查。新安装消费者调用 C++／QML 柱状 API 通过；M2 第三批、第二批、首批、M1 和迁移前共五份历史二进制直接加载新库成功。

日志及截图保存在本地 `build-led-release/chart-m2d/`。本轮没有实现柱实例化、标签旋转或任意标签委托；绘制几何仍经过私有 RHI 数据层。未验证 Qt 6.8、D3D12、Vulkan、Metal 或其他操作系统。

## CatChart M2 第三批：局部饼图、分离扇区与同心环（2026-10-17）

环境：Windows 11 x64、MSVC 2022、Qt 6.12.0 Release。

- C++ 图表契约 56 项、QML 契约 23 项、Gallery 图表交互 124 项、Gallery 数据控制器 26 项通过，含初始化／清理。覆盖角度与半径边界、极大权重、分离后的空隙和拾取、逐环可见性、系列移除／销毁，以及样式验证、批次原子性和容量覆盖。最终双／三环切换另行复查通过。
- D3D11 DPR 1／2 与 OpenGL DPR 1 各 33 项通过。新增像素检查验证局部角度、单扇区颜色和边框、中心孔、环间空隙、分离留白和清空后几何消失。
- OpenGL 首轮在既有跨窗口透明度用例出现一次颜色断言失败，单独复查通过。该像素用例随后关闭非测试目标的鼠标悬停覆盖，并用作用域清理保证断言提前返回时不会删除栈上 ChartItem；最终三个配置的完整测试通过。此记录不代表完成设备丢失注入验证。
- 库、Gallery、测试及两个 QML lint 目标通过；实际查看 1440px 中文桌面的局部饼图、双／三环，以及 420px Mobile 暗色图表和图例。修正了主题字号下百分比文本被错误省略的问题。
- 文档中的完整同心环 QML 示例创建和绘制成功；独立安装消费者通过新增 C++ 与 QML API。M2 第二批、M2 首批、M1 和 RHI 迁移前四个历史消费者二进制均直接加载本轮安装库运行成功。

日志与截图保存在本地 `build-led-release/chart-m2c/`；不提交构建产物。内部标签放不下时省略，外置引线标签／任意委托未实现。完整 M2、三维及扩展范围仍以 [进度清单](CHART_RENDERING.md) 为准；未验证 Qt 6.8、D3D12、Vulkan、Metal 或其他操作系统。

## CatChart M2 第二批：虚线、断点与区间面积（2026-10-17）

环境同下节：Windows 11 x64、MSVC 2022、Qt 6.12.0 Release。

- C++ 图表契约 52 项、QML 图表契约 22 项、Gallery 图表交互 123 项、Gallery 数据控制器 26 项通过，数量含初始化／清理。新增覆盖断点与上下界的批次拒绝、容量覆盖、缩容、线程限制、全部七种曲线模式的断点、LOD 保留断点、区间边界交叉／原值拾取及样式参数校验。
- D3D11 DPR 1／2、OpenGL DPR 1 各 32 项通过。新增像素检查验证虚线间隙、偏移和跨节点相位、七种曲线的面积断开、区间交叉填充与断点留白；既有缓冲、透明度、裁剪和窗口生命周期回归继续通过。
- 库、Gallery 及测试构建成功，两个 QML lint 目标通过。查看中文 1440px 桌面、420px Mobile 暗色区间交叉图，以及窄屏自然样条断点图；控件换行、页面滚动和图形显示正常。
- 三个图表页面的可复制 QML 示例均成功创建（含初始化／清理共 5 项）。
- 独立安装后的新 C++／QML 消费者成功调用范围批次、断点和虚线 API。M2 首批、M1 及 RHI 迁移前保存的消费者二进制均直接加载新安装库成功。

此次不代表完整 M2：端点／连接样式、曲线区间带、饼／环增强、实例化等仍见 [进度清单](CHART_RENDERING.md)。带断点的曲线暂时禁用峰谷采样，区间面积为直线边界；不据此承诺百万图元性能。未验证 Qt 6.8、D3D12、Vulkan、Metal 或其他操作系统。

## CatChart M2 首批二维变体：2026-10-17

环境：Windows 11 x64、MSVC 2022、Qt 6.12.0 Release；在下述 M0／M1 基础上验证阶梯／样条、四种标记及六种柱状布局。M2 完整范围和三维／扩展能力仍按 [功能清单](CHART_RENDERING.md) 推进。

| 检查 | 结果与覆盖 |
| --- | --- |
| 库、Gallery、测试构建及两个 QML lint 目标 | 通过，无新增编译或 lint 警告 |
| C++ 图表契约（software） | 41 项通过；阶梯归属、重复／无序 X、三种样条及极值／大时间戳／对数轴、形状拾取、正负百分比、极大分母、零值与隐藏系列 |
| QML 图表契约（software） | 22 项通过 |
| Gallery 图表交互（software） | 120 项通过；新变体切换、样式绑定、动态数据、语言切换、清空与恢复，以及原有演示回归 |
| Gallery 数据控制器 | 26 项通过；既有 15 个演示的百万记录保留和概览规则不变 |
| 实际 RHI 渲染 | D3D11 DPR 1／2、OpenGL DPR 1 各 29 项通过；包含新增类型清空、六种插值曲线及面积填充、四种标记像素检查；原有混合、裁剪、上传复用与跨窗口测试继续通过 |
| 独立安装与 ABI | 新安装头／模块的 C++、QML 变体消费者成功；M1 和 RHI 迁移前保存的二进制直接加载新安装库成功 |
| Gallery 视觉检查 | D3D11 中文浅色 1440px 自然样条＋菱形节点，暗色 420px Mobile 横向正负百分比柱图；窄屏滚动后控件、分类轴、占比轴和图例正常 |

以上测试数量包含初始化／清理。此次未测量新变体的 GPU 时间戳或百万图元 FPS；阶梯／样条没有套用普通折线的峰谷 LOD，实例化标记与矩形仍待实现。未验证的 Qt 版本与后端同下节。

## CatChart 私有 RHI 数据层：2026-10-17

环境：Windows 11 x64、MSVC 2022、Qt 6.12.0 Release。验证范围为现有十种二维图表迁移，
不包含规划中的三维图表、新二维变体或 GPU 性能基准。

- 库、Gallery 和相关测试构建成功；`QCatUiKit_qmllint`、`qcat-gallery_qmllint` 通过。
- software 回退：C++ 图表契约 32 项、QML 图表契约 22 项、Gallery 图表交互 113 项、Gallery 图表数据 26 项通过，均含初始化／清理。
- 新增 `tst_chartrender`：D3D11 的 DPR 1／2、OpenGL 的 DPR 1 各 15 项通过。覆盖十种图表显示及清空、十字线在不透明数据之上、悬停时零顶点上传、局部更新、尺寸变化复用缓冲、预乘透明度、绘图区裁剪、跨窗口重建、旋转／透明父项和 `layer.enabled` 下的多图表合成。
- 安装到独立目录，新 C++／QML 消费者编译运行成功；迁移前保存的消费者可执行文件直接加载新库运行成功。新增诊断方法也通过安装头调用。
- 查看中文 1440px 桌面浅色及 420px Mobile 密度暗色 Gallery 截图；窄屏页面保留滚动布局。既有 Gallery 数据操作契约继续通过。

GPU 测试需要真实图形窗口，默认不加入无窗口 CTest。可选注册并运行：

```sh
cmake -S . -B build/release -DQCATUIKIT_TEST_CHART_GPU=ON
cmake --build build/release --target tst_chartrender
ctest --test-dir build/release -R '^chart_render$' --output-on-failure
```

Windows PowerShell 示例（先将 Qt 和本项目运行库目录加入 PATH）：

```powershell
$env:QSG_RHI_BACKEND = "d3d11"
& ./build/release/bin/tst_chartrender.exe -o chart-d3d11.txt,txt
$env:QT_SCALE_FACTOR = "2"
& ./build/release/bin/tst_chartrender.exe -o chart-dpr2.txt,txt
```

上传统计计量的是提交给 QRhi 的顶点更新字节；没有测量 GPU 时间戳、显存占用或百万图元 FPS。
本轮未验证 Qt 6.8、D3D12、Vulkan、Metal、Linux／macOS，也未注入设备丢失；不把 shader 构建成功当作这些平台的运行结果。

## 初始完整回归（2026-10-12）

截至 **2026-10-12**，最近一轮完整回归环境为 Linux、Qt 6.12.0、Debug、CMake／Ninja。图形自动测试使用 offscreen 平台插件和 Qt Quick software 后端。

| 检查 | 结果 |
| --- | --- |
| 共享库、Gallery 和全部测试构建 | 通过 |
| QCatUiKit_qmllint / qcat-gallery_qmllint | 通过 |
| 完整 CTest | 9/9 测试组通过 |
| QML 控件 | 226 项通过 |
| Gallery 页面与交互 | 573 项通过 |
| 图表核心 | 32 项通过 |
| Gallery 图表数据控制器 | 26 项通过 |
| Gallery 表格／对话控制器 | 13 项通过 |
| 翻译 | 中英文各 766 条完成，占位符一致 |
| 布局检查 | 1200 px 桌面与 420 px 窄屏，亮暗主题和桌面／触摸／移动密度 |

各 Qt Test 数量包含初始化和清理用例。该次快照不代表在所有平台、Qt 小版本和图形后端完成验证。

## macOS 输入与滚动复测：2026-10-14

环境：macOS arm64、Qt 6.10.2、Debug、CMake/Ninja；自动图形检查使用 offscreen 平台和 Qt Quick software 后端。

- 完整 CTest 11/11 组通过，其中 QML 控件 253 项、Gallery 581 项（包含初始化与清理）。
- 库与 Gallery 的两个 qmllint 目标无警告通过；Gallery 启动恢复正常。
- 验证数值输入的 Enter、Tab、点击其他控件提交、非法文本保留及纠正；整数输入与十六进制颜色输入包含失焦提交检查。
- LED 布局的 11 类数值参数均通过键盘编辑与应用检查；布局、预览设置、灯具映射覆盖直接应用、非法输入拦截、取消与重开，布局及预览还覆盖默认值恢复。
- Deck/Place 速度、设备发送间隔、图表及表格/对话压力测试参数包含无需 Enter 的提交检查。
- 滚动专项 32 项通过，包含按钮/复选框上开始的拖动、嵌套视图、文字选取与滚动条。
- 查看了实际 Gallery 的中文共享布局弹窗：1200 px 浅色桌面和 420 px 深色窄屏，输入框与操作按钮无裁切。

本轮没有验证 Metal 渲染、真实灯具输出或重新运行 Linux/Windows；不能将离屏结果视为这些平台/后端的验收。

## LED 工作流迭代：2026-10-13

环境：Ubuntu 26.04 x86_64、GCC 15.2、Qt 6.12.0 Release，OpenGL 4.6 / Mesa 26.1.6，
AMD Radeon 780M，使用本机 X11/xcb 图形会话。源码仍声明 Qt 6.8+；本轮未在 Qt 6.8、
Windows、macOS、Vulkan、Metal 或 D3D 后端重测新增能力。

- 库、Gallery、测试构建成功；`QCatUiKit_qmllint` 和 `qcat-gallery_qmllint` 无警告通过。
- 11 组 CTest 全部通过；`led_pipeline` 当前 14 项（含初始化/清理）通过，覆盖旧 alpha 与 W 区分、
  冷暖白 50% 混合、所有通道置黑、独立时钟/工作副本、布局更新中保留待加载预设、停止状态保持黑帧、
  三种 Profile 地址与重排、慢输出取消和无窗口设备分支。
- `tst_ledrender` 在 OpenGL 的 DPR 1 和 DPR 2 各 8 项通过；新增立体几何/旋转、场景尺寸、显示间距、
  矩阵切片、独立预览限帧/停用和预览设置不修改设备通道，保留旧像素、透明背景、拖拽、MSAA、跨窗口与资源重建。
- 新 QML 契约验证设置草稿/取消/默认/非法值、相机恢复、独立 Deck UI、只读数据源、键盘预选和跨数据源活动身份。
- 安装到独立目录后，LED 消费者编译并运行成功，包含旧 LedSource/LedPreview 和新公共头/转发头、LedDeck QML。
- 实际查看 1600px 宽深色 Gallery、430px 宽中文浅色 Gallery 和预览设置弹窗；全量 Gallery 契约覆盖中英文与 360px 页面。

一次 Circle 短时基准：三路各 2048 点、32×64 矩阵、pointSize=6、ScreenPixels、透视相机，
窗口 1600×840、DPR 1，A/B 视口 389×235、Output 390×235；Deck 请求间隔 16ms，
预览目标上限 60 FPS，暖机 2 秒、测量 4.948 秒：颜色更新约 62.45 Hz，三路 LED pass 各约 55.17 FPS，
窗口 frameSwapped 约 59.42 Hz。不是 GPU 时间戳或显示器扫描频率，不代表 Cube/Sphere 的性能。

可复现入口：

```sh
qcat-gallery --page led-preview --width 1600 --dark --led-count 2048 --led-benchmark 5 --led-report /tmp/led-benchmark.json
```

本轮验证仅使用 MemoryOutputBackend；没有进行实际灯具、Art-Net、sACN 或其它硬件协议联调。
日志、截图和原始基准 JSON 留在本地临时目录，不提交构建产物。

### 同日性能优化复测

使用上述相同平台、完整 Gallery、三个 Circle / ScreenPixels 预览，默认最大 60 FPS、
pointSize=6、16 ms 生产周期。窗口 1600×840、DPR 1，视口 A/B 为 389×235、Output 为 390×235。
将优化前的共享库保留在独立目录，以相同可执行文件和参数分别运行；各次暖机 2 秒，实际测量约 6.0–6.3 秒。

| 每路点数 | 优化前 A / Output / B 绘制 FPS | 优化后 A / Output / B 绘制 FPS |
| --- | --- | --- |
| 2048（32×64） | 54.13 / 54.60 / 54.60 | 59.19 / 59.19 / 59.19 |
| 100000（250×400） | 14.76 / 29.53 / 19.84 | 55.52 / 59.29 / 57.65 |

10 万点时，A/B 颜色生产从约 14.76/19.84 Hz 提升到 56.01/62.07 Hz；
窗口 frameSwapped 从约 40.64 Hz 提升到 59.61 Hz。以上是短时实测，不保证其它硬件或 Cube/Sphere 的帧率。

变化包括：按绝对时间安排限帧并补偿定时器迟到；内置效果在每帧开始时读取参数，避免每个灯点重复查字典；
白光直接转换成预览 RGBA，移除 RGB 中间缓冲；混合循环直接访问连续缓冲，跳过值为 1 的亮度计算。
公开帧仍深拷贝外部输入并保持不可变，独立通道、透明度、量化公式及默认形状保持原语义。

使用四种效果 × 三种 Schema × 四个正/负/零时间采样，对原始通道、三组参考色温预览、
目标 Schema 转换和独立透明度进行优化前后比较，输出字节一致。

优化后 11 组 CTest 全部通过（其中管线 15 项，含初始化/清理）；OpenGL 的 DPR 1/2 各 9 项通过。
新增覆盖限帧调整时重排等待、停用/恢复和空闲时不重绘，并校验直接 RGBA 转换的通道与透明度。
GPU 测试按实际经过时间验证限帧，截图等待三路帧及目标画面完成，避免用固定等待时间推断绘制完成。
两个 QML lint 目标无警告；安装后无需重新编译的独立消费者继续运行通过。
另以三个预览共享同一 2048 点 Source 验证独立限帧：A 设为 5 FPS、其余设为 60 FPS，
实测约 4.95 / 59.04 / 59.04 FPS，低帧率预览未拖慢其它实例。

复现当前版本的帧率：

```sh
qcat-gallery --page led-preview --width 1600 --dark --led-count 2048 --led-benchmark 6 --led-report /tmp/led-2048.json
qcat-gallery --page led-preview --width 1600 --dark --led-count 100000 --led-benchmark 6 --led-report /tmp/led-100000.json
```

M2 收尾 D03：已验证 Auto／平头／方头／圆头、斜角／尖角／圆角、尖角限制、虚线节点相位连续、断点和抗锯齿。C++ 148、QML 70、Gallery 141、D3D11 155 项全部通过；大数据平移测试也覆盖显式圆头圆角虚线。修复新增下拉框在语言切换时的显示项重置；触摸注入用例等待初始触点交付，避免同帧合并。QML lint、桌面／暗色 Mobile 截图检查、当前安装消费者及 14 个历史二进制通过。记录在本地忽略目录 chart-m2-completion/stroke-*。其余 M2 项仍按执行清单继续。

M2 收尾 D06：多停止点与透明插值、图片透明度、铺满／平铺、错误回退、纹理复用已验证。C++ 149、QML 71、Gallery 143、完整 D3D11 160 项通过；填充专项在 software、OpenGL、D3D11 DPR 2 各 7 项通过。QML lint、桌面／暗色 Mobile 截图、安装消费者及 15 个历史二进制通过。记录在 chart-m2-completion/fill-*。纹理上传计数不含管线使用的固定白色纹理；切换平铺模式会建立对应采样器资源。

M2 收尾 D07／D04：独立上下界严格对齐、线性并集插值、范围交集、断点／拒绝保留原数据、七种区间曲线 LOD，以及带断点线性 Line／Area 的 LOD 通过验证。C++ 158、QML 72、Gallery 144、Gallery 数据控制器 27、D3D11 169 项通过。十万区间的曲线缓存随平移复用，交叉、双边界孤立尖峰、缺口、轴反向／对数尺度、细节恢复和原始值拾取有覆盖。实际 Gallery 十万区间 Catmull-Rom 的 10 次平移准备耗时为 1.78–3.03 ms（当前机器／场景，非通用帧率承诺）。QML lint、桌面与暗色 Mobile 截图、安装消费者及 16 个历史二进制通过。记录在 chart-m2-completion/range-*。

M2 实例化与纹理标记：C++ 159、QML 73、Gallery 145、完整 D3D11 175 项通过；software 7、OpenGL 28、DPR 2 专项 12、无硬件实例化回退 27 项通过。4096 个方形标记上传 4096×28 字节，单标记变化仅补传 28 字节；透明叠加、图像缓存、旋转标签及窗口迁移通过。桌面与暗色 Mobile 截图、QML lint、安装消费者及 17 个历史二进制通过。记录在 chart-m2-completion/markers-*。

M2 QML 委托阶段：C++ 160、QML 76、Gallery 146、完整 D3D11 176 项通过。验证预算、对象复用／回收、组件销毁、错误根项回退、点／柱体／内外标签与旋转、GPU 内置回退的像素覆盖；QML lint 和桌面／暗色 Mobile 截图检查通过。安装消费者在 M2 最终验收统一复核。记录在 chart-m2-completion/delegates-*。

M2 选择与拾取：C++ 162、QML 77、Gallery 147、完整 D3D11 177 项通过。验证单选／多选、数据变更与移除清理、委托选择状态、选择高亮不上传数据、1 万散点的有界候选查询及全部既有曲线／区间精化命中。记录在 chart-m2-completion/selection-*。


## M2 最终验收（2026-10-17）

环境为 Windows 11、Qt 6.12.0、MSVC 2022 Release。M2 执行清单全部完成，包括多轴、线条外观、多停止点／纹理、独立区间对齐及 LOD、实例化和纹理标记、旋转标签、有预算 QML 委托、选择／多选与拾取索引、标签引线冲突处理及 reducedMotion 过渡。C++ 164、图表 QML 79、图表 Gallery 148 项通过；仓库 11 组 CTest 全部通过。两个 QML lint 目标通过，中英文没有未完成翻译，git diff --check 通过。

D3D11、D3D11 DPR 2、OpenGL 的完整渲染回归各 178 项通过。最终边界复核又覆盖了极大值柱体／区间选择：屏幕端点先限界再构造矩形，避免约 1e100–1e300 的柱值使可见边缘因精度抵消而消失；纵／横、正／负极大值的拾取也有 C++ 覆盖。该修改后，极值选择、外观淡入、选择零数据上传、委托回退、六种柱状布局、各标签位置及裁剪边框的 30 项专项在三个渲染环境全部复核通过。software 专项 33 项及新增边界 4 项通过。

检查了桌面和 420 像素暗色 Mobile 的选择、委托、过渡控件与多环引线标签；修正了过渡控件嵌套布局的循环重排。委托完成回调中修改源数据的 QML 用例通过，过期快照不会在该同步查询中用于拾取。过渡默认关闭，启用后按图层淡入；它不插值数据或几何。reducedMotion 立即结束过渡，数据更新不停止。

最终实际 Gallery 平移复测使用两路各 100,000 个原始点（共 200,000），1440×1000 窗口，每种样条连续 10 次平移：Catmull-Rom 的 CPU 准备为 1.53–2.15 ms，自然三次为 1.51–2.65 ms；两者绘制点数 5368、访问 LOD 节点 2682，曲线缓存构建计数均未增长。数字仅为当前机器与场景的 CPU 准备时间，包含拾取索引，不是 GPU 时间或帧率承诺。

安装后新消费者及 18 个历史二进制共 19 个全部通过；新消费者覆盖公开 C++／QML 元属性、预算委托创建与释放、选择状态和可选过渡，旧消费者无需重编译。新构建消费者保存为本地 charts-consumer-m2-complete.exe，供下一阶段兼容性检查。API 与对象布局继续使用 Pimpl，旧虚函数、Sample／RangeSample 和已有枚举值保持不变。

复现证据位于本地忽略目录 build-led-release/chart-m2-completion：final-ctest.txt、final-cpp.txt、final-qml.txt、final-gallery.txt、final-{d3d11,dpr2,opengl}.txt、final-edge-*.txt、final-software*.txt、final-consumers.json、final-pan.txt、final-capture.txt 与 final-*.png。D3D12／Vulkan／Metal、其他操作系统和 Qt 6.8 实际运行仍未验证；M3–M5 不属于本轮交付。


## M3 验收（2026-10-17）

Windows 11、Qt 6.12.0 / MSVC 2022，Release 构建。M3 的独立 XYZ／网格契约、相机、Phong 材质、柱阵／散点／轨迹／曲面、极坐标、深度拾取及切片已验收。接口和限制见 [CHARTS3D.md](CHARTS3D.md)。

| 检查 | 本轮结果 |
| --- | --- |
| CTest 总体回归 | 12 组通过，包含新增 charts_3d；收尾边界修改另运行最终三维测试 |
| 最终三维 C++ | 21 通过：数据拒绝与保留、网格／高度图、所有类型、缺失单元、极坐标原始记录、大坐标／最大有限值、相机裁剪、几何预算、重合点深度顺序、自定义柱基线与切片选择 |
| 三维 GPU | D3D11、D3D11 DPR 2、OpenGL 三环境均通过全套测试；另在各环境通过双图表、旋转／裁剪／半透明 layer 组合补测 |
| 既有二维 GPU | D3D11 全套 180 通过，覆盖曲线与 LOD、实例、纹理、选择、窗口迁移及合成 |
| QML／Gallery | 图表契约 79、Gallery 图表 149、可复制示例 104 通过；新增控件验证中英文切换保留选择值、曲面输入、第二组柱阵及窄屏布局 |
| lint／文案／差异 | QCatUiKit_qmllint、qcat-gallery_qmllint、git diff --check 通过；中英文 TS 无 unfinished 文案 |
| 安装消费者 | 当前消费者及 19 个历史已编译消费者共 20 个通过，包含 M2 完成版；新增两类真实头和两种转发头安装正确，私有头未安装 |

GPU 验证检查真实像素与最近可见对象拾取，包括正反视角遮挡、完全重合对象按绘制顺序拾取、独立环境／漫反射／镜面分量、球面明暗且外部无光晕、三种曲面模式、纹理和自定义网格、数据清空、鼠标和触摸、投影切换、窗口尺寸与跨窗口资源重建。旋转相机时 geometryBuilds、vertexUploads 和 uploadedBytes 保持不变；这证明复用几何，并不是 FPS 测量。

已检查真实 Gallery 的桌面浅色和 420 像素 Mobile 深色截图：四类三维图、分组柱、极坐标曲面、材质与切片控件。坐标框留白、外侧轴标签的边界与冲突处理、标题及控件布局已修整。选中标签优先于轴标题和刻度；拥挤时省略冲突的刻度。software 下明确显示三维不可用，二维回退保留。

本机可复查产物在忽略目录 `build-led-release/chart-m3/`：`ctest-first.txt`、`final-cpu.txt`、`final-gallery.txt`、`final-qml.txt`、`final-examples.txt`、`final-2d-render.txt`、`final-{d3d11,dpr2,opengl}.txt`、`final-composition-*.txt`、`final-capture.txt`、`final-consumers.json` 与 PNG。最终消费者归档为 `charts-consumer-m3-complete.exe`，供下一阶段动态库替换验证。

未验证 Qt 6.8 实际运行、其他操作系统、D3D12／Vulkan／Metal、真机触屏和设备丢失注入。当前三维没有 LOD、透明排序、实时阴影或模型文件解析；曲面是行列规则拓扑，不能视为任意三角网格曲面实现。三维刻度和几何预算的差异已在接口文档说明，不宣称与 Qt Graphs 全部 API 或像素结果一致。

## 三维光照优化（2026-10-17）

Windows 11、Qt 6.12.0 / MSVC 2022 Release。默认光照升级为 Blinn–Phong，加入 sRGB 解码／输出编码、半球环境补光、曝光和保留 RGB 比例的亮部压缩；Phong 保留旧版颜色计算。共享几何新增 RoundedCube、三档网格精度、球体极点退化面清理，以及轨迹／线框细管平滑法线。细管三角形绕序同时修正为与外向法线一致，防止背面翻转使线框偏暗。

| 检查 | 结果 |
| --- | --- |
| 三维 C++ | 24 通过，含新增参数拒绝、信号与几何复用，三档网格法线／绕序／边界及正负倒角柱基线 |
| 三维 GPU | D3D11、D3D11 DPR 2、OpenGL 最终各 22 通过 |
| Gallery 图表 | 149 通过，含模型／精度／倒角形状切换、补光／曝光／光泽、重置及中英文切换保留设置 |
| QML lint／文案 | 两个 lint 目标通过；中英文 TS 无 unfinished；差异空白检查通过 |
| 安装与 ABI | 当前消费者与 20 个历史二进制共 21 个通过，包含 M3 完成版；使用最终安装 DLL 与安装 QML 模块 |
| 全仓库 CTest | 10/12 组通过；两组既有失败已用修改前构建复现，见下文 |

GPU 像素检查覆盖：sRGB 128 在四分之一线性环境光下输出约 66（旧 Phong 为 32）、纹理与点颜色一致、上／下半球亮度差、强高光仍保留层次、相反光线／视线和零曝光、轮廓外无光晕、倒角轮廓与真实三角形拾取一致。模型、补光参数与相机连续切换时，geometryBuilds、vertexUploads、instanceUploads、uploadedBytes、textureUploads、bufferAllocations 均不增加。该检查验证缓存与上传行为，不代表 GPU 帧率基准；提高网格精度仍会增加顶点处理量。

已检查实际 Gallery 桌面浅色和 420 像素 Mobile 深色截图，以及球体／倒角立方体的 Phong 与 Blinn–Phong 对比；检查倒角分组柱、曲面、补光／曝光／精度控件的布局和显示。截图测试无警告。

全仓库失败分别为 `led_pipeline` 的 `layoutMappingAndValidation`（Custom 布局输入断言）和 `qml_contracts` 的 `SelectionControls::test_compactComboDisplayFits`（三档密度的英文文字宽度）。单独运行仍失败；使用本轮修改前、22:05 生成的 `build/Desktop_Qt_6_12_0_MSVC2022_64bit_Release` 二进制与其模块复现相同断言。此次未修改这些 LED／基础选择控件代码，不能将全仓库结果描述为全部通过。

本机证据保存在忽略目录 `build-led-release/chart-lighting/`：`cpu-final.txt`、`{d3d11,dpr2,opengl}-final.txt`、`gallery-charts.txt`、`lint.log`、`consumers.json`、`ctest.txt`、`baseline-{led,selection}.txt`、`capture-final.txt` 和 PNG。新消费者归档为 `charts-consumer-lighting.exe`。PBR、实时阴影、HDR／ICC 纹理管理和自动三维 LOD 不在本轮范围；其他图形后端与平台未新增验证。


## ABI 兼容性演进记录

以下保留各次接口变更及消费者检查证据；现行规则见 [ABI 指南](ABI.md)。

2026-10-18 M4 首批新增 ChartStack／ChartHistogram 两个独立 Pimpl 类型，分别注册为 CatChartStack／CatHistogram；真实头、带扩展名及无扩展名转发头一并安装。ChartSeries::Type 末尾追加 Histogram = 15，保留原有 −1–14 数值。统计来源与分箱信息保存于私有 SeriesRecord，通过既有 QVariantMap 返回；Sample、RangeSample、既有对象布局和虚函数保持不变，ABI 版本不变。本机新安装消费者及 22 个历史图表消费者通过库替换运行检查。

M4 第二批新增 ChartFit／ChartWaterfall 两个独立 Pimpl 类型，注册 CatChartFit／CatWaterfall，三种公共头路径均安装。ChartSeries::Type 末尾追加 Waterfall = 16，保留已有值；瀑布柱的原始增减值使用既有 Sample.value，统计属性继续保存在私有记录中。未修改已有类的对象布局、虚函数或 Sample／RangeSample 字段，ABI 版本不变。新安装消费者与 23 个历史二进制共 24 个通过，其中首批 M4 消费者未重新编译；该检查不代替其他平台的完整符号审查。

M4 完成批新增 ChartColorScale／ChartBubble／ChartGrid／ChartContour／ChartOhlc／ChartBoxPlot 六个独立 Pimpl 类型，注册同名 Cat 前缀 QML 类型；真实头、带扩展名及无扩展名转发头均安装。ChartSeries::Type 末尾追加 Candlestick = 17、Ohlc = 18、BoxPlot = 19、Grid = 20、Contour = 21，保留既有枚举数值。ChartItem 末尾追加 radarRanges 元属性，使用非虚访问方法和既有 appearanceChanged 信号，状态仍在 Impl。Sample／RangeSample、既有公开对象及虚函数布局不变，ABI 版本保持 1。

数值格网的图集、映射元数据、纹理资源标识及上传计数均属私有实现或既有 QVariantMap 诊断输出。ChartVertex 的私有布局调整为 28 字节，不进入安装头。新安装消费者及 24 个历史二进制共 25 个通过；新消费者同时实例化六个公开 C++／QML 类型，历史消费者无需重新编译。该检查仅覆盖本机 Windows／MSVC／Qt 6.12 Release 环境及所使用的接口。

2026-10-17 图表 RHI 迁移保留 ChartItem 的 QQuickItem 基类与 Pimpl 大小，以及 ChartAxis、ChartSeries 和 Sample 的布局。新增 `renderingStatistics()` 是非虚函数；DPR 处理覆盖 QQuickItem 已有的 `itemChange` 虚函数槽。私有 RHI 子项、快照和 QRhi 资源不进入安装头，SOVERSION 不变。已验证迁移前编译的图表消费者直接加载新安装库；该结果不替代跨平台完整符号审查。

同日的 M2 首批二维变体仅在 ChartSeries::Type 尾部追加 PercentBar、HorizontalBar、HorizontalStackedBar、HorizontalPercentBar（10–13），保留既有 −1–9 的值。CurveMode、MarkerShape 及五个样式属性使用非虚方法和既有 styleChanged 信号，新增状态全部存于 Impl；Sample 与公开对象布局不变。新安装头／QML 模块调用成功，M1 和迁移前编译的消费者均可直接加载新库运行。

M2 第二批在 Type 尾部追加 RangeArea = 14，新增独立 RangeSample 值类型及范围批次入口；不向既有 Sample 加字段。断点、下界及虚线／填充状态保存在私有记录或 Impl 中，新增方法均非虚，沿用 styleChanged／dataChanged。RangeSample 的已安装字段布局同样属于后续兼容边界。M2 首批、M1 和 RHI 迁移前的已编译消费者均完成新库替换运行检查。


LED 工作流迭代保留 LedSource/LedPreview 既有成员签名、枚举值、Position 布局和 Pimpl 对象大小。
新增属性及非虚函数通过现有 Impl 存储；SOVERSION 不变。新增 Layout/Frame/Registry/Repository/Deck/Mixer/Output/Project
类型与转发头作为同一安装包交付，应用不应依赖 `_p.h`。新公开值结构的字段同样成为后续 ABI 维护边界。

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

M2 最终安装复核：新消费者与 18 个历史二进制共 19 个全部通过。animationsEnabled／transitionRunning 以及选择／委托信号均追加到元对象末尾，动画和屏幕拾取索引继续保存在私有实现。具体记录见本文的 M2 最终验收。

### 三维接口演进

ChartItem 末尾追加 scene3D 元属性及非虚访问方法，既有基类、对象布局、枚举和信号保持不变。ChartSeries3D／ChartScene3D 是新导出 Pimpl 类型，XYZ Point 与旧 ChartSeries.Sample 完全分离。真实头、带扩展名和无扩展名转发头均安装，QML 类型随模块注册。qcatchart3d_p.h 和 RHI 类型不安装、不作为应用 API；私有测试入口的导出符号不承诺 ABI。M2 完成版消费者纳入后续动态库替换验证。

三维光照优化在 ChartScene3D 末尾追加 lightingModel、ambientGroundRatio、exposure 元属性及 LightingModel 枚举，在 ChartSeries3D 末尾追加 meshDetail 元属性及 MeshDetail 枚举；RoundedCube 追加到 Shape 末尾，原有值不变。访问方法均非虚，复用既有信号，状态保存于 Impl，公开对象与 Point 布局保持不变。默认光照改为 BlinnPhong 是外观行为变化；显式选择 Phong 可保留旧版光照计算。几何精度变化独立于光照模式，不承诺逐像素恢复旧网格。

### PNG 导出接口演进

ChartItem 末尾追加 exportingImage 元属性、exportingImageChanged／imageExportFinished 信号，以及 exportImage／exportingImage 非虚方法。抓取句柄、目标 URL、尺寸和定时器保存于 Impl；既有属性／信号、类大小、虚函数、枚举和 Sample／RangeSample 均不变，ABI 版本保持 1。图像抓取沿用 Qt Quick，不增加公共类型或运行时模块依赖。

### 姿态与体感接口演进

新增独立 Pimpl 类型 TiltController，注册同名 QML 类型；安装真实头、TiltController.h 和 TiltController 转发头。
CatAttitudeIndicator/CatTiltControlPanel 是新增 QML 类型，既有公开类、元属性、枚举、布局与 SOVERSION 保持不变。
未增加 Qt Sensors 或协议库依赖；新接口的四元数使用既有 QtGui 依赖。

本机 Linux / Qt 6.12 Release 的姿态安装消费者与历史图表二进制替换检查通过；此前 745 个 QCatUiKit 函数符号全部保留。该验证不代替其他平台的 ABI 检查。

## LED 圆片版本基准记录（2026-10-12）

2026-10-12，Windows 11 x64，MSVC 19.39 Release，Qt 6.12.0，NVIDIA GeForce RTX 5060 Ti，D3D11。Gallery 窗口 1200×840，DPR 1，Deck A/B backing 为 281×320，Output 为 278×320 物理像素。三路同时运行，灯点直径 6 逻辑像素，透视视图，混合 50%，Output 亮度 100%，sampleCount 1。每组暖机 2 秒、测量约 10 秒：

| 布局 | 每路点数（总点数） | 颜色提交 Hz | LED pass FPS（A / Output / B） | window frameSwapped Hz |
| --- | --- | --- | --- | --- |
| 二维矩阵 | 10,000（30,000） | 62.5 | 59.2 / 59.2 / 59.2 | 59.2 |
| 二维矩阵 | 100,000（300,000） | 62.5 | 59.8 / 59.9 / 59.9 | 59.9 |
| 三维球形 | 10,000（30,000） | 62.4 | 59.8 / 59.8 / 59.8 | 59.9 |
| 三维球形 | 100,000（300,000） | 62.5 | 59.6 / 59.7 / 59.6 | 59.7 |

另以三路各 100,000 球形点、`--led-interval 4`、暖机 2 秒、测量约 5 秒，测得颜色提交 213.4 Hz、三路 LED pass 与 frameSwapped 均约 59.8 Hz。数据生产和渲染频率独立，GUI 停滞的合并通知测试还验证了不会积累颜色帧队列。测量未包括 GPU 时间戳或显示器实际扫描完成；这些短时结果不保证更大窗口、不同显卡或长期负载稳定达到 60 FPS。

实际完成：干净编译库/Gallery/测试、两组 QML lint、10 组 CTest（Windows 离屏测试指定系统字体目录）、安装并构建运行独立 LED 消费者。真实窗口测试分别在 D3D11、OpenGL 和 D3D11 DPR 2 上通过，覆盖三路颜色、圆形裁剪及像素大小、深度遮挡、帧内存隔离、混合 0/50/100%、Output 亮度、鼠标左/中键/滚轮/双击、2D 忽略旋转但保留平移缩放、空/单点/重合布局、1 万/10 万动态布局、resize/MSAA、控件销毁重建、跨窗口以及场景图释放重建。Gallery 的英/中文和窄屏页面创建由回归测试覆盖，宽屏/窄屏还检查实际截图。

原始 JSON、日志、截图为本地构建产物，未加入版本控制。源码保持 Qt 6.8 最低要求，但本次没有实编/运行 Qt 6.8，也未验证 Linux/macOS、Vulkan、Metal、D3D12。上述结果只代表列出的实际工具链与环境。
