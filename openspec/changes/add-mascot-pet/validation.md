# Validation Record

## Planning Checks

本轮仅创建规划文件。实现、构建、性能和任何平台实机验证均未执行。

2026-10-27 规划检查：

- `openspec.cmd status --change add-mascot-pet`：4/4 规划产物完成，所有实现任务保持未完成。
- `openspec.cmd validate add-mascot-pet --strict`：通过。
- `python tools/check_docs.py`：通过；检查 35 个 Markdown 文件、490 个本地链接与 122 个公开 Cat 类型。该脚本的既定范围不等于覆盖所有 change 文件，change 结构由 OpenSpec 严格验证补充检查。

环境说明：PowerShell 禁止执行 npm 的 `.ps1` shim，使用现有 `openspec.cmd` 完成同一 CLI 操作，未修改执行策略。Git 只在命令级指定当前仓库 safe.directory，未修改全局配置。

## Platform Acceptance Matrix

以下为验收计划，不是当前支持声明。每格初始为 Pending；实机失败改为 Failed，通过需附证据。具体系统、Qt、桌面及配套版本在原型阶段确定。

| 场景 | Windows | macOS | X11 | Wayland KWin | Wayland Mutter |
| --- | --- | --- | --- | --- | --- |
| 透明无边框、显示不抢焦点 | Pending | Pending | Pending | Pending | Pending |
| 可配置置顶、菜单与气泡键盘操作 | Pending | Pending | Pending | Pending | Pending |
| 角色区域交互、空白点击到背后窗口 | Pending | Pending | Pending | Pending | Pending |
| 整窗穿透与宿主／托盘恢复 | Pending | Pending | Pending | Pending | Pending |
| 拖拽、定位、平滑移动及取消 | Pending | Pending | Pending | Pending | Pending |
| 跨屏、混合 DPI、移除屏幕及状态恢复 | Pending | Pending | Pending | Pending | Pending |
| 气泡边缘避让、长文本、输入区域同步 | Pending | Pending | Pending | Pending | Pending |
| 托盘可用或不可用诊断及宿主恢复路径 | Pending | Pending | Pending | Pending | Pending |
| 精灵／Lottie 播放、隐藏及减少动效 | Pending | Pending | Pending | Pending | Pending |
| 多实例隔离、关闭隐藏、不退出宿主 | Pending | Pending | Pending | Pending | Pending |
| 配套缺失、禁用、版本错误、断开恢复 | Pending | Pending | Pending | Pending | Pending |

配套故障场景在不依赖配套的环境记录“不适用及原因”；Wayland 两环境必须实测。完整支持仅针对矩阵通过的明确版本组合，不外推到所有系统版本或所有 Wayland 合成器。

## Evidence Template

每次运行记录：日期、提交标识、系统版本、Qt 与工具链、图形后端、会话类型（原生 Wayland/XWayland/X11）、桌面／合成器及配套版本、屏幕布局与 DPI、执行命令、预期／实际结果、日志或截图位置、失败和未覆盖场景。

重复操作步骤后续维护在 `src/uikit/doc/VALIDATION.md`，本文件仅保留本 change 的实际结果及验收状态，归档时一并保留。


## Implementation Checks — 2026-10-28

任务 1.1：新增独立 `tools/mascot-probe`，采集 QPA、会话、Qt、系统、桌面、屏幕及图形后端。能力初始 Unknown，验收保持 Pending；未使用环境变量推断实际图形能力。

环境：Ubuntu 26.04 LTS，Qt 6.12.0 gcc_64，GNOME，会话变量为 Wayland。此次仅运行 offscreen/software，没有桌面交互证据。

- 独立 CMake/Ninja 配置与构建：通过。
- `ctest --test-dir /tmp/qcat-mascot-probe --output-on-failure`：1/1 通过，覆盖原生 Wayland、XWayland、X11、未知 X11 会话、headless、Windows/macOS 分类。
- offscreen/software `--json`：报告 Headless、Pending，所有能力 Unknown。
- offscreen/software 默认窗口启动：运行 3 秒，无 QML 错误，随后由 timeout 结束；不作为视觉或实机验收。

后续门槛：任务 1.2 需要 Windows 原型及背后窗口点击/焦点实测，当前 Linux 环境无法提供该证据。1.3–1.6 与全部平台验收仍未完成。探针目前只做诊断，不具备透明窗口或穿透交互测试装置。

角色素材已查看 `CatGrayWiki/logo/ip/model_voxel/desktop_pet` 的 README、manifest 和动作总览。其共享图集 startFrame、归一化 anchor 与现有设计草案不同；尚未复制素材、修改源资产或实现包兼容。素材自带的检查不能替代 Qt 播放验证。


## Character Package and Sprite Playback — 2026-10-28

用户明确授权先继续角色包解析与播放，保留平台原型和实机验收待办。本阶段不缩减两个 delta specs 的剩余要求，不同步主规格，不归档变更。

已实现任务 2.1、2.2、3.1、3.4，总进度 5/36（含此前 1.1）：

- 新增 Pimpl 公共类型 MascotPetController，维护真实头、两种转发头、QML 注册与私有包含目录。内部 MascotSprite 使用 Qt Quick Image、裁剪和统一时钟选择图集帧，不公开页面内嵌组件。
- mascot.json 支持共享图集 startFrame；兼容 CatGray manifest.json 的单图集、归一化 anchor、animations 与 stateMap.Idle。适配仅在内存执行。
- 校验本地/qrc 来源、版本、尺寸／锚点／区域／数值、图片解码及预算、动作与帧范围；拒绝网络 URL、父路径、符号链接越界。延后加载携带代次，切包与销毁使旧任务失效。
- 默认底态、有限／无限循环、Replace/FIFO/IgnoreIfBusy、64 项队列、请求标识及明确终态；暂停、有效可见性和 reducedMotion 冻结进度，恢复不补放。
- 新增普通窗口开发工具 qcat-mascot-player，内置 CatGray 十动作精灵包，支持动作、暂停、缩放与减少动效。只复制三份运行资源并保留许可，三个副本 SHA-256 均与源文件一致；未改动外部 CatGrayWiki 目录。
- 同步接口专题、组件目录、维护入口、构建说明、ABI、UI 技能及文档覆盖检查。

环境与构建：Ubuntu 26.04 LTS，Qt 6.12.0 gcc_64，GCC 15.2，Debug/Ninja。读取当前 CMakeCache 后选用 build/Desktop_Qt_6_12_0-Debug；根 build 缓存使用其他工具链，未复用。原 ccache 默认目录只读，构建命令通过 CCACHE_DIR=/tmp/qcat-ccache 与 CCACHE_TEMPDIR=/tmp/qcat-ccache-tmp 使用临时缓存，未更改全局配置。

