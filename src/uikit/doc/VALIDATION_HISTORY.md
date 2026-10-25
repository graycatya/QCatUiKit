# 验证执行记录

[文档索引](README.md) · [当前验证方法](VALIDATION.md)

仅保留当前版本依据、安装及跨平台检查和可复现基准。过期的实验版本、目录迁移过程和逐批开发日志已清理。各条结果仅适用于列出的日期、平台和范围，不代表本次全部重新执行。

## 文档与旧接口清理（2026-10-25）

核对公开头列表、QML 类型注册、矢量属性／方法实现、构建配置及当前文档。移除旧版本单例 `LibraryInfo`，版本入口统一为 `QCatUiKit`；移除 `thorvg_qrhi.h`／`tvg::QrhiCanvas` 别名及旧头接入回退，适配入口统一为 `qcatthorvgqrhicanvas.h`／`QCatUiKit::ThorvgQrhiCanvas`。旧 QML 调用方和外部实验源码需迁移；这些旧入口的源码兼容不再保留，未删除导出的 C++ 函数或改变公开对象布局。

同步架构、版本、组件目录、构建指南、矢量专题、中英文首页和验证方法。补充 CPU 几何准备与 GPU 绘制分工，将 Ready／renderedFrames 说明修正为成功记录绘制命令，不代表 GPU 完成或实际呈现。清理已过期的实验版本与逐批开发日志，保留安装、跨平台、ABI 和 LED 基准证据。仍有用途的标准安装头入口、LED RGBA8 接口和布局文件版本读取保留。

Windows Qt 6.12.0／MSVC Debug：全量构建通过；版本、QML 控件、Gallery 启动和矢量契约 CTest 4/4（61.85 秒）；原生 D3D11 7 passed、0 failed、0 skipped（2.165 秒）；库与 Gallery QML lint 通过。生成的 qmltypes 不含旧版本别名，托管源码不含旧适配头，适配文件来源哈希一致。未重复完整 Gallery 综合组、Release 全量、其他平台或旧源码兼容检查。

日志位于忽略的 `build/interface-audit-build.log`、`interface-audit-tests.log`、`interface-audit-vector-gpu-results.txt` 和 `interface-audit-lint.log`。文档校验及 Git diff 空白检查通过；名称覆盖检查不能代替所有 API 行为验收。

## 统一回官方 ThorVG v1.1.3（2026-10-25）

此前 QRhi 构建沿用了实验工作区对应的提交和 1.2.0 Meson 覆盖配置，偏离了指定版本。本次 CPU-only 与 QRhi 均统一到官方 `v1.1.3` 压缩包，SHA-256 为 `03850904c300e5969560d49446eab3e534c4160c379909f32bc49a32ea6f4072`。QRhi 由自有适配层提供，官方不包含 QRhi 不影响选择该版本。以下旧条目中的版本仅代表当时状态。

- 三个 Meson 覆盖入口重新基于 v1.1.3 生成，仅加入 QRhi 选项、配置和源码接入；保留官方版本、工具选项及其他配置，不从实验工作区带入新的核心实现。适配使用的 tvgRender.h／tvgCanvas.h 在两个基础版本中接口相同。
- Windows Qt 6.12.0／MSVC Debug 全量构建通过，Meson 报告 1.1.3。矢量契约 1/1（0.67 秒）、原生 D3D11 7 passed、0 failed、0 skipped（2.176 秒），库与 Gallery QML lint 通过。
- 新增消费者选项 `QCATUIKIT_THORVG_TEST_QRHI=ON`。独立 Release 工程链接含 QRhi 的静态库，实际运行检查 Initializer::version 为 1.1.3，创建自有画布，并验证 SVG CPU 中心像素；CTest 1/1（0.03 秒）。首次独立运行缺少 Qt DLL 路径，补齐 Windows CTest 的 Qt 运行路径后通过。
- 全新副本验证脚本增加 Meson 实际版本必须为 1.1.3 的断言；已用本次真实生成的项目元数据检查该断言，未重新运行整个无缓存流程。来源记录区分当前 v1.1.3 基础和最初实验代码来源，文件哈希与当前覆盖源码一致。

日志位于忽略的 `build/thorvg-v113-qrhi-build.log`、`thorvg-v113-qrhi-check.log`、`thorvg-v113-contracts.log`、`thorvg-v113-vector-gpu-results.txt` 和 `thorvg-v113-lint.log`。本次未重复完整 Gallery 综合测试、其他平台或其他 GPU 后端。重复步骤见当前验证说明。


## Qt 套件安装与双配置（2026-10-25）