执行结果：

- `cmake --build build/Desktop_Qt_6_12_0-Debug --parallel 4`：通过（最终执行使用上述临时缓存变量）。
- `ctest --test-dir build/Desktop_Qt_6_12_0-Debug -R '^mascot_contracts$' --output-on-failure`：通过，23 个 QtTest 用例（含数据行）全部通过。涵盖本地／qrc 目录、资源根、十动作清单、版本、尺寸、默认动作、路径、符号链接、远程 URL、帧范围、FPS、loops、区域、资源缺失／损坏、清单超限、代次／销毁、Lottie 明确错误、队列、暂停与实例隔离。
- 颜色网格实际渲染检查：跨行帧序、镜像、8→32 像素缩放通过；窗口隐藏与 reducedMotion 冻结检查通过。有限动作完成事件无伪造，默认底态不重复发宿主终态。
- `cmake --build build/Desktop_Qt_6_12_0-Debug --target QCatUiKit_qmllint qcat-gallery_qmllint --parallel 4`：通过。独立对工具 Main.qml 执行 qmllint（导入当前 build/qml）：通过。
- 安装至 `/tmp/qcat-mascot-install`，独立配置并编译 tools/mascot-player 至 `/tmp/qcat-player-consumer`：通过。仅使用安装头和目标，不包含 Gallery／平台私有头；运行时显式指定安装 QML 根目录，成功读取内置 qrc 精灵包并生成说话动作截图。
- offscreen/software 人工截图检查：760×660 浅色 working、420×520 深色 thinking，以及安装消费者 speaking，角色、动作选项和文本无裁切；无绑定／布局错误。截图保存在本次会话 `/tmp/qcat-mascot-*.png`，不提交临时产物。此检查不代表 GPU 或实机窗口验收。
- `python tools/check_docs.py`：通过，36 个 Markdown 文件、498 个本地链接、397 个公开属性及 179 个 Q_INVOKABLE 名称覆盖。
- `openspec validate add-mascot-pet --strict`、`git diff --check`：通过。
- `ctest --test-dir build/Desktop_Qt_6_12_0-Debug --output-on-failure`：20/20 通过，耗时 367.23 秒；其中 Gallery 845 个 QtTest 用例通过，角色模块最终 23 个用例通过。最终增加资源根路径与 Lottie 明确错误检查后，另行重建并重跑受影响角色测试，全部通过。

未完成：2.3 Lottie 播放、2.4 第二类示例包、拖拽反馈、区域命中／输入适配、公共桌面窗口、气泡、托盘、状态迁移、Gallery 完整页面及所有平台验收。Lottie 基本清单和外部资源边界可校验；尚未接入播放器时控制器明确返回 RendererUnavailable，不报告 Ready 或 Completed。当前图集上限固定 4096，GPU 纹理能力仍待实测；本阶段未验证包文件在校验之后被外部修改的运行时错误反馈。平台矩阵继续全部 Pending。任务 7.1 保留未完成，待完整功能交付后再做最终验收。


## Lottie and Character Interaction — 2026-10-28

本阶段完成 2.3、2.4、3.2、3.3，总进度 9/36。本节更新上一阶段关于 Lottie／拖拽／区域命中的待办状态，保留先前执行证据。平台验收矩阵仍全部 Pending，不同步主规格或归档。

实现：

- 私有 MascotRenderAdapter 将 CatLottieAnimation 绑定至控制器，autoPlay=false；Lottie 使用既有 VectorItem 时钟与 finished 回调，不增加完成时长推算。有效 QRhi 反馈后 Ready，动作切换可再次 Loading；未挂接视觉时等待渲染。控制器的精灵时钟不推进 Lottie。
- 统一 RenderFailed：后端或运行时图片失败使当前请求 Failed、队列 Cancelled，冻结动画并保持 Error。包内 fallbackImage 可用时显示图片，否则显示默认占位。延后图片错误回调携带包代次；旧反馈不能覆盖新包。
- 新增原创 Vector Cat Lottie 包，idle／wave 分文件并附 Apache-2.0 来源／许可，保留 CatGray 精灵包原许可。普通开发工具可切换两包，新增 --lottie；未引入 lottie-web、JS 播放器、第三方服务或音频。CatGray 原始矢量资源仅在 /tmp 包中试播，未改写或纳入该原创示例的许可。
- 新增 dragEnabled、dragAction、dragging、beginDrag/endDrag；有效反馈无限循环，抢占当前并取消队列，期间新请求明确 InteractionBusy，结束回默认底态；无反馈时保持调度。切包与禁用结束交互。拖拽 API 不负责原生窗口移动。
- 新增 interactionRegions、hitTest、activateRegion 与 regionActivated；支持三种形状、后声明优先、用途筛选、动作整体覆盖、镜像与缩放、原始逻辑坐标反馈。私有 TapHandler 实际接入点击；区域及锚点有独立变更通知，未接入原生输入范围。

环境和命令：继续使用 Qt 6.12.0 / GCC 15.2 / Debug/Ninja 与现有 build/Desktop_Qt_6_12_0-Debug，构建使用临时 ccache 目录。有效图形测试通过本机显示服务执行：QPA=xcb，宿主会话变量 Wayland / GNOME，因此分类为 XWayland。QSG 日志明确创建 Vulkan QRhi，选择 AMD Radeon 780M Graphics (RADV PHOENIX)，驱动 26.1.6、Vulkan API 1.4.354；不是原生 Wayland 验收。

结果：

- 完整构建：通过。
- `ctest --test-dir build/Desktop_Qt_6_12_0-Debug --output-on-failure`：20/20 通过，395.85 秒；Gallery 845 个用例通过。最终锚点错误通知复查后，重建并重跑受影响角色模块：27 个 QtTest 用例全部通过，约 3.7 秒。损坏图片用例产生预期 Image 解码警告，随后断言 RenderFailed／Failed／Cancelled。
- `QCAT_MASCOT_GPU=1 QT_QPA_PLATFORM=xcb QSG_RHI_BACKEND=vulkan .../tst_mascot lottieRendering`：通过；一个 GPU 综合场景（含初始化／清理共 3 项）验证像素内容、进度、暂停、隐藏恢复、有限完成、无限拖拽反馈、InteractionBusy、切包取消与旧反馈隔离。有效场景运行约 13.6 秒。
- 独立配置 `/tmp/qcat-mascot-no-vector`，设置 QCATUIKIT_ENABLE_THORVG=OFF、QCATUIKIT_BUILD_THORVG=OFF、QCATUIKIT_BUILD_EXAMPLES=OFF；构建 tst_mascot 后运行 mascot_contracts：27 个用例全部通过，明确错误与静态占位可观察。常规软件后端同一测试通过。
- 库及 Gallery qmllint：通过；独立工具 Main.qml 的 qmllint：通过。私有桥接属性在 qmltypes 中使用完整命名空间，未依赖未解析类型。
- 安装到 `/tmp/qcat-mascot-install` 后独立重建 tools/mascot-player 消费者：通过；使用安装 QML 根目录运行 Lottie/Vulkan，显示并自动退出。公开头不引入平台或桥接私有头。
- 原始 CatGray 的 Blink.json 与 Mouth_Talk.json 复制至 `/tmp/qcat-catgray-lottie`，添加文档中的标准清单，本地目录加载及 talk 动作通过 Vulkan 实际显示。临时截图 `/tmp/qcat-catgray-lottie.png` 已人工检查；未改变外部源资产。
- 原创 Lottie 包的 420×560 深色预览与安装消费者截图已人工检查，角色、包切换、动作和底部控件无裁切。正常演示无绑定／布局错误。临时截图和日志不提交仓库。
- 环境探索记录：沙箱内不能连接本机 X 服务，原生验证在获准访问本机显示服务后完成。尝试强制 offscreen+rhi/Vulkan 时，Qt 插件报告不支持 createPlatformVulkanInstance，初始化失败并以 139 退出；改用已验证的 xcb/Vulkan 环境。未把该失败或普通 offscreen 软件检查作为 GPU 成功证据。
- 文档检查、OpenSpec 严格验证与 diff 空白检查通过；最终文档检查范围为 36 个 Markdown、499 个本地链接、402 个公开属性与 183 个可调用方法名称覆盖。

后续仍包括桌面窗口及平台适配、气泡、托盘、状态导出恢复、Gallery 完整演示与翻译、平台原型和实机矩阵。任务 7.1 继续保留待最终交付检查；当前渲染与交互结果不改变平台 Pending。固定图集预算及特定 GPU 纹理能力验证的限制继续保留。

## 2026-10-28 CatGray 实际 IP Lottie 素材纠正

此前 Vector Cat 是手绘几何测试夹具，不能作为 CatGray IP 素材验收依据。本轮按所有者要求，从 `CatGrayWiki/logo/ip/model_voxel/desktop_pet` 的原图集新生成 `desktop_pet_lottie`：10 动作、240 帧、192×208、8 FPS，保留源模型和精灵。转换为每帧 32 色、alpha≥128 的纯矢量矩形路径；存在阴影量化和透明边缘差异，并非直接从 Blender 网格导出的连续矢量骨骼动画。输出含生成脚本、专用授权、来源 SHA-256 和动作 SHA-256；全部输出校验一致、assets 为空、所有层为 shape，无嵌入位图。

预览工具 `--lottie` 和按钮改为 CatGray 实材 idle / working，两动作运行副本与 IP 目录生成输出一致；几何猫仅保留为确定性测试夹具。启动参数 `--action working` 的截图仍显示 idle，预览工具的启动动作时序问题留待修复；独立实材测试在 Ready 后明确提交 working 并验证完成。

- Qt 6.12.0，xcb / Vulkan（GNOME Wayland 会话中的 XWayland），`QCAT_CATGRAY_LOTTIE_PACKAGE` 指向 IP 目录的新包：`catgrayLottieRendering` 3 passed、0 failed（含 init/cleanup，9.96 秒）。断言实际像素面积、进度、暂停恢复、working 单次 Completed 及返回 idle。
- 常规 offscreen 回归：27 passed、0 failed、1 skipped（实材 GPU 测试未设置环境变量时跳过）。
- 预览工具构建通过，内置 CatGray Lottie 截图可见实际体素角色。截图 `/tmp/qcat-real-ip-working.png` 仅为本机临时证据。
- 文档检查通过：36 Markdown / 500 links。平台矩阵任务继续待验收；本结果不代表原生 Wayland 或桌面悬浮能力通过。

## 2026-10-28 从桌宠 Blender 模型直接导出完整 Lottie

按所有者要求，弃用此前 192×208 精灵描摹和 32 色量化的生产素材，直接只读加载 `desktop_pet/CatGray_DesktopPet.blend`，采样求值网格、原正交相机、骨骼、表情和 working 键盘。idle / success / error 沿用原桌宠程序姿态，其余动作读取 blend 内的 Action。投影模型面、按仿射深度裁剪遮挡、合并同色可见面；输出为矢量路径和预合成，不包含图像资产、纹理、外链或嵌入位图。固定方向的平面光照与 Cycles 渲染有差异，体素造型自身的棱角仍保留。几何计算使用 0.001 画布像素精度网格和 0.001 模型单位的共面深度噪声容差。

- 原 blend SHA-256：`324968b5e0361a8245dbbcb7a08520bdbdf3b00432a61d364de5deabb207468f`，导出及安装前后均一致，未保存或改写源模型；原精灵未改动。
- Blender 5.2.2 LTS；纯矢量画布 768×832，24 FPS，10 动作 / 720 帧 / 30 秒。运行逻辑尺寸仍为 192×208，锚点及交互坐标保持一致。全部动作 JSON 共 135,482,884 字节，各文档小于解析器 64 MiB 上限；fallback 单独渲染为 768×832 PNG。
- 输出同时替换 `CatGrayWiki/logo/ip/model_voxel/desktop_pet_lottie` 和 `tools/mascot-player/catgray-lottie`；十动作文件 SHA-256 逐一一致。`generation.json` 记录 blend、姿态辅助脚本、生成器和动作校验和；`verification.json` 记录帧数、矢量类型和边界检查。保留角色专用授权，移除旧精灵转换生成器。生成脚本为 `tools/mascot-player/export_catgray_blend_lottie.py`，通过已有结果复核来源与代码校验和后，十动作均可正确复用。
- 全部采样帧通过非空及画布内边界断言；两个相交平面的构造检查通过：按深度在交线处分割、无重叠、面积全覆盖。单张两倍尺寸 idle 实渲染与同相机 Cycles fallback 的轮廓 IoU 约 0.980，作为相机及轮廓辅助检查，不代表逐帧像素一致。
- 新内置包已注册全部十动作资源，预览下拉框直接使用全部动作清单。启动动作改为等待 Ready 后提交，截图等待目标动作生效及稳定窗口；`--scale` 支持直接检查放大轮廓。
- 真实窗口：Qt 6.12.0 / xcb / Vulkan（GNOME Wayland 会话中的 XWayland）；`lottieRendering catgrayLottieRendering`：4 passed、0 failed、0 skipped（含 init/cleanup，17.969 秒）。实材检查覆盖十动作各自的加载、播放进度和非空画面，两倍逻辑尺寸渲染，暂停恢复，以及 working 单次 Completed 后返回 idle。并未声称每个动作全部帧均经过逐像素 GPU 比对。
- 常规 offscreen/software 回归：27 passed、0 failed、1 skipped（实材 GPU 测试未启用时跳过），3.629 秒。预览工具及测试构建、预览 QML lint、文档检查、OpenSpec 严格验证和差异空白检查通过。
- 内置包 `--lottie --action working --scale 2` 实际截图退出码 0，截图显示 working 与键盘，修复此前启动参数截图仍显示 idle 的问题。临时证据 `/tmp/qcat-blend-final-working.png`；原有 1.2 秒固定截图时刻已改为就绪和动作匹配后截图，30 秒超时失败。