后续补验全部既有消费者：`consumer`、`led-consumer`、`charts-consumer`、`attitude-consumer`、`dock-consumer`、`version_consumer`，均仅指定本机 Qt 6.12.0 MSVC 套件前缀，以 Ninja Multi-Config 独立构建。六个工程的 Debug／Release 编译及共 12 次运行全部通过（退出码 0）。运行时清除 QML 导入环境变量，仅使用 Qt 的 bin；图表程序显式传入该套件 qml 目录，姿态和 Dock 使用安装包导出的 QML 路径。版本参数为 `1.0.0-experimental experimental`。LED 未设置字体路径的首次离屏运行有 Qt 字体目录提示但两种配置均通过；其他离屏消费者设置 Windows 字体目录。日志位于忽略的 `build/installed-consumer-*.log` 和各消费者的 `build/*-{Debug,Release}.{stdout,stderr}.log`。本轮验证不包含真实 GPU 渲染。

Windows、Qt 6.12.0、MSVC 2022。默认 `QT` 安装布局取自所用 Qt 包，新增 Qt6 组件包装入口，保留独立包和 `STANDALONE` 布局。Windows Debug 库与 QML 插件使用 `d` 后缀；安装包要求构建时的同一 Qt 版本。

- 使用 `build/qt-module`、Ninja Multi-Config，关闭 Gallery 和常规测试，Release／Debug 均构建成功。全新配置默认前缀为所用 Qt 套件，QML 安装根目录为 `qml`。
- 两种配置先安装到 `build/qt-module-stage`。32 个公开头及两组转发头、两种库／插件、两种 CMake 配置导出和 Qt6 组件入口均已安装。暂存消费者使用 Qt 的附加包前缀，普通 QML 应用与纯插件消费者在两种模式下共四次运行退出码均为 0。
- 实际安装到本机 Qt 6.12.0 MSVC 套件。`tests/auto/cmake/qt-module-consumer` 只指定 Qt 前缀，`QT_ADDITIONAL_PACKAGES_PREFIX_PATH` 为空；CMake 配置和两种模式的构建无缺失插件目标警告。清除 QML 导入环境变量，仅用 Qt 的 bin 搜索 DLL，两个消费者各运行 Release／Debug，四次退出码均为 0。程序未调用 `addImportPath`；纯插件消费者仅链接 Qt Quick。普通消费者的 qmllint 通过。
- `STANDALONE` 模式另安装到 `build/qt-standalone-stage`，QML 位于 `lib/qml`；通过独立 `find_package(QCatUiKit)` 编译并运行 Dock 的 C++／QML 消费者，退出码为 0。随后将 `build/qt-module` 恢复为 `QT` 布局及本机 Qt 前缀。
- 既有 `build/dock-validation` 的库、Gallery、测试程序构建及库／Gallery qmllint 通过。未重跑 Linux、macOS、交叉编译、最低 Qt 版本、GPU 图形验收或旧二进制替换检查；非 Windows 的相对 RPATH 本次仅更新计算规则，未运行验收。
- `ctest --test-dir build/dock-validation --output-on-failure -j 2`：18/18 通过，耗时 148.81 秒；可选 GPU 组未开启。`python tools/check_docs.py`：34 份 Markdown、450 个本地链接、119 个公开类型通过。
- 日志保存在忽略的 `build/qt-module-*.log`、`build/qt-standalone*.log` 和 `build/qt-install-regression-*.log`。可复现步骤见 [当前验证方法](VALIDATION.md#qt-套件安装)。


## 测试可靠性与运行器（2026-10-24）

沿用下节 Windows 11、Qt 6.12.0、MSVC 2022 Release 环境，范围为全部已注册测试及四组可选 GPU 测试。仅修改测试、测试构建与验证文档，没有修改库或 Gallery 运行代码。

- 所有测试使用共同注册入口，默认超时 300 秒并保留原有更短限制；Windows 自动配置 Qt／库 DLL 路径及离屏字体目录。QML／Gallery 启动时检查字体数据库，缺少字体会失败。保留下拉框原有三种密度、原有宽度与完整文字可容纳断言，增加字体诊断信息。
- 修正 Custom 布局测试的 C++ 单元素嵌套列表构造，检查实际 XYZ 坐标，并补充平铺列表被拒绝、布局版本和点数保持不变的断言。
- LED 原有完整 GPU 像素、透明度、尺寸、资源重建与生命周期检查保留在不带工具栏的真实渲染项上；增加独立的真实 QML 鼠标输入用例，验证左键旋转、中键平移、滚轮缩放和双击复位的相机状态及 LED 像素。交互测试只排除顶部工具栏，纯渲染测试仍比较完整图像，没有扩大像素容差。滚动嵌套测试保留滚动归属检查，使用条件等待替代固定渲染等待；节流／静止等必须观察时间区间的等待保留。
- 预览设置与设备输出隔离测试先等待明确的 RGB/CW/WW 通道值 `00 00 00 4d cc`，避免以尚未更新的黑帧作基线。
- QtTest 报告逐组写入构建目录，由运行器转发给 CTest。反向验证使用仅位于忽略构建目录的 `QVERIFY2(false, ...)` 用例：运行器返回 1 并输出实际 FAIL；带空格的程序／报告路径验证通过。另验证不存在的程序启动失败会返回非零，并清空旧报告。未将这些预期失败计入正式套件的通过数。
- 第一轮全量 CTest（新日志运行器接入前）为 22/22，`-j 2` 耗时 121.79 秒。运行器接入后曾出现 18/22：四组 GPU 测试均在首个窗口显示检查失败，后续用例仍执行。定位为 Windows `execute_process` 隐藏首窗的启动行为，改由 `cmd /d /c call` 消费该启动标志，未改动窗口显示断言或延长其等待。该失败轮聚合日志保存在 `build/test-improvements-hidden-window.log`。
- 修正运行器后的 LED 专项执行 `ctest --test-dir <build> -R '^led_render$' --output-on-failure --repeat until-fail:3`，三次全部通过，每次 10 通过、0 失败、0 跳过，总耗时 23.98 秒。此命令遇到失败即停止，不是重试至通过。
- 最终版本执行 `ctest --test-dir <build> --output-on-failure -j 2`：22/22 组通过，0 组失败，耗时 122.41 秒，四组 GPU 均通过。最终聚合日志另存于 `build/test-improvements-final-ctest.log`；LED 三次重复验证的最后一次报告另存于 `build/test-improvements-led-repeat.txt`；运行器反向验证输出见 `build/failure-probes/negative-spaces-output.txt` 与 `negative-loader-output.txt`。

完整构建、`QCatUiKit_qmllint` 与 `qcat-gallery_qmllint` 已通过。可重复方法与日志位置见 VALIDATION。本轮不涵盖其他操作系统、最低 Qt 6.8、其他 GPU／驱动、真实输入设备和原生 Wayland 手工验收；没有新增跳过或预期失败标记。


## Windows 全量单元测试（2026-10-24）

环境：Windows 11、Qt 6.12.0、MSVC 2022 Release，使用现有 Desktop Release 构建目录。开启 `QCATUIKIT_TEST_LED_GPU=ON` 与 `QCATUIKIT_TEST_CHART_GPU=ON` 后重新配置、完整构建通过，再执行 `ctest --test-dir <build> --output-on-failure -j 1`。22 组中 19 组通过、3 组失败，总耗时 247.45 秒；包含三组图表 GPU 测试与 LED GPU 测试。未修改运行代码。

CTest 未捕获 QtTest 正文，因此对三个失败组使用 `-o <log>,txt` 单独复跑；QML 复跑沿用 CTest 的 offscreen/software 环境与构建目录 QML 导入路径。失败均复现：

- `led_render`：8 通过、1 失败。`LedRenderTests::framesCameraAndLifecycle()` 在 `tests/auto/led/tst_ledrender.cpp:364` 比较图像失败：鼠标双击复位后，窗口图像未恢复为原图。
- `led_pipeline`：14 通过、1 失败。`LedPipelineTests::layoutMappingAndValidation()` 在 `tests/auto/led/tst_ledpipeline.cpp:189` 的 `m.applyNow(c)` 返回 false；该处输入为单点 Custom 布局。
- `qml_contracts`：385 通过、3 失败、4 跳过。`SelectionControls::test_compactComboDisplayFits` 的 `density_0`、`density_1`、`density_2` 均在 `tests/auto/qml/data/tst_selectioncontrols.qml:90` 失败；可用宽度分别为 92、80、80，文字内容宽度均为 98。

其余 19 组（含 Gallery、Dock 与图表 GPU）均通过。日志保存在忽略的构建目录：`build/Desktop_Qt_6_12_0_MSVC2022_64bit_Release/Testing/Temporary/LastTest.log`，以及同目录的 `led-render-details.txt`、`led-pipeline-details.txt`、`qml-contracts-details.txt`。这些结果不包含真实鼠标手工验收、其他平台或 Qt 6.8 运行验证。


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

## ABI 兼容性演进记录

2026-10-18，Windows／MSVC／Qt 6.12 Release：图表统计与扩展类型的安装消费者及 24 个历史二进制共 25 个通过库替换检查。既有公开对象、Sample／RangeSample 字段及虚函数布局保持不变，新增枚举追加在末尾，ABI 保持 1。

图表 RHI、LED 工作流、三维材质、导出、姿态及 Dock 实现将新增状态放在 Pimpl 或私有渲染对象中。此前 Linux Qt 6.12 Release 的姿态安装消费者与历史图表二进制替换检查也通过，745 个既有函数符号保留。这些结果不代替其他工具链或后续接口改动的审查。

当前维护规则见 [ABI 指南](ABI.md)。2026-10-25 清理移除了旧 QML 版本别名与旧实验适配入口；它们的源码兼容不再保留，当前应用入口以版本及矢量专题为准。没有删除已导出的 C++ 函数或改变公开对象布局。

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