另以 `--package` 直接加载 IP 目录的新包，speaking 动作在深色 420×560 窄窗口截图退出码 0，布局及角色显示通过人工检查；临时证据 `/tmp/qcat-blend-final-speaking-dark.png`。

本轮仅完成素材及播放验证；桌面悬浮、穿透、托盘等实现和各平台验收任务继续保留待办。没有将 XWayland 普通预览窗口结果当作原生 Wayland 或其他平台支持证据。

## 2026-10-28 验证程序归入 tests/manual，删除几何猫

按所有者要求，将 `tools/mascot-probe` 和 `tools/mascot-player` 分别迁移到 `tests/manual/mascot-probe`、`tests/manual/mascot-player`。本文件此前的 tools 路径是历史执行位置，保留原始验证记录。当前命令及入口见 `src/uikit/doc/VALIDATION.md`。

移除播放器在 examples 下的构建入口；启用 `BUILD_TESTING=ON` 和显式 `QCATUIKIT_BUILD_MANUAL_TESTS=ON` 后，从 tests/manual 构建两个程序，默认关闭手动程序构建。探测器同时支持独立配置，不再在嵌入根项目时重建 project 或重复引入 CTest。自动测试不依赖手动可执行文件的构建。

完整删除 `vector-cat` 素材、qrc 资源登记、许可声明及现行文档引用。Lottie 调度测试改用完整 CatGray 包：working 用 loops=1 验证自然完成，有限 jump 动作用于无限拖拽反馈验证；保留暂停、隐藏恢复、抢占、队列及切包取消检查。十动作实材测试继续保留。两个程序仅迁移源码路径，内部 qrc URL 保持一致。

素材生成器从仓库的测试程序中移除；删除前确认与 IP 项目 `desktop_pet_lottie/export_catgray_blend_lottie.py` 副本逐字节一致，IP 项目的生成器及素材未改动。角色专用授权随手动预览素材保留。

- 根项目配置和 qcat-mascot-probe / qcat-mascot-player / tst_mascot 构建通过。
- CTest：mascot_contracts、mascot_probe_classification，2/2 通过（7.61 秒）。
- 探测器从 tests/manual/mascot-probe 独立配置、构建和分类测试通过，1/1。
- Qt 6.12.0 / xcb / Vulkan（XWayland）：lottieRendering、catgrayLottieRendering 共 4 passed（含 init/cleanup）、0 failed、0 skipped，29.448 秒；全部使用 CatGray 素材。
- 文档检查通过（36 Markdown / 500 本地链接），差异空白检查通过。平台验收任务保持待办。

迁移后的播放器从新构建目录以 `--lottie --action working --scale 2` 实际截图退出码 0，临时证据 `/tmp/qcat-moved-player.png`。播放器从 tests/manual/mascot-player 使用已安装 QCatUiKit 独立配置通过。现行源码、构建入口及文档不存在旧 tools 路径或 vector-cat 引用；历史验证段落保留当时路径。

## 2026-10-28 大角色包后台加载与原画面保留

完成新增任务 2.5–2.7。包加载由 GUI 上的零延时回调改为后台完整校验，最多两个校验任务并行。切包、清空和销毁使用取消标记及加载代次；在动作与读块边界协作取消，已经开始的单次 JSON 解析完成后丢弃。完整来源、版本、数值、图集及 Lottie 外部资源检查继续执行，没有延后或跳过未选择动作的校验。

VectorItem 本地/qrc 文档读取改为后台读块，上限保持 64 MiB；Lottie CPU 解析与场景物化在另一个最多两线程的队列中完成。任务不捕获 UI 对象裸指针；任务结果只在当前代次采用。ThorVG 每个线程 init/term 配对，渲染器持有后端引用并在取消时先回收已发布场景；CPU 场景独占移交，QRhi 创建、画布挂接和绘制仍在渲染线程。UI 对象销毁不等待解析，应用退出通过收尾钩子等待后台任务结束后再释放全局资源。

VectorItem 新源加载时保留并冻结旧场景；私有角色视觉在换包校验期间保留上一个画面及逻辑尺寸，但不暴露旧交互状态。首次加载无旧画面时正常显示加载提示，错误和 Null 仍明确占位或清空。修复保留画面后同一动作 URL 被误用为已加载缓存的问题：新包加载代次可强制重读，实测同路径 JSON 的总帧数从 96 改为 48 后能取得新元数据。手动预览增加 Loading 文本。

补充 R10 异步资源加载约束、设计、VECTOR/MASCOTPET 指南和 VALIDATION 中可重复的诊断命令。日志类别 `qcatuikit.mascot.loading`、`qcatuikit.vector.loading` 默认关闭 debug，显式开启后分别报告完整包校验、读取、CPU 准备、画布挂接及首帧绘制命令耗时。未引入新公共属性、方法或 QtConcurrent 依赖，公开 Pimpl 类布局保持不变。

验证结果（Qt 6.12.0，当前 Desktop kit）：

- 完整构建及库／Gallery qmllint、手动播放器 QML lint 通过。
- 完整 CTest：21/21 通过，228.18 秒（`-j 4`）；Gallery 相关测试同时通过。mascot_contracts：28 passed / 0 failed / 2 skipped，13.356 秒；大包校验期间 GUI 心跳 392 次、记录最大间隔 13 ms，旧任务不能覆盖新包、加载期间销毁检查通过。
- 关闭 ThorVG 的独立构建（manual tests OFF）构建通过；vector_contracts / mascot_contracts 2/2 通过，13.87 秒。关闭后端或 software 仍明确 Error，没有伪 Ready / Completed。
- 原生图形窗口使用 xcb / Vulkan（GNOME Wayland 会话中的 XWayland）：VectorTest 8 passed、0 failed、0 skipped，1.631 秒；覆盖 SVG、内存 Lottie、Canvas、重新加载和资源销毁。
- 同环境下 lottieRendering / catgrayLottieRendering / asynchronousLottieHandoff：5 passed（含 init/cleanup）、0 failed、0 skipped，39.935 秒。覆盖实际十动作、暂停、隐藏、有限动作完成、无限拖拽反馈、队列取消、换包期间视觉保留、同路径内容重读、快速换源及解码期间销毁。
- 单次约 18 MB idle 文档观测：后台读取 14 ms、CPU 场景准备 911 ms、渲染挂接 0 ms、首帧绘制命令 5 ms；该次 Loading 期间 GUI 心跳 75 次、最大间隔 34 ms，56 次窗口采样均非空白。再次加载同文档观测 CPU 921 ms、挂接 14 ms、首帧命令 5 ms。数据来自本机热缓存与当前测试调度，不作为冷启动或跨设备固定时延保证。
- 安装至 `/tmp/qcat-async-install`，仅用安装公共头与目标构建独立 C++ 消费者，验证后台包校验与文件错误异步反馈：通过。首次未显式选择运行库路径时混入系统 Qt 6.10 Network，报 Qt 私有 ABI 符号错误；指定本 kit 的 Qt lib 和安装库目录后退出码 0，不是代码失败的绕过。
- 手动播放器通过内置 qrc 加载完整包，`--lottie --action working --scale 2 --screenshot` 退出码 0；截图实际显示 working 及键盘，临时证据 `/tmp/qcat-async-player-final.png`。
- 文档检查通过：36 Markdown / 501 本地链接。OpenSpec 严格验证及差异空白检查通过。

本轮减少的是 GUI/渲染线程上的长时间读取与 Lottie CPU 准备阻塞；加载总时长仍受素材结构影响，GPU 首帧路径处理与上传仍可能产生开销。没有增加全动作预加载、动画缓存或素材减重。场景内部单次解码不强制抢占；已开始的取消任务可短暂继续计算后回收。原平台能力和平台验收矩阵继续待办，不以普通预览窗口结果声明原生 Wayland 或其他平台完整支持。

## 2026-10-28 动作场景缓存、常用预热与切换测量

用户授权后新增任务 2.8 和 R11。私有 mascot 集成为每实例启用独占场景缓存，最多两个非活动场景，源文档准入成本合计 48 MiB，活动场景另计。已播放动作保留 Animation 及 QRhi canvas；预热动作后台读取和准备 CPU 场景，首次使用仍实际挂接、绘制。默认动作、配置的拖拽动作及可选 working / jump 选择至多三个来源，预热逐个执行且不主动淘汰已有缓存。普通 VectorItem 默认加载语义不变，没有新增公共 QML 属性或改变公开对象布局。

切包、清空、显式 reload、QRhi 更换及销毁会取消或失效缓存；旧包保留画面不能进入新包缓存。冷动作准备期间继续显示旧画面。源文件仍后台读取检查，命中主要省去 CPU 解析，resident 同时省去画布重建；未声称命中零 I/O 或首帧 GPU 零成本。日志区分命中、淘汰、预热完成和执行开始到首帧渲染反馈。

环境：Windows 11，Qt 6.12.0 msvc2022_64，MSVC 14.39，Release/Ninja。读取现有 CMakeCache，使用 `build/Desktop_Qt_6_12_0_MSVC2022_64bit_Release`。`QSG_INFO=1` 确认 D3D11、NVIDIA GeForce RTX 5060 Ti。只验证普通 Qt Quick 测试窗口，桌面窗口及全部平台矩阵保持 Pending。

独立 `tst_mascot.exe actionCacheAndLatency`（`QCAT_MASCOT_GPU=1`）最终结果：3 passed / 0 failed / 0 skipped，10.955 秒。覆盖预热命中、20 次 idle / working 往返无重复 CPU 解析、非空画面、从 192×208 调整到 384×416 后复用画布、第四动作触发淘汰、显式 reload 重解码、清空及同包重载失效、预热期间销毁。

| 指标 | 本机本次结果 | 边界 |
| --- | --- | --- |
| 冷包加载到 Ready | 4210 ms | 包含完整包及资源校验，不与冷动作直接比较 |
| 首次 idle 动作首帧反馈 | 269 ms | 从动作执行开始，包含读取、解码、绘制及 GUI 反馈 |
| 预热 working 请求到 Ready | 38 ms | 同次动作首帧反馈日志为 36 ms |
| 缓存往返请求到 Ready，20 次 | P50 38 ms，P95 41 ms | 后半段使用两倍窗口尺寸，无重复 CPU 解析 |
| 未缓存 thinking 首帧反馈 | 281 ms | CPU 解码 235 ms；低频冷动作仍有等待 |
| 整段缓存测试进程峰值工作集 | 558.6 MiB | PowerShell 每 100 ms 读取进程 PeakWorkingSet64；含包校验及临时解析，不是缓存常驻内存 |

截图 `build/mascot-cache-resident.png` 为本机临时产物，已查看并确认两倍尺寸的 working 角色和键盘正确显示。日志 `build/mascot-cache-final-latency.txt` 为本机临时证据。未做屏幕扫描时刻或 GPU fence 测量，Ready / 渲染反馈不能等同于显示器输出；执行日志不含 Enqueue 等待。样本来自单机热文件缓存和当前素材，不作为跨设备 50 ms 保证，也未完成缓存开关同条件的常驻内存对照。

最终源码的完整构建、库及 Gallery qmllint 通过。独立既有 GPU 回归 `lottieRendering catgrayLottieRendering asynchronousLottieHandoff`：5 passed / 0 failed / 0 skipped，28.207 秒，覆盖十动作、暂停、隐藏、自然完成、包切换和读取期间销毁。加载时 GUI 心跳 24 次，最大间隔 17 ms，24 次窗口采样均非空；该次进程曾观测到约 613 MiB 峰值工作集，不与上述不同测试作内存优劣对照。

验证环境修正：直接调用 cl.exe 的首轮构建缺少 MSVC 标准库 INCLUDE；改为通过现有 vcvars64.bat 初始化工具链后构建通过。首次以 Hidden 启动 GPU 测试未产生可曝光窗口，停留 Loading 并超时；改为普通可见测试窗口通过。测试程序同时修正等待动作异步调度的测量边界，避免 requestAction 返回时误把旧 Ready 当成目标动作已显示。

最终 `ctest --test-dir build/Desktop_Qt_6_12_0_MSVC2022_64bit_Release --output-on-failure -j 4`：21/21 通过，115.08 秒，包含 software/offscreen 契约和可选 GPU `mascot_render`。`python tools/check_docs.py`：36 Markdown、503 本地链接通过；OpenSpec 严格验证、clang-format 检查及差异空白检查通过。任务 2.8 完成，总进度 13/40；本轮未重复独立关闭 ThorVG 的构建，未验证其他平台，不同步主规格、不归档。

## 2026-10-28 修正直接运行时的后端预期

用户在 Qt Creator／普通桌面直接运行 tst_mascot 时，未设置 QCAT_MASCOT_GPU，但实际 D3D11 播放成功。此前 lottieRendering 把环境变量缺省误判为后端不可用，因而期待 Error 却得到 Ready。现改为依据实际 backendAvailable 和 graphicsApi 选择成功／失败契约；显式 QCAT_MASCOT_GPU 仍要求原生 QRhi 和 ThorVG，不能以 software 错误分支充当 GPU 通过。可选测试 SKIP 文本说明需显式启用，避免被理解为本机后端不可用。

同一 Windows / Qt kit 下：清除 GPU 和 software/offscreen 变量后直接运行完整 tst_mascot，28 passed / 0 failed / 3 skipped，21.611 秒；跳过项为未启用的可选 GPU 检查。CTest mascot_contracts 的 offscreen/software 契约 1/1 通过，14.21 秒。显式 QCAT_MASCOT_GPU=1 的 lottieRendering 3 passed / 0 failed / 0 skipped（含 init/cleanup）。受影响目标构建、文档检查、clang-format 和差异空白检查通过。

## 2026-10-28 可配置动作容量预算

按用户要求新增任务 2.9，将原固定两个非活动场景／48 MiB 规则替换为公开 `MascotPetController.actionCacheLimitMiB`，默认 150，0 禁用缓存和预热，负值拒绝。配置按实例独立保存，切包不重置宿主配置，可运行中增减；无固定动作数限制。预算按不同来源的动作源文档字节合计，包含活动动作和非活动缓存；活动动作超限仍可播放，但不保留其他场景。保留单文档 64 MiB 校验，不把缓存容量放宽当成单文件限制放宽。

预热优先默认／拖拽／working／jump，然后按来源去重覆盖剩余动作和方向；每实例一次一个推测任务，预算不够则跳过，不为推测预热淘汰现有场景。预热期间通过反馈标志驱动渲染轮询，暂停时钟仍能推进全部预热，完成后停止额外轮询。已尝试的来源避免每帧重复文件检查；预算增大可重试。动态缩小预算在下一次渲染同步淘汰，0 会取消非活动预热，不打断当前播放。已开始的内部解析仍遵守协作取消边界，旧画面和后台任务可暂时并存。

同一 Windows 11 / Qt 6.12.0 MSVC Release kit 下，独立 `tst_mascot configurableActionCache actionCacheAndLatency` 首轮 4 passed / 0 failed / 0 skipped（含 init/cleanup），21.680 秒。默认 150 MiB 时实际 CatGray 十动作全部预热，暂停状态下等待九个后台场景完成，跨十动作固定乱序切换二十次，断言没有淘汰且 CPU 解码计数不变。降为 1 MiB 后缓存淘汰，超限动作仍 Ready 且重新解码；0 禁用后不再命中。整段测试进程观测峰值工作集 571.6 MiB，包含完整校验与临时解析，不是缓存常驻占用，不能把源文档预算理解为实际 RAM／显存上限。临时证据 `build/mascot-budget-gpu.txt`。

最终测试另补充了从 0 恢复到 150 后重新完成九动作预热并命中，以及 QML 属性设置；这些由最终 CTest mascot_render / mascot_contracts 覆盖。公开配置契约验证默认值、实例隔离、重复值不通知、负值原子拒绝、0 和切包保留配置。40 MiB 显式小预算仍运行既有往返延迟及淘汰／reload／销毁测试，不能把不同容量和调度条件下的延迟直接与历史单次测量作提升比较。

完整构建、库及 Gallery qmllint 通过。安装至仓库内临时 `build/mascot-budget-install`，独立消费者仅包含安装公开头并链接安装导出目标，验证默认 150、C++ setter 和 QObject property 写入／读取，构建及运行退出码 0；未使用源码或 Gallery 私有头。公开类新增非虚访问方法、属性与通知信号，配置存于原 Pimpl，不改变公开对象数据布局或虚函数列表；未声称已经完成旧二进制替换矩阵。

首轮全量并行 CTest 为 20/21：mascot_render 61.05 秒通过，mascot_contracts 的 software 分支因完整大包校验超过旧 5 秒上限失败，QtTest 明确报告 5200 ms 即可达到预期 Error；不是取得错误状态或渲染失败。将该实际大包的首次 Ready／Error 等待统一为 30 秒，与其他大包加载检查一致，不改变任何播放逻辑或断言结果。该时限只用于契约测试等待异步完整校验，不是切换性能验收门槛。

最终重新执行完整 `ctest --test-dir build/Desktop_Qt_6_12_0_MSVC2022_64bit_Release --output-on-failure -j 4`：21/21 通过，142.49 秒；mascot_contracts 29 passed / 0 failed / 4 skipped（可选 GPU 检查未启用），mascot_render 7 passed / 0 failed / 0 skipped（含 init/cleanup），61.44 秒。文档检查通过：36 Markdown、503 本地链接、403 个公开 C++ 属性名称覆盖；OpenSpec 严格验证、clang-format、差异空白检查通过。任务 2.9 完成，总进度 14/41。未扩大平台支持声明，未同步主规格或归档。

## 2026-10-29 Windows 桌面窗口前置原型

完成任务 1.2。`tests/manual/mascot-probe` 增加 Windows 交互模式、独立原生背后窗口进程、原生输入检查与 JSON 证据输出。Qt Widgets 仅用于探针的 QApplication、托盘和菜单；没有改库接口或引入库的 Widgets 依赖，没有交付公共 CatMascotPet。

环境：Windows 11 Version 26H2，内核 10.0.26300，Qt 6.12.0 msvc2022_64，MSVC `_MSC_FULL_VER=193933522`，Release/Ninja，实际图形 API Direct3D11。两块屏幕均 DPR 1.0，本次操作限主屏，未测试跨屏及混合 DPI。最终报告时间为 `2026-10-28T16:10:13Z`（客户端北京时间 2026-10-29 00:10:13）。探针 PID 36088、独立背后窗口 PID 32900，进程归属断言通过。

根构建实测命令：`build/mascot-probe-root-check/bin/qcat-mascot-probe.exe --windows-check build/windows-mascot-prototype-final.json`。退出码 0，23 项原型检查全部 Passed。完整平台 acceptance 仍为 Pending；只有已测原型能力标记 Supported，并在原因中注明单主屏原型范围。

| 场景 | 实际结果 |
| --- | --- |
| 被动显示、不抢焦点 | 显示前后前台 HWND 均为独立目标；角色点击和画面计数更新后仍保持目标焦点 |
| 空白与角色区域 | 初始背后点击 1 次，透明空白点击后为 2；角色点击只增加 bodyClicks 到 1，不增加目标点击 |
| 原应用继续输入 | 真实 A 键只增加背后原生 WM_KEYDOWN 计数到 1 |
| 透明合成 | 浮层空白和同帧窗口外背景参考像素一致，角色像素不同；合成截图已查看 |
| 整窗穿透 | 点角色和气泡分别令目标点击增至 3、4，角色／气泡回调不增加 |
| 动态气泡输入区域 | 隐藏气泡后原气泡区域点击令目标增至 5；显示后仍可主动交互 |
| 拖动、外部定位 | 真实按下／移动／释放得到逻辑位移 30×20；读取原生窗口矩形，复位原生坐标为主屏 160×150 |
| 置顶开关 | 原生 WS_EX_TOPMOST 的关闭／开启状态符合请求；本次不覆盖独占全屏 |
| 主动气泡焦点与键盘 | 鼠标点击后获得前台，Tab 到确认按钮、空格激活，bubbleClicks 从 1 到 2 |
| 隐藏与宿主恢复 | 隐藏和恢复可见状态分别通过，不依赖托盘恢复 |
| 托盘与菜单 | 注册及 geometry 非空；展开隐藏区后取得实际橙色图标，收到原生 Context（1），实际 Home／Enter 菜单操作隐藏浮层 |
| 菜单回调语义 | 穿透及退出回调以 QAction 派发验证；退出仅计数请求为 1，宿主仍运行，不称为所有菜单项鼠标实测 |

最终计数：目标鼠标 5、键盘 1；角色按下 2（含拖动）、气泡激活 2；菜单动作 4、退出请求 1。`build/windows-mascot-prototype-final.json`、相邻合成及托盘 PNG 为本机临时证据，不加入源码资源。已查看最终合成图和展开后图标，确认无裁切、气泡按钮和角色区域可辨识。computer-use 另检查交互控制窗口的实际控件，并通过 Close probe 清理测试窗口。

发现并解决的验证问题：沙箱拒绝 SendInput（错误 5），不能作为真实穿透证据；改在获准的交互桌面运行，最终检查使用原生输入。Windows 隐藏托盘图标时 Qt 返回展开箭头位置，原检查右键命中箭头而非图标；现先展开再查询 geometry。混合 Qt Quick／QMenu 原型需在主动托盘交互时明确取得菜单焦点。透明比较改为同帧背景参考，避免色彩管理／桌面过渡影响固定 RGB 断言。气泡使用标准 Tab／空格行为，未把普通 Button 的 Enter 当作必然激活语义。

独立构建 `build/mascot-windows-probe` 通过；根项目另建 `build/mascot-probe-root-check`，仅该临时构建启用手动测试、关闭 ThorVG／Gallery，探针目标构建通过，未改用户原有 kit 缓存。两种构建的分类及 headless 拒绝 CTest 各 2/2 通过，最终分别 0.13 秒和 0.29 秒。headless 检查特意预写旧 Passed 文件，验证 offscreen 检查返回 2 并将旧结果替换为 Pending、能力 Unknown，不制造或沿用实机通过。

新增 Windows QML 的直接 qmllint、C++ clang-format、文档检查和差异空白检查通过；文档检查 37 Markdown / 509 本地链接。仅改手动探针与文档，本轮未重复此前库／Gallery 21 项全套回归。原平台完整矩阵及任务 7.2 继续 Pending；其他系统原型任务不勾选，不同步主规格、不归档。

## 2026-10-29 macOS 桌面窗口原型实施与部分验证

本轮开始任务 1.3，交付 `tests/manual/mascot-probe/macosprobe.h`、`macosprobe.mm`、`MacosProbe.qml` 及 Apple 条件构建。未新增公共库 API，未修改角色渲染／调度实现。任务 1.3 保持未完成，整体仍为 15/41，不同步主规格、不归档。

环境：macOS Tahoe 26.6.2（25G83），Apple arm64，Qt 6.10.2，AppleClang 21.0.0.21000101，Release/Ninja。起点提交 `69dcdf0` 加本轮工作区改动。单屏报告逻辑可用区域 1920×974 左上角 (0,30)，DPR 1；未测试跨屏或混合 DPI。角色／气泡为 AppKit 原生矩形面板，Qt Quick 仅用于诊断界面；没有 Metal／Lottie 像素验收。

实现采用两个 nonactivating NSPanel 分别承载声明的角色与气泡矩形区域，中间空白无窗口；没有实现任意多边形／椭圆区域。角色拒绝成为 key window，气泡提供原生按钮，拖动同步气泡位置；置顶映射到 NSFloatingWindowLevel／NSNormalWindowLevel，整窗穿透同时设置两面板 ignoresMouseEvents。菜单栏 QCat 提供 show、hide、passthrough、exit，退出仅记录宿主请求。独立目标进程通过 stdout JSON 输出鼠标、键盘和焦点状态。菜单栏项目显式持有并在退出时移除，修复首轮清理中的 EXC_BAD_ACCESS，随后检查正常返回 2，交互界面关闭返回 0。

工具：当前 PATH 无 Node/OpenSpec，在 `/tmp` 下载官方 Node 22.23.4（SHA-256 校验一致），从 npm 安装临时 @fission-ai/openspec。执行 list、status、instructions apply，确认 repo-local 根和 ready 状态，未更改系统安装或项目配置。

命令与结果：

- `cmake -S tests/manual/mascot-probe -B build/mascot-macos-probe -G Ninja`，附当前 kit 的 Qt prefix、Ninja 路径和 Release：独立配置／构建通过，生成 qcat-mascot-probe.app。
- `ctest --test-dir build/mascot-macos-probe --output-on-failure`：2/2 通过；最终一次 0.05 秒。覆盖平台分类与离屏拒绝检查，后者覆盖预写旧 Passed 的报告，不发送系统输入。
- Qt 6.10.2 `qmllint tests/manual/mascot-probe/MacosProbe.qml`：通过。使用 Bound 组件行为修复委托中的非限定访问。
- 沙箱内 Cocoa 启动报告 no screens available，不能用于实机验收；获准在真实桌面执行 `--macos-check build/macos-mascot-prototype-final.json` 后能取得屏幕和目标进程记录。
- 最终 Cocoa 记录：8 项 Passed（独立目标进程归属、非激活角色窗口属性、原生置顶、关闭置顶、原生定位、两区域隐藏、宿主恢复、两窗口忽略鼠标属性）；被动显示焦点检查 Pending，因为打包运行中的目标进程没有取得初始 key window。检查先建立基线，未建立时不声称显示导致抢焦点或已验证焦点保持。
- `CGPreflightPostEventAccess()` 为 false；检查在发送事件前保存报告并返回 2。最终鼠标／键盘／角色／气泡计数均为 0；没有把窗口属性检查计为真实点击穿透。整体 prototypeAcceptance 和 acceptance 都为 Pending。
- computer-use 查看诊断窗口及截图，确认操作按钮、说明和滚动 JSON 可读；关闭诊断窗口后主探针退出 0。工具选择应用额外启动的诊断进程及早期未打包交互进程另行清理。未通过 computer-use 验证原生面板输入、菜单栏操作或透明合成。
- OpenSpec strict validate 通过，文档检查与 git diff --check 通过；最终计数见文档验证历史。本轮只改独立手动探针与文档，未重复库／Gallery 全套构建和 CTest。

未覆盖／阻塞：系统事件发送权限不可用；焦点基线未建立。真实区域和整窗点击穿透、实际拖动、原应用继续输入、气泡 Tab／空格、菜单栏实际交互、透明合成像素仍待验证。菜单注册不等于菜单操作实测。未修改辅助功能／录屏权限或自动申请新权限。运行命令和人工检查方法已补入 VALIDATION，当前限制补入 MASCOTPLATFORMS。任务 1.3 及完整 macOS 任务 7.3 均保持未勾选。

## 2026-10-29 macOS 原型真实输入及人工菜单验收完成

继续任务 1.3。用户明确允许仅本次验收启用探针辅助功能权限，并亲自在系统密码框完成验证；通过系统设置选择当前构建。终端直接启动二进制时 postEventAccess 仍为 false，改用 `open -n -W build/mascot-macos-probe/qcat-mascot-probe.app --args --macos-check <absolute-report-path>` 后为 true。开发构建重新链接会使旧授权身份失效，期间在同一已授权范围重新选择当前 app；最终验收后已观察 qcat-mascot-probe 辅助功能开关为 off。未更改其他应用权限，未申请录屏权限。

修正发现的两个问题：穿透／气泡状态变更与真实点击不能在同一回调内立即连续执行，现分为独立检查步骤，等待 WindowServer 生效；主动气泡操作显式激活应用、设置原生按钮响应者，按钮明确支持 Tab／Shift+Tab 和空格，不依赖用户修改系统键盘导航设置。被动角色仍拒绝 key window。报告逐项保存独立进程输入、角色／气泡计数、气泡 keyWindow 与响应者类，并记录两个气泡真实键盘事件。

最终构建仍为 macOS 26.6.2（25G83）arm64 / Qt 6.10.2 / AppleClang 21 Release。角色／气泡以 AppKit 绘制，诊断窗口使用 Qt Quick；图形会话为 Cocoa，未作 Metal／Lottie 渲染测试。最终屏幕布局以原始报告为准，单屏 DPR 1。本轮 Qt GUI 会话无额外平台配套组件。

最终输入证据 `build/macos-mascot-final-acceptance-input-repeat.json`（临时本机产物）：19/19 Passed，postEventAccess=true，focusBaselineEstablished=true，检查覆盖独立进程归属、被动显示焦点、角色非激活属性、置顶开关、原生定位、隐藏／恢复、原生穿透属性、透明空白点击、角色点击与焦点、背后键盘、角色／气泡整窗穿透、气泡点击及 Tab／空格、气泡隐藏后的原区域点击、真实拖动及气泡同步。

| 真实输入场景 | 实际结果 |
| --- | --- |
| 空白点击 | 背后 mouse 从 0 到 1 |
| 角色点击 | bodyClicks 到 1，背后 mouse 保持 1，背后 keyWindow 保持 true |
| 背后键盘 | A 键使背后 keys 到 1 |
| 整窗穿透 | 角色及气泡点击分别使背后 mouse 到 2、3，角色／气泡计数不增加 |
| 主动气泡 | 鼠标触发一次；Tab／空格再触发一次，bubbleClicks=2，bubbleKeyEvents=2，末次 keyCode=49 |
| 隐藏气泡 | 原气泡区域使背后 mouse 到 4 |
| 拖动 | 1 次 dragEvents；角色原点由 (200,220) 到 (230,240)，气泡同步到 (170,440)，均为 AppKit 逻辑坐标 |

较早一次最终复跑为 18/19：原生拖动出现 5 个移动事件，角色到 (230,120)，与预期轨迹不符。不能仅凭日志确定来源；告知用户暂勿操作鼠标／键盘并保持代码不变后复验，最终 19/19 且只有预期的 1 个拖动事件。保留初次报告 `build/macos-mascot-final-acceptance-input.json`，不删除失败记录，也不把它解释成已经证明的产品缺陷或用户干扰。

人工菜单证据 `build/macos-mascot-menu-evidence.json`：电脑工具仅绑定应用窗口，无法直接读取系统菜单栏，向窗口发送按键还会收起系统菜单。用户从真实 QCat 菜单栏执行 hide、show、passthrough 切换及 exit，明确确认隐藏／恢复正确、exit 后宿主仍运行，角色与气泡之间可以看到背后窗口且没有实色大矩形。探针记录 menuActions=9、exitRequests=2、lastMenuAction=hide、末态 panelsVisible=false、passthrough=false；操作次数超过请求的最小五步，按实际计数记录，不推断每一次动作的顺序。诊断窗口 save 成功写入系统临时目录后复制保存此原始记录。视觉结论来自用户实机确认，不称为工具像素对比或录屏证据。

诊断窗口补充 menu 调试入口，展开与菜单栏共用的 NSMenu；异步展开避免阻塞当前 Qt 事件。该入口不替代菜单栏点击证据。菜单回调记录计数与最后动作。交互 save 默认写入 outputPath 显示的系统临时目录，避免 LaunchServices 工作目录不可写。

汇总证据 `build/macos-mascot-prototype-accepted.json` 合并上述自动检查及人工确认，prototypeAcceptance=Passed，顶层 acceptance 继续 Pending；仅在上述系统／Qt 的单屏矩形原型范围将已测能力记录为 Supported。自动输入入口本身仍返回 2 / Pending，等待人工菜单及视觉证据，不自动沿用本次人工结论。任意形状输入区域、角色动作联动、跨屏／混合 DPI、平滑移动、气泡避让、屏幕移除、Spaces／全屏及完整平台矩阵不在本轮通过范围。

最终独立构建通过；CTest 2/2，0.08 秒；MacosProbe qmllint、C++ clang-format 检查通过。探针与独立目标进程已全部退出，系统进程检查未发现残留。文档、OpenSpec 严格验证及差异空白检查结果见本轮最终记录；未重复库／Gallery 全套回归。任务 1.3 勾选完成，进度 16/41；任务 7.3 保持未完成，不同步主规格、不归档。
