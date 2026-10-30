# Validation Record

## 2026-10-29 Ubuntu Wayland 与 xcb 环境及渲染启动检查

用户同意在当前 Ubuntu 会话分别通过 Wayland 和 xcb 验证。本轮为现有诊断探针的环境及真实图形启动检查，不是 Linux 窗口交互原型验收。任务 1.4、1.6 和完整平台矩阵保持 Pending，进度仍为 16/41。

环境：Ubuntu 26.04 LTS、GNOME Shell 50.1、Qt 6.12.0 gcc_64、GCC 15.2.0、Release/Ninja，宿主会话为 Wayland。单屏 HDMI-1、1920×1080、DPR 1。Wayland 报告可用区域 (0,0,1920,1080)，xcb 报告 (67,32,1853,1048)，按各 QPA 的实际报告记录，不推断其为统一的桌面可用区契约。

- 使用当前 Release CMakeCache 中的 Qt prefix，独立配置及构建 `build/mascot-linux-probe`：通过，未修改原有 kit 缓存。
- `ctest --test-dir build/mascot-linux-probe --output-on-failure`：1/1 通过（会话分类），0.04 秒。
- `QT_QPA_PLATFORM=wayland build/mascot-linux-probe/qcat-mascot-probe --json`：退出 0，qpaPlatform=wayland、session=NativeWayland。
- `QT_QPA_PLATFORM=xcb build/mascot-linux-probe/qcat-mascot-probe --json`：退出 0，qpaPlatform=xcb、session=XWayland；不能代替原生 X11 验收。两份 JSON 的全部能力为 Unknown，acceptance=Pending；该命令不初始化图形后端。
- 分别以 `QT_QPA_PLATFORM=wayland` 和 `QT_QPA_PLATFORM=xcb`、`QSG_INFO=1` 启动默认诊断窗口，通过 `timeout 5s` 自动结束。两者均建立 threaded render loop 和 OpenGL QRhi，日志报告 AMD Radeon 780M / Mesa 26.1.6 / OpenGL 4.6，未出现 QML 加载错误。退出 124 为预期的超时终止，不作为正常退出或完整验收通过。Wayland 另有 textinput disableSurface 焦点警告，未验证其交互影响。

沙箱内首次连接 Wayland 被拒绝、xcb 无法连接显示；通过执行审批访问真实桌面后两者成功，不能将首次启动失败解释为平台插件或产品不支持。未安装或启用 GNOME 扩展、未改系统设置。未取得透明合成、区域／整窗穿透、真实输入去向、拖动、定位、置顶、焦点、托盘及配套故障恢复证据；这些仍需后续 Linux 交互原型。没有公共 CatMascotPet 支持声明。

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
| 跳过任务栏、宿主最小化独立与恢复显示 | Pending | Pending | Pending | Pending | Pending |
| 精灵／Lottie 播放、隐藏及减少动效 | Pending | Pending | Pending | Pending | Pending |
| 多实例隔离、关闭隐藏、不退出宿主 | Pending | Pending | Pending | Pending | Pending |
| 平台受限能力失败与宿主恢复 | Pending | Pending | Pending | Pending | Pending |

当前修订以无系统配套依赖的宿主集成为准，平台受限能力须验证明确失败与宿主恢复；历史配套研究不作为交付门槛。完整支持仅针对矩阵通过的明确版本组合，不外推到所有系统版本或所有 Wayland 合成器。

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

## 2026-10-29 Linux 椭圆窗口交互原型与两后端检查

继续任务 1.4 的 Linux 原型，在 `tests/manual/mascot-probe` 增加 linuxprobe、LinuxProbe／LinuxOverlay／LinuxTarget 及 Linux 条件构建。没有修改公共库、Gallery 或角色包播放接口。原型通过 QWindow.setMask 声明椭圆角色与矩形气泡区域的并集，整窗穿透使用 WindowTransparentForInput；xcb 用 Xlib 原生读回位置、焦点和 `_NET_WM_STATE_ABOVE`，通过运行时可选 XTest 发送鼠标／键盘输入。独立 Qt Quick 子进程通过 stdout 上报真实窗口事件计数，父进程检查随机 token、PID、QPA，并在 xcb 下核对窗口 `_NET_WM_PID` 与子进程 PID 一致。

环境沿用本文件 Ubuntu 记录：Ubuntu 26.04 / GNOME Shell 50.1 / Qt 6.12.0 gcc_64 / GCC 15.2.0 / Release/Ninja；宿主 Wayland，单屏 HDMI-1、DPR 1。两个后端的角色实际 graphicsApi=3（OpenGL）。本轮目标是单屏窗口原型，不包括角色动画、完整气泡、多实例或平台完整矩阵。

### 最终自动证据

- `QT_QPA_PLATFORM=xcb build/mascot-linux-probe/qcat-mascot-probe --linux-check build/linux-xcb-prototype-ellipse-final.json`：35 Passed、2 Pending、0 Failed，退出 2（待人工验收）。透明合成和真实托盘菜单保持 Pending。
- `QT_QPA_PLATFORM=wayland build/mascot-linux-probe/qcat-mascot-probe --linux-check build/linux-wayland-prototype-ellipse-final.json`：5 Passed、4 Pending、0 Failed，退出 2。通过项为独立进程、渲染 alpha、托盘服务查询诊断、隐藏与宿主恢复；待验收项为原生输入、定位／置顶、桌面合成、托盘菜单。托盘查询只证明可以读取服务可用状态，不证明实际菜单操作。
- 两个自动命令串行执行并正常清理退出。Wayland 仍有 Qt textinput disableSurface 焦点警告，不把它作为输入已通过或失败的结论。

xcb 最终独立目标 PID 753755，目标鼠标计数 5、键盘计数 1；角色按下 2（含拖动）、气泡激活 2、拖动事件 1。原生位置初始为 (227,192)，真实拖动后 (257,212)，复位后恢复初始值。实际目标坐标和输入均按原始 JSON 保存，不把这一单屏位置当作跨屏坐标契约。

| xcb 场景 | 实际结果 |
| --- | --- |
| 独立进程及原生窗口归属 | 随机 token、PID、QPA 与原生 `_NET_WM_PID` 一致 |
| 被动显示及角色点击 | 背后保持输入焦点，角色按下只增加角色计数 |
| 空白及椭圆外角 | 目标 mouse 分别增加至 1、2；角色计数保持 0 |
| 背后继续输入 | A 键使目标 keys 增至 1 |
| 整窗穿透 | 角色及气泡点击使目标 mouse 增至 3、4，角色／气泡计数不增加 |
| 隐藏气泡区域 | 原气泡位置点击使目标 mouse 增至 5 |
| 气泡焦点及键盘 | 主动点击后取得焦点，Tab／空格使 bubbleClicks 增至 2 |
| 拖动、定位和置顶 | 真实拖动位移 (30,20)，原生位置读回、复位、置顶关闭／开启符合请求 |
| 隐藏与宿主恢复 | 两项通过，恢复不依赖托盘 |

查看了最终角色 PNG：椭圆形体、两按钮及文字可辨识，无裁切；PNG 空白具有 alpha。该 PNG 来自窗口渲染目标，不是桌面合成截图。xcb 的桌面抓图未取得有效独立蓝色背景参考，因此不作透明合成像素通过声明。服务查询报告托盘可用，实际菜单仍需人工检查。两个最终 JSON 的 prototypeAcceptance 和顶层 acceptance 都保留 Pending；仅对取得全部关联证据的 xcb 单屏能力记录 Supported。

原生 Wayland 未实现 Mutter 配套：position、smoothMovement、alwaysOnTop 明确报告 Unavailable，companionConnection=NotImplemented；不以 Qt 内部坐标或请求 flags 宣称成功。输入区域、整窗穿透、焦点及系统拖动保持未实测，未用 XTest 在原生 Wayland 下伪造输入证据。未安装／启用任何桌面扩展，没有修改系统配置。原生 X11 和 KWin 环境本轮未取得；任务 1.4、1.6、7.4、7.6 继续未完成，总进度仍为 16/41，不同步主规格、不归档。

### 发现并修复的问题

首轮 `build/linux-xcb-prototype-initial.json` 有输入检查失败，退出清理还发生崩溃；没有删除失败记录。XWayland 指针移动需在后续事件轮次检查实际位置与命中归属，修正后才派发按下／释放；不向其他窗口补发事件。调试过程中两个后端曾重叠运行，相关 `linux-xcb-prototype-debug.json` 不用作验收，后续全部串行复测。测试基线在本程序的独立目标窗口建立焦点，再显示非激活角色；不声称已验证任意其他应用的激活策略。

Wayland 的 GDB 栈定位退出崩溃为：角色窗口销毁后，QProcess 结束信号触发 update 并访问已释放窗口。现析构先断开子进程信号，销毁角色后清空指针，再回收子进程；最终两个后端均正常返回 2，没有再次崩溃。探针会在结束时释放拖拽按下状态、恢复光标及此前 X11 焦点，并回收目标进程和托盘。

### 构建及静态验证

- 独立 `build/mascot-linux-probe` 配置／构建通过；最终 CTest 2/2 通过（0.06 秒）。新增 headless 检查预写旧 Passed 文件，再验证离屏调用返回 2 并覆盖为 Pending、能力 Unknown。
- 用 `-DCMAKE_DISABLE_FIND_PACKAGE_X11=ON` 配置 `/tmp/qcat-mascot-probe-no-x11`：最终构建及 CTest 2/2 通过（0.07 秒），验证不安装 Xlib 开发包时仍能构建原型。该配置的真实 xcb 输入不视为已验证。
- Qt 6.12.0 qmllint 检查三个新 QML：通过。C++ clang-format、文档检查（38 Markdown / 514 本地链接）、OpenSpec 严格验证及差异空白检查通过。
- 只改独立手动探针及文档，本轮未重复库／Gallery 全套构建、qmllint 和 CTest；不把手动探针检查等同于全库回归。

人工验收准备：以 `QT_QPA_PLATFORM=xcb build/mascot-linux-probe/qcat-mascot-probe --linux-prototype` 打开交互窗口，请用户确认透明空白及真实托盘菜单，并通过控制窗口 Save evidence 保存记录。未收到确认及对应证据前维持 Pending。

## 2026-10-29 人工菜单记录及隐藏诊断窗口后的拖动修复

用户回复已尝试所请求的透明背景和真实托盘菜单操作，报告唯一具体异常为诊断控制窗口隐藏后拖动角色卡顿。保留用户保存的原始记录为 `build/linux-xcb-user-menu-evidence.json`：menuActions=7、exitRequests=2、lastMenuAction=Show、passthroughRequested=false、petVisible=true、bodyClicks=12、dragEvents=1159，记录时间 `2026-10-29T03:42:41Z`。这些是实际计数，不据此推断每次菜单操作顺序，也不把用户仅表示“都试了”扩写成逐项性能验收。完整平台仍 Pending。

源码核查发现，原拖动路径在每次 QML 局部位置变化时查询 QCursor::pos，再刷新诊断数据；位置变化回调与窗口移动耦合，report 属性和控件状态共用 changed 信号，隐藏文本控件也可能重新序列化和布局。未做独立 CPU profiler 归因，不宣称已量化每项开销。修复改为在角色窗口事件过滤器中读取 QMouseEvent.globalPosition，取消 QML onPositionChanged 驱动拖动及逐次系统光标查询；诊断 report 使用单独通知与缓存，显示时最多每 100 ms 更新，隐藏／最小化时停止，重新显示时刷新。保存证据仍采集当前状态，不依赖界面报告缓存。

补充真实输入回归：自动入口从启动时将诊断窗口最小化，在已有输入检查后保持角色按下，按 16 ms 定时请求 40 个连续移动点，再检查原生窗口位移和诊断刷新数。修复版最终 `QT_QPA_PLATFORM=xcb build/mascot-linux-probe/qcat-mascot-probe --linux-check build/linux-xcb-minimized-drag-verified.json`：37 Passed、2 Pending、0 Failed，退出 2。连续拖动检查收到 40/40 个移动事件，最终原生坐标 (267,212)，相对起点位移 (40,20)；拖动期间诊断刷新 0 次，连续移动事件最大间隔 17 ms。该数字是本轮 GUI 输入处理间隔，不是呈现帧率、扫描输出延迟或所有设备的流畅性保证；用户实际手感仍需复验。

保留调试记录：`linux-xcb-drag-fix.json` 首轮多项输入失败；`linux-xcb-minimized-drag-final.json` 有额外目标点击及新回归基线失败，无法仅凭记录确定额外输入来源。后续 `linux-xcb-minimized-drag-repeat.json` 原有 35 项均通过，新回归因复位后 XWayland 实际指针仍在上个拖动终点而拒绝派发按下。改为从已验证的上轮终点开始下一段连续拖动，沿鼠标事件中保存的按下全局位置生成轨迹；未删除实际指针或窗口归属检查，未将拒绝事件记作成功。最终 verified 报告取得完整 40 点与最终原生位移证据。

同一最终源码原生 Wayland `--linux-check build/linux-wayland-drag-fix.json`：5 Passed、4 Pending、0 Failed，退出 2，正常清理；原有 textinput 警告仍存在，真实 Wayland 拖动未验证。独立构建通过，正常配置 CTest 2/2（0.07 秒）、关闭 X11 查找配置最终构建及 CTest 2/2（0.06 秒）通过；三个新 QML 的 qmllint、C++ clang-format 通过。文档及 OpenSpec 最终检查见本轮结果。不改变全库验证范围、平台支持声明或任务完成数。

已重新打开修复版 xcb 交互窗口，请用户比较控制窗口显示和隐藏时的实际拖动并保存证据；收到复验前，不以自动回归代替用户报告问题的最终验收。

### 拖动修复人工复验通过

用户随后明确回复“两种情况下均流畅，已保存”，确认控制窗口正常显示和最小化／隐藏时拖动均流畅。读取对应原始记录并复制为 `build/linux-xcb-drag-user-accepted.json`：探针 PID 857546、记录时间 `2026-10-29T03:54:47Z`、bodyClicks=34、dragEvents=2844、diagnosticsRefreshes=195、menuActions=1、exitRequests=1、lastMenuAction=Exit request、petVisible=true。diagnosticsRefreshes 是整个交互会话的累计值，不代表隐藏期间刷新；隐藏期间为 0 的证据来自前述连续拖动自动回归。

本次隐藏后拖动卡顿修复的人工验收通过，自动证据与用户确认分别保留。该确认不扩大到原生 Wayland 拖动、原生 X11、多屏、完整气泡或平台完整矩阵；任务完成数继续 16/41。最终文档检查、OpenSpec 严格验证及差异空白检查通过。

## 2026-10-29 Ubuntu GCC QUrl 清空赋值编译修复

用户在 Qt 6.12.0 / GCC 15 的 Release 构建中报告 `qcatvectoritem.cpp` 五处 `activeSource = {}` 赋值重载歧义。将这五处改为 `activeSource.clear()`，明确清空 QUrl；不改变缓存失效、保留画面或资源释放的既有行为，不新增公共接口。

验证使用用户现有 `build/Desktop_Qt_6_12_0-Release` CMakeCache，未修改配置：

- `cmake --build build/Desktop_Qt_6_12_0-Release --parallel 4`：完整构建通过，包含库、Gallery、示例及已配置的自动测试目标。沙箱首轮因默认 ccache 目录只读失败，随后仅为命令设置 `CCACHE_DIR=/tmp/qcat-ccache-build-fix` 完成构建；没有修改系统缓存配置。
- 同目录 `QCatUiKit_qmllint`、`qcat-gallery_qmllint`：通过。
- `ctest --test-dir build/Desktop_Qt_6_12_0-Release --output-on-failure -R '^(vector_contracts|mascot_contracts)$'`：2/2 通过，vector 0.23 秒、mascot 12.82 秒，总计 13.05 秒。此次为 software/offscreen 契约测试，不包含 GPU 像素／动作缓存性能或其他平台验证。
- C++ clang-format、文档检查、OpenSpec 严格验证及差异空白检查通过。未重复全部 CTest，不将此修复计为完整任务 7.1 或平台验收完成。

## 2026-10-29 GNOME/Mutter 配套源码、客户端和安装包

继续任务 1.6。交付 `tests/manual/mascot-probe/gnome` 的 Shell 扩展及共享 GJS 服务、协议核心，`mascotcompanion` 私有 Qt DBus 客户端、集成测试及打包目标。所有代码仅进入独立手动探针，没有改公共库依赖／接口或 Gallery。候选范围为 GNOME Shell 50 原生 Wayland，扩展版本 0.1.0、metadata 包版本 1、协议 1；尚未安装／启用到当前桌面，不能据此声明 Mutter 实机操作通过。

协议使用会话总线 `org.qcat.MascotProbe1`。客户端核对服务与 `org.gnome.Shell` 的唯一连接归属，再进行版本握手；服务端从 D-Bus 查询调用者 PID，按 PID、原生 Wayland 类型与精确 UUID 窗口标题选择目标。只允许 snapshot、move、above，不接受请求提供的 PID，不注入输入或修改托盘服务。操作返回原生矩形／置顶读回，客户端验证字段及 PID；已发出请求不等于执行成功。禁用／调用者断开恢复原置顶状态，断开取消未完成请求且重连不重放。平滑移动仍未实现。

环境：Ubuntu 26.04、GNOME Shell 50.1、Qt 6.12.0 gcc_64、GCC 15.2.0、GJS 1.88.0、Node 24.19.0，单屏 DPR 1。参考 GNOME 官方 GJS 扩展／D-Bus 指南与 Mutter Window API；在线 API 页面可能对应更新主版本，真实 Shell 50 兼容性仍须启用扩展验证。

- 独立 `build/mascot-linux-probe` 配置与构建通过，增加同 kit Qt DBus，仅用于手动原型。
- 最终 `ctest --test-dir build/mascot-linux-probe --output-on-failure`：4/4 通过，1.62 秒。包括会话分类、纯协议、私有总线和 headless 旧结果覆盖。私有总线 QtTest 为 9 passed / 0 failed / 0 skipped（含 init/cleanup），1.404 秒；覆盖配套缺失后连接、错误主协议、非 Shell 服务拒绝、无效原生证据拒绝、两实例状态隔离、其他真实 PID 借用 token 被拒绝、断开终态唯一且重连不重放、禁用恢复置顶并重新启用。
- 私有总线运行实际共享 GJS 服务，只有 Meta.Window 替换为模拟对象；调用者 PID 来自真实 D-Bus。它不验证实际 Mutter 窗口方法。协议 Node 测试含 3 项，通过；涉及参数、PID／窗口类型／token 选择、原生读回不符合请求时 applied=false。
- 沙箱初次私有总线测试因禁止绑定 `/tmp` 本地套接字失败；获准运行临时私有 D-Bus 后上述测试通过。没有连接或模拟替换当前桌面的 Shell 服务，没有安装扩展。
- `QT_QPA_PLATFORM=wayland build/mascot-linux-probe/qcat-mascot-probe --linux-check build/linux-wayland-companion-missing.json`：5 Passed、4 Pending、0 Failed，退出 2，正常清理。companion.state=Missing，定位／置顶为 Unavailable，原因指出服务名不存在；渲染和隐藏恢复仍通过。原有 Qt textinput disableSurface 焦点警告保留，真实 Wayland 输入未通过。没有以 xcb 结果替代原生 Wayland。
- 关闭 X11 查找的 `/tmp/qcat-mascot-probe-no-x11` 最终构建通过，分类／headless CTest 2/2 通过。该配置不增加真实图形或 X11 支持结论。
- 三个 Linux QML 的 qmllint、相关 C++ clang-format、JS 语法检查通过。未重复公共库／Gallery 全套构建和 CTest；本轮只改手动原型。
- `cmake --build build/mascot-linux-probe --target mascot_probe_gnome_package` 通过。安装包位于 `build/mascot-linux-probe/gnome/mascot-probe@qcatuikit.local.shell-extension.zip`，检查只含 metadata.json、extension.js、service.mjs、protocol.mjs，且逐字节匹配当前源码。安装包为本机构建产物，不加入源码。

已将协议、版本范围、权限边界、安装／禁用／卸载步骤及故障验收方法写入 MASCOTPLATFORMS 和 VALIDATION。遵循 design.md 的“不自动安装或启用桌面扩展”，本轮停在安装包可审查阶段，待用户选择是否安装／启用再做实机验收；不会自动注销或重启 Shell。未覆盖真实 Meta.Window 操作、原生输入、真实禁用重连、Shell 升级、多屏及完整矩阵。任务 1.6 保持未完成，进度 16/41，不同步主规格、不归档。最终文档检查、OpenSpec 严格校验及差异空白检查通过。

## 2026-10-29 用户授权 GNOME 配套安装，等待新会话发现

用户授权将 GNOME 配套安装到当前用户并启用以继续实机验证；此前明确暂缓 KWin 配套，保留其未完成状态，不作为本轮 GNOME 及后续公共窗口工作的前置阻塞。

安装前 `gnome-extensions info mascot-probe@qcatuikit.local` 返回扩展不存在，用户扩展目录也无该 UUID，未覆盖其他版本。安装包 SHA-256 为 `a36c926682b8bdb66d626c56a09c7fc6e1f86503d82a71206c874ec5f2dee7aa`。执行 `gnome-extensions install build/mascot-linux-probe/gnome/mascot-probe@qcatuikit.local.shell-extension.zip` 退出 0；安装在当前用户 GNOME extensions 目录，metadata.json、extension.js、protocol.mjs、service.mjs 四份文件与仓库源码逐字节一致。

随后执行 `gnome-extensions enable mascot-probe@qcatuikit.local` 返回 2，提示当前 Shell 未发现该扩展。安装成功不等于启用成功，不将这一结果计为 Mutter 窗口操作通过。等待用户保存工作并自行注销、重新登录后继续启用和验证；未自动注销、终止或重启 Shell，未更改其他扩展。此前安装及启用授权对后续步骤继续有效。任务 1.6、平台矩阵均保持 Pending。

### 安装后的再次发现检查

用户请求再次验证后，`gnome-extensions info mascot-probe@qcatuikit.local` 仍返回 2（不存在）。核对当前用户 HOME、Shell 的扩展搜索环境及四份安装文件正常，metadata UUID 与目录一致。Shell 进程 PID 17349 的启动早于本次安装，尚无新的 Shell 会话发现该目录；定向检索日志未发现该扩展的加载错误。没有重复覆盖安装、修改全局扩展设置或强制重启 Shell。继续等待用户保存工作、注销并重新登录后验证，不能以锁屏或重启开发工具代替 Shell 新会话。安装和启用授权保持有效。

## 2026-10-29 重启后 GNOME 扩展加载检查与修复

用户重启后继续 Ubuntu 验证。当前仍为 Ubuntu 26.04 / GNOME Shell 50.1 / 原生 Wayland / Qt 6.12.0，单屏 1920×1080、DPR 1。新 Shell 已发现扩展，初始为 INITIALIZED、Enabled=No；沿用此前授权启用后为 ERROR。会话日志明确报 `Meta.is_wayland_compositor is not a function`，发生在 extension.js 的 enable；此前私有总线测试未加载 Shell 入口，因此未覆盖此问题。

已修复 Shell 入口：保留严格 Shell 50 主版本检查，移除 Shell 50 已删除的 X11/Wayland compositor 检测函数；窗口筛选仍严格要求原生 WAYLAND 类型与调用 PID。Shell 50 已移除 X11 compositor，参考上游 [Mutter 源码 README](https://gitlab.gnome.org/GNOME/mutter/-/blob/50.1/README.md)。

- GNOME 打包目标与 `node --check tests/manual/mascot-probe/gnome/extension.js` 通过。
- `ctest --test-dir build/mascot-linux-probe --output-on-failure`：4/4 通过，1.58 秒；私有总线不代表实际 Shell 加载通过。
- 在原安装启用授权范围内禁用本扩展、更新已安装 extension.js 并重新启用；仍为 ERROR。当前 Shell 已加载旧 JS 模块，磁盘更新及 disable/enable 没有完成重载，需新登录会话验证修复。不自动注销或重启 Shell。
- `QT_QPA_PLATFORM=wayland build/mascot-linux-probe/qcat-mascot-probe --linux-check build/linux-wayland-after-reboot.json`：5 Passed、4 Pending、0 Failed，退出 2。companion=Missing，定位与置顶正确降级 Unavailable，托盘服务可用；Qt textinput disableSurface 警告仍在。该报告证明失败降级，不证明修复后 Mutter 操作通过。

任务 1.6 和完整平台矩阵仍 Pending。下一步由用户保存工作并注销／重新登录（无需重启整机），再检查扩展状态、原生定位／置顶与禁用重连。


## 2026-10-29 重新登录后 GNOME 扩展加载与原生窗口操作

用户确认已登出并指定本 change 继续。沿用此前安装及启用授权，查询当前会话扩展状态：`gnome-extensions info mascot-probe@qcatuikit.local` 返回 Enabled=Yes、State=ACTIVE、Version=1。修复后的入口已在新会话加载；未再次安装、注销或重启 Shell。

运行 `QT_QPA_PLATFORM=wayland build/mascot-linux-probe/qcat-mascot-probe --linux-check build/linux-wayland-after-relogin.json`：10 Passed、3 Pending、0 Failed，退出 2（存在待验收项）。GNOME Shell 50.1，配套 0.1.0、协议 1，连接 Ready。原生 snapshot 返回 nativeWayland=true、调用者 PID 228705；move 实际读回矩形 (160,160,420,320)，置顶关闭／开启分别读回 above=false／true，后续 snapshot 保持同一矩形及 above=true。证据限于本次单屏原型操作，不外推跨屏或公共窗口支持。

真实 Wayland 输入、桌面透明合成和托盘菜单仍为 Pending，Qt textinput disableSurface 警告仍存在。未验证真实扩展禁用／重连、版本不兼容及完整矩阵；任务 1.6 保持未勾选，总进度 16/41。本轮未修改实现，未重复构建或契约测试。


## 2026-10-29 宿主直接集成与公共 CatMascotPet

用户明确桌宠由 Gallery 这类宿主应用直接集成，开机自启、应用启停、系统集成和存储由宿主自行实现，并授权修正规划及实施。已同步 proposal、design、两个 delta specs 和 tasks：不再要求部署或管理 GNOME/KWin 配套。保留已有研究代码及历史证据，本轮未调用扩展安装、启用、禁用或卸载命令；公共库不连接研究配套，不增加 Qt DBus 或 Qt Widgets 依赖。

交付：

- 公共 QML Window CatMascotPet，实例持有 MascotPetController，转发角色、动作、朝向、缩放与交互；默认隐藏，关闭默认隐藏，不主动退出宿主。隐藏冻结角色与气泡时限，销毁回收窗口。
- 私有窗口适配提供逐项能力、屏幕信息、输入范围、整窗穿透、拖拽、定位／可取消移动、按需焦点及状态导出／恢复。原生 Wayland 的精确定位、平滑定位与置顶为 Unavailable；即使研究扩展已启用也不借用它。offscreen/minimal 不假报桌面支持。
- Linux xcb 可选使用 X11，动态检查 Xext 运行库及 Shape 1.1，仅设置 ShapeInput，不用 bounding mask 裁切画面。初次构建未具备 Xext 开发头而正确报告 inputRegions=Unavailable；随后改为不需要 Xext 开发头的可选运行库检测，最终该分支已编译并在 xcb 启动验证。原生 Wayland 使用 Qt surface 输入区域。Windows/macOS 公共区域输入适配仍未完成，明确报告不可用；不把既有原型证据搬作公共组件支持证明。
- 纯文本气泡支持唯一标识、替换／追加／更新／关闭、按钮仅发事件、隐藏冻结和有界滚动。提供边缘方位与窗口原点补偿；原生 Wayland 不能承诺全局锚点补偿，完整气泡矩阵仍 Pending。
- 可选原生托盘默认禁用，由宿主提供图标和附加动作；显示／隐藏／穿透及退出请求不结束宿主。没有原生托盘／菜单或缺少图标时明确诊断。真实菜单及服务消失／恢复仍 Pending。
- Gallery 新增 mascot-pet 页面，复用两份已许可角色包，支持独立实例、动作、气泡、移动、穿透和托盘。新增中英文文案、专题／索引／维护／构建／ABI／技能说明、已安装模块的独立消费者。

验证环境沿用 Ubuntu 26.04、GNOME Shell 50.1、Qt 6.12.0 gcc_64、GCC 15、Release/Ninja，单屏 DPR 1；源码基于 e6f1676 加当前工作区改动。实际构建目录来自当前 CMakeCache：build/Desktop_Qt_6_12_0-Release，未替换 Qt kit。构建命令使用 CCACHE_DIR=/tmp/qcat-ccache-build-fix，避免默认缓存只读；首次未带此变量的 lint 触发重编译失败不算代码检查通过，最终已带变量完成。

最终结果：

- `cmake --build build/Desktop_Qt_6_12_0-Release --parallel 4`：完整构建通过。`QCatUiKit_qmllint`、`qcat-gallery_qmllint` 最终通过。
- `ctest --test-dir build/Desktop_Qt_6_12_0-Release --output-on-failure --timeout 300`：20/20，通过，190.85 秒。Gallery 为 852 passed / 0 failed；mascot 为 31 passed / 0 failed / 5 skipped，跳过项为显式桌面检查和既有可选 GPU 检查，不把 software/offscreen 当作 GPU 或真实输入证据。
- 最后输入适配与布局补偿的小幅修订后，受影响 `mascot_contracts|vector_contracts` 再次 2/2 通过（12.53 秒），最终构建及两项 qmllint 通过。没有重复无关测试。
- 显式 `QCAT_MASCOT_DESKTOP=1`，分别以 wayland 与 xcb 执行 `tst_mascot desktopWindowIntegration`：各 3 passed / 0 failed（含 init/cleanup）。Wayland 正确拒绝定位／移动；xcb 完成移动、无效状态原子拒绝、有效状态恢复、取消终态唯一。气泡窗口保持 320×161；xcb 显示气泡前后导出角色原点均为 (340,320)。截图为窗口渲染，不能证明桌面透明合成、输入去向或屏幕扫描时序。
- `cmake --install ... --prefix /tmp/qcat-mascot-install` 通过；以该独立前缀和同一 Qt 配置／构建 tests/auto/cmake/mascot-consumer，离屏运行通过。消费者仅使用公开头和已安装模块；最终重新安装后同一消费者再次通过。
- Gallery 中文浅色 1200 宽、中文深色 mobile 密度 440 宽截图已查看，文本和按钮可辨、换行正常，窄屏使用页面滚动，没有绑定循环。Gallery 测试覆盖公共窗口控制、独立气泡、按钮只发事件且不自动关闭；窗口契约覆盖关闭隐藏、缺少托盘诊断及销毁后无遗留窗口。
- 文档检查、OpenSpec 严格验证、clang-format 与差异空白检查通过，最终再次运行结果见交付记录。

发现的问题与修复：Qt 6.12 Flow 的 implicitHeight 只读导致初版无法加载，已移除写入；气泡布局与共享通知引发高度绑定循环，已拆分通知并消除布局闭环。qmllint 曾因私有 controller 属性未使用完整命名空间而以 255 退出，补齐类型后通过。Wayland 初版窗口在延迟 configure 后缩回 32×32，已加固定内容尺寸与事件轮次同步，最终真实窗口保持完整尺寸。xcb 气泡多轮布局与原生几何回报导致角色原点偏移，现合并布局期间的角色原点并在 50 ms 收尾时重新补偿，最终原点断言通过；该收尾时间不是无跳动或帧率保证，完整视觉矩阵继续 Pending。首轮全量测试 19/20，失败仅为离屏插件不支持 propagateSizeHints；离屏分支跳过原生尺寸提示后最终 20/20。保留失败日志，不以早期截图证明最终通过。

证据目录：build/mascot-host-integration-20261029，含完整 CTest、最终受影响测试／lint／构建、两个 QPA 的桌面日志、窗口 PNG、Gallery PNG、安装消费者结果与尺寸／锚点修复前日志。

完成任务 4.1、4.6、5.1、5.5、6.1、6.3、7.1，总进度 23/41。4.2–4.5、5.2–5.4、6.2 仍需补齐其完整契约或实机证据；未取得真实点击穿透、焦点／键盘、托盘菜单、混合 DPI／多屏移除与其他系统证据，不勾选相应平台任务。Windows/macOS 的公共输入区域私有适配和托盘运行中故障恢复仍待实现／验证。不自动同步主规格、不归档，也不宣称完整跨平台支持。

最终收尾检查：python tools/check_docs.py 通过（38 Markdown、517 本地链接、123 公开 Cat 类型）；OpenSpec 严格验证通过；clang-format dry-run、git diff --check 及格式化后完整构建通过。已打开原生 Wayland Gallery 的 mascot-pet 页面供用户试用；没有把打开页面计作人工验收。


## 2026-10-29 移除托盘、独立工具窗与播放反馈修正

用户进一步明确组件不应提供托盘，要求默认置顶、默认鼠标不穿透、桌宠不单独进入任务栏且不随主程序最小化；并报告 Lottie 切换加载圈，以及 PNG 正常播放抖动（已澄清不是只在缩放或拖动时出现）。本轮修订取代此前公共组件的托盘接口和验收要求；独立研究探针及历史结果保留。

- 删除 CatMascotPet、私有窗口适配和 Gallery 的全部托盘属性、菜单、图标、退出请求与按钮。托盘、开机自启、应用启停仍完全由宿主实现。无新增系统依赖或扩展操作。
- 默认 alwaysOnTop=true、inputTransparent=false。新增 inputRegionsEnabled=false，默认整个窗口接收鼠标；声明区域外穿透须显式启用。整窗穿透仍可由宿主开启并恢复。状态 v1 增加可选 inputRegionsEnabled，旧快照按 false 恢复。
- QML 显式 transientParent=null，私有适配防止原生 transient 关系重新绑定，保留 QObject 所有权供宿主销毁。xcb 为自身窗口设置 EWMH SKIP_TASKBAR／SKIP_PAGER；默认请求置顶。原生 GNOME Wayland 仍明确反馈置顶／任务栏策略限制，不依赖系统扩展。Gallery 仅在 Linux Wayland 会话、有 DISPLAY 且未显式选择 QPA 时默认用 xcb；显式 QT_QPA_PLATFORM 或 -platform 仍由用户控制。库本身不替宿主选择 QPA。
- 加载圈来自 CatLottieAnimation 的通用 Loading 提示，不是缓存失效的证明。桌宠私有视觉改用相同 VectorItem 渲染／缓存入口，保留旧画面至首帧接管，不再创建 CatBusyIndicator；Loading／错误反馈、取消和缓存预算保持有效。普通 CatLottieAnimation 不改。
- PNG 使用 Qt AnimatedSprite 的单帧采样，暂停其自身动画，由控制器唯一时钟设置当前帧；不再移动整张图集并依靠裁切选择帧。帧间混合关闭，公共桌宠默认 smooth=true，像素画仍可设 false。未改源图集或包内帧率，CatGray PNG 样例仍为 8 fps。空间采样与窗口位置稳定性已验证，原素材低帧率及主观播放观感仍需用户复验，不把测试通过扩写为所有动画和设备流畅。

环境仍为 Ubuntu 26.04 / GNOME Shell 50.1 / Qt 6.12.0 / GCC 15 / Release/Ninja，当前真实桌面检查为 Wayland 会话中的 xcb/XWayland，不代替原生 X11。使用当前 build/Desktop_Qt_6_12_0-Release 配置，构建时保留临时 ccache 路径。

验证：

- 完整构建、QCatUiKit_qmllint 和 qcat-gallery_qmllint 通过。
- 完整 CTest 20/20，通过，193.77 秒；Gallery 为 852 passed / 0 failed；mascot 为 32 passed / 0 failed / 5 skipped。跳过项仍是独立显式桌面入口及既有可选 GPU 场景。
- `QT_QPA_PLATFORM=xcb QSG_RHI_BACKEND=opengl QCAT_MASCOT_GPU=1 tst_mascot framesRendered spriteSamplingStability lottieRendering actionCacheAndLatency`：6 passed / 0 failed / 0 skipped（含 init/cleanup），20.486 秒。缓存 resident 20 次反馈 P50=34 ms、P95=50 ms，prewarm=38 ms，冷包=3237 ms；这些是本机首帧反馈测量，不是屏幕输出延迟或通用保证。缓存命中、淘汰与限额关闭仍通过。
- 显式 `QCAT_MASCOT_DESKTOP=1`，同一 xcb/OpenGL 下执行 desktopWindowIntegration、framesRendered、spriteSamplingStability：5 passed / 0 failed / 0 skipped，3.614 秒。xprop 实际读回 SKIP_TASKBAR、SKIP_PAGER、ABOVE。故意设置 transient parent 后应仍为空；最小化独立宿主后桌宠保持显示、非最小化。连续 PNG 帧期间窗口位置与尺寸不变，气泡前后角色原点均为 (340,320)，状态恢复和移动取消仍通过。测试同时观察到 DEMANDS_ATTENTION 标志，不将此结果写成完整焦点验收；任务 4.5 继续 Pending。
- 新像素回归在 0.35、0.5、1、1.25 倍下遍历四帧内部像素，软件和真实 GPU 均通过，覆盖邻帧混入与透明接缝。首版测试在 resize 尚未呈现时立即离屏抓取，取得透明旧帧；现等待 frameSwapped 再抓取，保留完整像素断言，没有把该测试时序问题称作产品修复证据。原有图集顺序、镜像、暂停、隐藏、减少动效检查继续通过。
- 重新安装到 /tmp/qcat-mascot-install，原独立公开头／模块消费者运行通过；无需 Gallery 或平台私有头。文档、OpenSpec 严格验证、差异空白与新增／变更代码格式检查通过。Gallery main.cpp 只保留平台选择块的实际改动，没有整文件格式化噪声。

证据位于 build/mascot-ui-feedback-20261029。已打开修复后的 Gallery mascot-pet 页面供用户复验，启动日志未见 QML 错误。实际桌面面板图标表现与 PNG 播放手感尚未收到用户确认，不以打开页面代替人工验收。完成修订后的任务 5.3，当前 24/41；真实输入去向、完整焦点／键盘、多屏、其他平台与整体矩阵仍保留待办，不同步主规格、不归档。

### 2026-10-29 PNG 旧版包无法拖动修复

根因：Gallery 旧版 sprite manifest 未声明 regions，兼容转换后没有 draggable 命中区域，窗口按下事件未进入拖动。兼容转换现在仅在旧版包缺少 regions 时生成全角色 body；保留显式区域语义。新增 legacySpriteMouseDrag，直接加载 Gallery 同源 PNG 包并发送按下、移动、松开事件。离屏定向测试 5/5 通过；真实 xcb 定向测试 4/4 通过，窗口位移 (40, 30)，含宿主最小化独立性及任务栏策略回归。桌面证据：`/tmp/qcat-mascot-drag-desktop.log`。该自动测试使用合成 Qt 鼠标事件，真实手动拖动仍需复测。

补充验证：mascot_contracts 整组通过（13.96 秒，`/tmp/qcat-mascot-drag-contracts.log`）；文档检查与 OpenSpec strict 校验通过。已重新启动修复版 Gallery。

### 2026-10-29 公共组件输入、移动打断与边缘气泡

用户确认旧版 PNG 包鼠标拖动人工复测通过。本轮完成任务 4.3；4.2 的 Windows/macOS 输入区域适配和 5.2 的窄屏完整矩阵仍待完成。

修正两项行为：拖动期间拒绝宿主 setPosition/moveTo（InteractionBusy，分别返回 false／Failed），松开后不重放；从自动移动进入拖动时以当前窗口位置接续，避免跳回按下位置。先建立拖动状态再取消轨迹，取消回调重入也不能启动竞争轨迹。气泡按角色两侧的真实剩余空间选择方位并限制高度；私有适配提供布局期间稳定的角色原点，异步通知位置，消除水平／垂直绑定循环。

真实桌面命令：设置 QT_QPA_PLATFORM=xcb、QSG_RHI_BACKEND=opengl、QCAT_MASCOT_DESKTOP=1，运行 tst_mascot desktopInputRouting desktopMovementInterruptions desktopBubbleEdges legacySpriteMouseDrag desktopWindowIntegration。Qt 6.12.0，Ubuntu 26.04，当前 GNOME Wayland 会话的 xcb/XWayland 路径；结果 7 Passed、0 Failed、0 Skipped，15.266 秒。

- desktopInputRouting：XTest 真实输入，背后原生窗口计数验证默认全窗阻挡、区域外穿透、镜像、缩放、动作矩形／椭圆区域、整窗穿透恢复、气泡操作按钮和关闭后的范围。
- desktopMovementInterruptions：新移动、定位、重复取消、隐藏、切包、真实拖动取消旧移动；每个请求终态唯一。拖动期间位置请求被拒绝，松开后位置保持；隐藏结束拖动。屏幕变化仅注入 screenChanged 通知，不证明实际拔屏。测试保留背后宿主窗口并核对原生鼠标归属，避免 XWayland 根窗口上的合成鼠标投放失败；早期无宿主测试失败保留在 /tmp 诊断日志，不计作产品验收通过。
- desktopBubbleEdges：明暗主题、四个屏幕角、长文字与操作按钮，气泡显示／关闭前后角色原点一致，完整窗口在可用区内，无绑定循环。八张原生截图保存，人工查看浅色顶部与深色底部截图，文字换行、按钮与角色显示正确。
- 现有桌面回归：任务栏跳过、置顶、宿主最小化独立、气泡窗口补偿及旧版 PNG 鼠标事件回归通过。

构建与库／Gallery qmllint 通过；全量 CTest 20/20 通过（200.88 秒），最终测试源码修改后 mascot_contracts 再次通过。文档与 OpenSpec 严格检查另随记录执行。构建期间最后修正位置通知与水平绑定后，真实桌面全部场景已在最终库重跑；全量测试不替代原生输入证据。

证据目录：build/mascot-input-movement-20261029，含 desktop.log、ctest.log、mascot-contracts.log、qmllint.log、build.log、test-build.log 和八张明暗主题截图。不外推原生 X11、原生 Wayland、Windows/macOS、多屏／混合 DPI、窄屏或焦点键盘矩阵；对应任务保持未完成。

### 2026-10-29 Windows 多实例状态与平台文档收尾

完成任务 5.4、1.7，当前 27/41；不归档、不同步主规格。沿用已有业务实现，本轮新增契约测试并整理文档，未新增平台支持声明。

环境：Windows 11，Qt 6.12.0 MSVC2022 64-bit Release；构建目录来自当前 CMakeCache：build/Desktop_Qt_6_12_0_MSVC2022_64bit_Release。状态定向检查使用 windows QPA 与 software Quick 后端，不代表 D3D 渲染、桌面合成、真实输入或多屏验收。

- 新增 independentWindowState：两个公共 CatMascotPet 的控制器、无限动作、排队请求与气泡隔离；其中一个切包及销毁，另一个保持动作、队列、气泡不变。
- 校验导出 v1 的八个配置字段；无效类型、版本、非有限坐标／缩放、缺失必填字段及包不支持的朝向被拒绝，配置快照不变；拒绝不清空现有队列或气泡。
- windows QPA 下实际执行失效 screenId 回退、恢复缩放与整窗穿透，旧 v1 缺少 inputRegionsEnabled 时恢复 false。3 Passed、0 Failed、0 Skipped（含 init/cleanup），470 ms。此项验证不存在的标识，不代替真实拔屏。
- 源码检查 mascot 模块及 CatMascotPet 未使用 QSettings、QSaveFile、WriteOnly 或 Settings，未引入自动持久化；角色源、动作队列和气泡不在快照中。
- MASCOTPLATFORMS 修正公共窗口尚未交付等过时说明，汇总 Windows/Cocoa、xcb、原生 Wayland 的公共机制、协议、依赖、能力状态与限制。研究配套与公共组件边界保持明确。可重复测试步骤写入 VALIDATION。

执行及结果：

1. cmake --build build/Desktop_Qt_6_12_0_MSVC2022_64bit_Release --target tst_mascot QCatUiKit_qmllint qcat-gallery_qmllint --parallel 4：通过；最终测试增补后再次构建 tst_mascot 通过。
2. windows QPA 运行 bin/tst_mascot.exe independentWindowState：通过，证据 build/mascot-state-test.txt。
3. ctest --test-dir build/Desktop_Qt_6_12_0_MSVC2022_64bit_Release -R '^mascot_contracts$' --output-on-failure：整组通过，16.76 秒，证据 build/mascot-state-contracts.log。
4. Gallery 的 gallery_smoke、gallery_contracts、gallery_chart_data、gallery_data_demo 四项通过，证据 build/mascot-state-ctest.log。组合运行中的 mascot_contracts 因另一个同名测试同时写同一结果文件而失败，属于本轮重复启动导致的文件锁冲突；以上第 3 项串行重跑通过，不将该次组合报告称作全绿。
5. python tools/check_docs.py：通过，38 Markdown、517 本地链接、123 公共 Cat 类型。openspec.cmd validate add-mascot-pet --strict 与 git diff --check：通过。

剩余 14 项继续按任务跟踪：Windows/macOS 公共区域适配、焦点键盘、窄屏气泡、多屏／混合 DPI 及各平台完整矩阵未在本轮验收；1.7 的文档汇总完成不表示这些平台已通过。没有使用研究配套绕过公共组件限制。

## 2026-10-29 macOS 公共气泡键盘生命周期与 Gallery 可访问性

从当前最新提交 `802a165` 继续实施，起始 27/41。仓库已采用宿主直接集成及宿主自有托盘契约，本轮未恢复旧配套／托盘实现。当前平台为 macOS Tahoe 26.6.2（25G83）arm64，Qt 6.10.2、AppleClang、Release/Ninja；从已有 CMakeCache 选择 `build/Qt_6_10_2_for_macOS-Release`，保留 Qt kit 和既有架构。公共 macOS 区域外穿透仍缺少不裁切画面的适配；不以旧矩形探针或 Qt layer mask 代替其完整实现，4.2 不勾选。

本轮完成 6.2，并修复 4.5 的公共气泡焦点生命周期：

- 操作按钮点击完成后按需申请窗口焦点并定位按钮焦点，避免在鼠标按下与释放之间改变窗口标志。关闭按钮直接关闭气泡。窗口失活、隐藏、气泡消失或整窗穿透时恢复 WindowDoesNotAcceptFocus；隐藏／无气泡／整窗穿透时拒绝内部焦点申请。未新增公开头、属性或 ABI 接口。
- 气泡关闭按钮、文本滚动区及动作按钮建立明确 Tab／Shift+Tab 顺序，动作可用空格触发，不依赖 macOS 系统键盘导航偏好，也不更改用户系统设置；消息文本提供可访问名称。
- Gallery 将数字状态、布尔穿透值、能力标签及能力状态改为中英可读文案；动作按钮提供本地化的“播放动作”可访问名称，保留包动作 ID。已知能力原因本地化，未知原因保留原始文本。两份 TS 同步新增 20 条文案。
- 新回归覆盖关闭气泡、隐藏／恢复、整窗穿透后的被动策略；Gallery 六组数据覆盖明暗主题、Desktop／Touch／Mobile、680／400 逻辑像素宽度、中英运行时切换、键盘显示窗口、状态／能力／动作可访问文本和独立实例生命周期。

验证：

- 完整构建通过；最终库 `QCatUiKit_qmllint` 与 Gallery `qcat-gallery_qmllint` 通过。未更换 Qt／构建目录。
- 最终 `ctest --test-dir build/Qt_6_10_2_for_macOS-Release --output-on-failure -j 2`：20/20，通过，88.72 秒。早期完整检查亦为 20/20（86.38 秒）；后续 QML 焦点修订后执行了上述最终全量检查。
- `QT_QPA_PLATFORM=offscreen QT_QUICK_BACKEND=software .../tst_mascot publicBubbleKeyboardLifecycle` 通过；该测试使用内置小包，验证鼠标动作、空格、Shift+Tab 至关闭及空格关闭，再验证隐藏／穿透后的被动标志。
- `env -u QT_QUICK_BACKEND QT_QPA_PLATFORM=cocoa .../tst_mascot publicBubbleKeyboardLifecycle`：最终 3 passed / 0 failed（含 init/cleanup），525 ms。该行在真实 Cocoa 图形会话运行，QtTest 向本测试窗口投递事件，不注入其他应用，不申请辅助功能权限；未将其当作独立背后应用的焦点保持或 Metal／Lottie GPU 证据。
- 初期 Cocoa 检查动作计数为 0：测试在新气泡布局尚未稳定时立即取按钮坐标；加入布局稳定等待并放在屏幕内部后，点击和空格路径通过。不能仅凭该失败断言真实用户首击必然丢失。随后复现 macOS 默认 Tab 导航不能到达关闭按钮；明确 KeyNavigation 焦点链后最终通过，没有改变系统偏好。测试按稳定 objectName 及可观察信号／窗口标志验证，不依赖私有对象层级。
- Gallery 专项 `.../tst_gallery -import <build>/qml GalleryMascot::test_accessibleStatusAndKeyboard GalleryMascot::test_hostControlsIndependentWindows`：9 passed / 0 failed（含 init/cleanup），1.264 秒，最终全量检查再次覆盖。software 对 Gallery 大图集显示既有 2048 纹理尺寸警告，该路径不作为角色 PNG/GPU 视觉验收；控件、文案、布局及键盘断言通过。
- 已查看 Gallery 桌宠页英文亮色桌面 1200 宽、中文深色 Mobile 440 宽截图，按钮正常换行，窄屏页面可滚动；最终英文截图能力诊断使用可读标签，状态显示 Ready / Off。临时图 `/tmp/qcat-mascot-final-light.png`、`/tmp/qcat-mascot-dark-mobile.png` 不作为源码资源。未以控件页截图推导桌面透明合成或角色动画正确性。
- 文档检查通过：38 Markdown／518 本地链接；OpenSpec 严格验证和差异空白检查在最终执行。可重复步骤已写入 VALIDATION，焦点用法写入 MASCOTPET；本轮无需安装消费者验证，未改变公共 C++ API 或安装规则。

任务 6.2 完成，总进度 28/41。4.5 仍需背后原生应用验证显示／更新不抢输入及其他平台实机结果；4.2 的 Windows/macOS 区域输入、4.4 多屏、5.2 完整气泡矩阵和全部平台矩阵保留待办。当前没有 Linux／Wayland 实机会话，不以本机结果代替它们；不宣告完整平台支持，不同步主规格、不归档。本轮未自动提交。

## 2026-10-29 Windows 屏幕坐标与边界续作（任务 4.4）

起始及当前进度 28/41，继续保留已有 macOS 的 6.2 与焦点生命周期改动。本轮完成 4.4 的代码修正与自动回归，真实多屏／混合 DPI／拔屏矩阵未完成，因此 4.4 不勾选；其他平台支持状态不变，不同步主规格、不归档。

修正：边界基于角色逻辑尺寸与 characterOffset 计算，而非包含气泡的整个窗口。窗口原点减去气泡偏移后仍可到达角色可用区四边，显示／关闭气泡不缩小定位范围。提取私有逻辑坐标函数，非整数尺寸向上取整，超大尺寸先限于可用区再转换整数；不把 DPR 乘入 Qt 已提供的逻辑坐标。该头仅为私有源码，不增加公开 API 或 ABI。

已有及新接入屏幕统一连接可用区、几何和逻辑 DPI 变化，窗口 DPR 变化亦会取消旧移动并触发校正；屏幕回退不再选择已不在当前屏幕列表中的旧对象。重新开启 constrainToAvailableGeometry 时重新计算缩放及边界。异步布局补偿保留更新后的角色锚点，避免旧布局计时器撤销边界／缩放校正。

环境：Windows 11、Qt 6.12.0 MSVC2022 64-bit Release，构建目录取自当前 CMakeCache：build/Desktop_Qt_6_12_0_MSVC2022_64bit_Release。定向窗口检查使用 windows QPA、software Quick 后端；不视作 D3D／Lottie GPU 或真实输入路由验收。

验证与证据：

- 完整 cmake --build <build> --parallel 4 通过；库 QCatUiKit_qmllint、Gallery qcat-gallery_qmllint 通过。最终测试格式整理后再次构建 tst_mascot 通过。日志 build/mascot-geometry-full-build.log、mascot-geometry-build-final.log、mascot-geometry-test-build.log。
- windows QPA：tst_mascot screenCoordinateRules desktopScreenCoordinates independentWindowState publicBubbleKeyboardLifecycle，6 Passed、0 Failed、0 Skipped（含 init/cleanup），2.501 秒。日志 build/mascot-geometry-native.txt。
- screenCoordinateRules：负原点、窄屏、非整数角色尺寸、气泡偏移、超大角色及多个角色缩放值；这些是合成逻辑几何，不是多屏硬件证据。
- desktopScreenCoordinates：公共窗口带气泡定位至可用区两端、关闭气泡后角色位置不变、注入 DPR 变化事件取消旧移动、重新开启约束后缩放与位置校正。未修改系统显示配置，未注入其他应用输入。事件注入不能代替实际 DPI 切换或拔屏。
- 查看 build/mascot-geometry-bubble.png：右下角长文本气泡换行与关闭按钮完整，角色小型测试帧位于气泡下方。PNG 仅是窗口自身渲染，不是桌面透明合成或原生点击去向证据。
- 离屏 mascot_contracts 整组通过，19.57 秒，日志 build/mascot-geometry-contracts.log。
- 文档与 OpenSpec 严格验证通过；可重复命令和未覆盖内容写入 VALIDATION，坐标规则同步 MASCOTPET。完整 CTest 结果见下方补充。

Windows/macOS 公共区域外穿透仍缺少保持完整画面的私有适配；本轮没有用裁切窗口形状替代该契约。其余平台原型、真实跨屏拖动、混合 DPI、拔屏、窄屏完整输入矩阵与跨应用焦点保持仍为待验收。

完整检查补充：最终 `ctest --test-dir build/Desktop_Qt_6_12_0_MSVC2022_64bit_Release --output-on-failure` 为 20/20 通过，197.71 秒；含 Gallery 合约及最终 mascot_contracts（16.40 秒）。证据 build/mascot-geometry-ctest.log。最终文档检查、OpenSpec 严格验证及差异空白检查通过。完整自动检查不替代前述多屏实机缺口，任务进度保持 28/41。

### 2026-10-29 任务 4.4 用户人工验收确认

在对话中明确列出剩余人工项目（真实跨屏拖动及目标屏幕定位、不同 DPI 屏幕间显示与输入区域对齐、移除当前屏幕后的位置回退和缩放校正）后，用户反馈：“我测试了，没问题！”。据此记录任务 4.4 人工验收通过，结合本节既有坐标自动测试、Windows 定向窗口检查及全量 CTest，勾选 4.4，总进度 29/41。

证据类型为用户人工测试确认，不将其表述为本代理执行的系统输入测试。用户未另附显示器型号、逐屏 DPI、操作录像或新增系统／图形后端版本，故不补造这些细节；已有自动检查环境仍为前述 Windows 11／Qt 6.12.0。该确认用于本轮 4.4 验收，不扩展为其他平台或 Windows 全部功能矩阵通过；4.2、4.5、5.2 及 7.2—7.6 等未完成任务保持原状态。前文“4.4 尚未验收”的记录保留为本次确认前的历史状态。

### 2026-10-29 Gallery 关闭后桌宠残留修复

用户报告打开桌宠后关闭 qcat-gallery，桌宠没有退出。根因是 Gallery 未明确绑定主窗口与进程退出策略：独立桌宠窗口可阻止最后窗口自动退出；直接调用 QGuiApplication::quit() 也会尝试关闭所有窗口，而桌宠 closeHides=true 会拒绝关闭事件并隐藏，从而取消退出。预览的 Component.onDestruction 尚未执行，因为事件循环仍在运行；该现象本身不作为堆内存泄漏结论。

修复仅在 Gallery 宿主：主窗口变为不可见时调用 QCoreApplication::exit(0) 结束事件循环，随后已有栈上 QQmlApplicationEngine 析构销毁所属窗口和控制器；最小化仍保持 visible，不触发退出。不改变公共 CatMascotPet 的关闭隐藏或宿主最小化独立性，也不让库负责退出进程。

新增 gallery_mascot_shutdown 进程级 CTest，复用 --smoke-test --page mascot-pet：必须找到并显示两个真实公共桌宠，最小化主窗口后确认两个实例仍可见，再关闭主窗口。超时 30 秒；实例缺失或提前消失返回 3。普通 gallery_smoke 同样改为走主窗口关闭路径，截图模式保留原有行为。

Windows 11／Qt 6.12.0 MSVC2022 64-bit Release，在当前 build/Desktop_Qt_6_12_0_MSVC2022_64bit_Release 构建 qcat-gallery、Gallery qmllint 通过。首版仅调用 app.quit 的新回归超时，证实退出被桌宠关闭策略阻挡；Windows 原生试运行同样残留，该本轮测试进程已清理。改为事件循环退出后最终结果：

- ctest --test-dir <build> -R '^(gallery_smoke|gallery_mascot_shutdown)$' --output-on-failure：2/2 通过，2.14 秒，日志 build/mascot-shutdown-tests-final.log。
- windows QPA、software Quick 后端运行 qcat-gallery.exe --smoke-test --page mascot-pet：退出码 0；包含最小化独立性检查，日志 build/mascot-shutdown-native-final.log。
- mascot_contracts 整组通过，16.52 秒，见 build/mascot-shutdown-tests.log；该组合中的首版退出测试超时保留为修复前证据，不称为全绿。后续仅修改 Gallery main.cpp 的退出／测试路径，未再次更改库。
- python tools/check_docs.py、OpenSpec strict 与 git diff --check 最终通过；重复执行入口写入 VALIDATION，Gallery README 说明主窗口关闭与最小化的差异。

本修复补强既有 Gallery／宿主生命周期交付，任务进度保持 29/41，不将本次退出测试代替平台完整矩阵，不归档。

### 2026-10-29 Windows 公共区域输入（任务 4.2 部分）

本轮接入 Windows 私有输入窗口，完成 4.2 的 Windows 实现及独立进程真实输入回归。macOS 的非裁切区域输入适配与实机证据仍缺失，因此 4.2 不勾选，当前仍为 29/41；不把本轮结果扩展为完整跨平台或 Windows 全矩阵支持。

实现：QCatUiKit 视觉窗口在 Windows 整窗穿透，另以 WS_EX_NOREDIRECTIONBITMAP／NOACTIVATE／TOOLWINDOW 创建同实例 owned 原生窗口。它不绘制、不进入任务栏，形状来自声明区域及气泡矩形；默认关闭区域模式时承接整个窗口。只对输入窗口设置区域，视觉窗口不裁切；没有透明度扫描、全局钩子或光标轮询。区域外点击由系统直接交给背后应用。输入窗口将自己收到的真实 Win32 鼠标／滚轮消息转换为同实例 Qt 事件，按当前 DPR 转换坐标；捕获支持连续拖动，捕获丢失／隐藏时释放交互，销毁实例回收原生窗口。原生请求失败报告 InputRegionUnavailable 并将能力设为不可用。不新增公开头、公开 API、第三方库或系统服务；user32／gdi32 为系统依赖。

Gallery 新增“区域输入”开关，按会话能力启用；复用已有中英文翻译，离屏不可用时禁用。Gallery 回归覆盖开关可访问名称、能力反馈和 checked 绑定；已查看英文亮色桌面与 440 宽深色 Mobile 截图，控件正常排列、窄屏可滚动。首轮离屏截图没有指定字体目录而出现缺字方块，设置 QT_QPA_FONTDIR=C:/Windows/Fonts 后重新生成并查看；未将缺字体截图作为视觉验收。截图为 build/mascot-regions-gallery-light.png、mascot-regions-gallery-narrow.png，仅为控件布局，不作为桌面输入证据。

原生环境：Windows 11、Qt 6.12.0 MSVC2022 64-bit Release；当前构建目录 build/Desktop_Qt_6_12_0_MSVC2022_64bit_Release，取自已有 CMakeCache。以 QCAT_MASCOT_DESKTOP=1、QT_QPA_PLATFORM=windows 运行 tst_mascot windowsInputRouting：

- QT_QUICK_BACKEND=software、QT_SCALE_FACTOR=1：3 Passed／0 Failed／0 Skipped（含 init/cleanup），9.219 秒，实际 graphics API=1（Software）。证据 build/mascot-windows-input-native.txt。
- software、QT_SCALE_FACTOR=1.25：3 Passed／0 Failed／0 Skipped，9.245 秒，DPR=1.25、API=1。证据 build/mascot-windows-input-scaled.txt。Qt 缩放模拟不代替混合 DPI 硬件。
- 移除 QT_QUICK_BACKEND、QSG_RHI_BACKEND=d3d11、缩放 1：3 Passed／0 Failed／0 Skipped，9.429 秒，实际 API=4（Direct3D11）。证据 build/mascot-windows-input-d3d11.txt；不据此声明其他 GPU／驱动或 Lottie 动作缓存全部验收。

该回归启动独立 Win32 背后进程，使用窗口属性记录真实点击计数，与桌宠区域及气泡信号交叉核对。覆盖默认全窗阻挡、矩形／椭圆／多边形、镜像、缩放、动作覆盖、整窗穿透恢复、区域模式关闭恢复全窗阻挡、气泡点击及真实空格、角色点击及动作更新保持背后焦点、六点连续拖动与最终原生位置、隐藏后点击透过、销毁后本线程原生输入窗口数量恢复。屏幕抽样颜色与 Qt 画面参考一致，区域切换后颜色不变，证明测试点的视觉不被裁切或遮盖。

测试只向本轮桌宠和独立目标进程 SendInput，按下前核对窗口进程归属；结束恢复光标与此前前台窗口、清理子进程。当前桌面有其他置顶应用，受控背后窗口需临时置顶再把本轮桌宠置于其上，并在鼠标定位后使用原生 SetWindowPos 同步层级；没有修改其他应用层级。开发期曾因置顶目标激活后覆盖桌宠而出现计数失败，不能作为区域实现已通过的证据；修正测试层级后最终三条路径均通过。直接向透明视觉 HWND SendMessage 的早期版本不能稳定产生 Qt 控件事件，已改为原生输入窗口持有捕获并转换同实例 Qt 事件，最终操作信号、键盘和拖动均经过上述真实输入验证。

构建及接入：完整构建与库／Gallery qmllint 通过，日志 build/mascot-windows-input-full-build-final.log、mascot-windows-input-qmllint.log。安装到 build/mascot-input-install，再从该前缀配置／构建 tests/auto/cmake/mascot-consumer（QCatUiKit_DIR 明确指向该安装），离屏运行退出码 0；只包含公开头与安装模块。安装／消费者证据 build/mascot-windows-input-install.log、mascot-windows-input-consumer.log、mascot-windows-input-consumer-run.log。新增输入实现的私有头不作为消费者接口。

已有全量 CTest 为 21/21（207.11 秒）；加入 Gallery 区域开关和连续拖动／模式恢复断言后，又执行最终全量 CTest，结果在下方补充。重复验证步骤写入 VALIDATION，平台机制及边界同步 MASCOTPET、MASCOTPLATFORMS、MAINTENANCE 和 design；最终文档检查与 OpenSpec strict 随补充执行。

最终补充：最终代码及 Gallery 区域开关下，`ctest --test-dir build/Desktop_Qt_6_12_0_MSVC2022_64bit_Release --output-on-failure -j 2` 为 21/21 通过，135.69 秒，证据 build/mascot-windows-input-ctest-final.log。Windows 原生 D3D11 路径运行 `qcat-gallery --smoke-test --page mascot-pet` 退出码 0，验证两个桌宠可见、主窗口最小化保持实例可见、关闭主窗口退出的既有修复未被新输入窗口破坏；日志 build/mascot-windows-input-gallery-exit.log。文档检查、OpenSpec 严格验证、私有实现与新增原生辅助程序的代码格式及差异空白检查通过。任务进度仍为 29/41，4.2 的 macOS 部分、其他待验收项及各平台完整矩阵继续保留。


## 2026-10-30 macOS 公共区域输入实现（4.2 仍待验收）

新增私有 Objective-C++ AppKit 输入适配与独立背后进程测试，不新增公共 API。视觉窗口整窗穿透，QRegion 非重叠矩形映射到透明 nonactivating NSPanel；只转换该实例收到的事件，不用全局钩子、光标轮询、透明像素扫描或系统输入权限。窗口／工作区通知同步独立面板的显隐、位置和层级，CoreGraphics 只读取窗口 ID 顺序。按下期间保留输入视图，释放后应用待更新几何；隐藏／销毁清理。Expose 时重申视觉穿透策略，避免 QML 初始 flags 覆盖。默认关闭区域模式时承接全窗。

实机 macOS 26.6.2 (25G83)、arm64、Qt 6.10.2 Release、Metal（实际 graphics API=6）、DPR=1。原始 child-window 方案在真实点击中椭圆角点仍被窗口组拦截，未记为通过；改为独立面板后系统命中检查通过。最终部署版直接运行 nativeLifecycle、nativeEventDelivery、nativeShapePlacement 为 5 passed／0 failed，2095 ms，日志 `/tmp/qcat-deployed-direct.txt`。该结果包含系统命中与进程内事件，不代表跨进程输入通过。

用户明确允许本次临时辅助功能权限，已在系统设置将部署 app 添加为开启。实际直接运行 --native-input 时 CGPreflightPostEventAccess 仍为 false，routing 跳过，报告 inputAcceptance=Pending、nativeLifecycle=Passed、checks=[]；日志 `/tmp/qcat-real-native.txt`，报告 `/tmp/qcat-real-native.json`。通过 LaunchServices 启动的进程未进入 main，采样停在 dyld 依赖文件 open；完整自包含部署及 codesign --verify --deep --strict 校验成功后仍复现，未生成测试日志。这些启动和权限失败不算真实输入验收。仅停止本轮测试进程，已在系统设置确认 tst_mascotmacosinput 权限关闭；既有 qcat-mascot-probe 仍关闭。4.2 保留未勾选，进度 29/41，真实穿透、键盘及连续拖动仍待验收。

构建目录为 build/Qt_6_10_2_for_macOS-Release。完整构建通过；最终回归结果见本节后续补充。安装到 `/tmp/qcat-mac-input-install` 成功。临时日志不作为版本控制内的持久证据，以上环境、结果与限制为本次记录；重复步骤见 VALIDATION 的“macOS 公共区域输入”。

最终补充：完整 CTest 22/22 通过（157.05 秒，`/tmp/qcat-macos-final-ctest.log`），库与 Gallery QML lint 通过。重新安装最终库后，已安装消费者以 QT_QPA_PLATFORM=offscreen、QT_QUICK_BACKEND=software 运行退出 0；首次未指定离屏时因沙箱无屏幕退出，未计为成功。文档检查 38 文件／519 本地链接、OpenSpec strict 和 git diff --check 通过。原生测试及背后 helper 已退出；权限 UI 确认为 off。仍不声明 4.2 或完整 macOS 平台验收通过。


## 2026-10-30 macOS 公共区域真实输入续验

LaunchServices 本轮恢复正常：部署版原生结构／事件／椭圆命中在 DPR=1 为 5 passed（1794 ms），DPR=1.25 为 5 passed（1472 ms），均 Metal。用户再次授权仅本轮临时辅助功能权限并自行完成系统密码验证。首次真实输入已通过矩形、椭圆、多边形、镜像、缩放、整窗穿透恢复及被动焦点，但气泡点击后空格未触发；该次为 5 passed／1 failed，不记为完整通过（`/tmp/qcat-continue-real.txt`）。

核对本机 Qt Cocoa 源码：QCocoaWindow::shouldRefuseKeyWindowAndFirstResponder 同时拒绝 WindowDoesNotAcceptFocus 与 WindowTransparentForInput。因此视觉窗口改由 AppKit ignoresMouseEvents=true 保持鼠标穿透，普通显示及角色点击仍不激活；只有公共气泡显式请求键盘时清除拒绝焦点标志、激活本应用并将 Qt 内容视图设为 first responder／key window。同步也覆盖空输入区域，避免 Qt 重设 flags 后误接收整窗鼠标。

重建并部署后 codesign --verify --deep --strict 通过；重新签名使旧系统授权失效，第一次复验 routing 跳过并报告 Pending。通过系统设置为同一路径的同一测试 app 刷新本轮授权后，真实输入最终结果：

- DPR=1、Metal：6 passed／0 failed／0 skipped，7692 ms；`build/macos-public-input-final.txt` 与同名 JSON。
- QT_SCALE_FACTOR=1.25、Metal：6 passed／0 failed／0 skipped，7551 ms；`build/macos-public-input-final-scaled.txt` 与同名 JSON。Qt 分数缩放模拟不代替混合 DPI 硬件验收。

两份报告 postEventAccess=true、inputAcceptance=Passed，10 组记录覆盖默认阻挡、矩形及背后焦点／真实 A 键、椭圆角点与中心、多边形内外、镜像、角色缩放、整窗穿透及恢复、气泡穿透／点击及真实空格、六步连续拖动终点（+30,+18）、关闭区域模式恢复整窗阻挡；另断言隐藏后命中独立背后窗口、销毁后原生输入窗口 ID 全部消失。测试仅向本轮进程窗口发送事件，背后独立 helper 真实记录 6 次点击／1 次按键；主动气泡键盘后背后 keyWindow=false，普通角色阶段保持 true。验收后已确认 tst_mascotmacosinput 辅助功能关闭，qcat-mascot-probe 仍关闭。完整平台 acceptance 仍 Pending，多屏、真实混合 DPI、Spaces／全屏等未据此声明通过。

本轮完成 4.2，进度 30/41；4.5 等其余任务保持原状态。最终构建／回归补充见下方。

最终补充：完整构建通过；空输入区域也重申 AppKit 穿透策略后的最终代码，原生结构／同实例事件／椭圆命中再次 5 passed（1642 ms，`/tmp/qcat-continue-last-structural.txt`）。最终全量 CTest 22/22（160.29 秒，`/tmp/qcat-continue-final-ctest.log`）、库／Gallery QML lint、重新安装后的消费者离屏运行均通过。文档检查 38 文件／520 本地链接、OpenSpec strict、差异空白检查通过。测试及独立 helper 无残留进程；辅助功能开关最终为 off。OpenSpec apply 已确认 4.2 完成、30/41。


## 2026-10-30 macOS 置顶与按需焦点（4.5）

扩展既有独立进程真实输入程序，不新增公开 API。部署版先运行无需权限的原生结构／同实例事件／椭圆命中：5 passed（1762 ms）；既有 Cocoa publicBubbleKeyboardLifecycle：3 passed（700 ms）。用户明确授权仅本轮辅助功能权限，通过系统设置刷新同一测试 app 的签名记录。

首轮真实输入中被动显示、动作及气泡增量更新后的真实按键均进入独立 helper，但关闭 alwaysOnTop 后原生层级为 3 而非普通层 0，测试 5 passed／1 failed（23650 ms，`/tmp/qcat-focus-policy-real.txt`），未计通过。本机 Qt Cocoa 源码 windowLevel 对 Qt::Tool 默认使用 NSFloatingWindowLevel，移除 WindowStaysOnTopHint 仍然浮动。因此私有 AppKit 输入适配同步视觉窗口与输入面板的显式置顶策略：开启为 NSModalPanelWindowLevel，关闭为 NSNormalWindowLevel，仅在变化时写入，不改变独立宿主生命周期。保留非激活输入面板与按需气泡焦点。

重建、部署及 codesign --verify --deep --strict 通过，刷新同一 app 的本轮签名授权后，macOS 26.6.2 / Qt 6.10.2 arm64 Release / Metal 真实输入：

- DPR=1：6 passed／0 failed／0 skipped，11581 ms，`build/macos-mascot-focus-final.txt`／`.json`。
- QT_SCALE_FACTOR=1.25：6 passed／0 failed／0 skipped，11283 ms，`build/macos-mascot-focus-scaled.txt`／`.json`。分数缩放模拟不代替实际混合 DPI 硬件。

两份报告 inputAcceptance=Passed、postEventAccess=true，16 组证据。沿用原 10 组区域输入、键盘、拖动与清理检查，新增六组：passive-show-action-bubble-update、always-on-top-toggle-without-activation、real-tab-backtab-and-keyboard-close、focus-release-hide／passthrough／deactivate。独立 helper 最终记录 6 次点击、9 次真实按键；被动操作后始终保持其 keyWindow=true，主动气泡后切换到本测试窗口。置顶关闭时读回原生普通层且系统命中背后浮动 helper，恢复时读回原层级；全程未激活用户其他应用。真实 Tab 到关闭按钮、Shift+Tab 返回动作、空格关闭气泡；关闭／隐藏／整窗穿透／失活后恢复 WindowDoesNotAcceptFocus，再显示仍由背后应用接收按键。结束恢复原光标／前台应用并清理 helper，临时辅助功能 UI 已确认为 off，原 qcat-mascot-probe 仍 off。

本轮完成 4.5，进度 31/41。窗口坐标会随气泡尺寸变化，本次不将窗口坐标直接当作角色锚点证据；反复显示／更新的角色原点稳定性与边缘避让继续留在 5.2 验证。多屏、Spaces／全屏及其他平台完整矩阵不据此标为通过，报告整体 acceptance 仍 Pending。

最终检查：完整构建、库／Gallery QML lint、CTest 22/22（156.11 秒，`/tmp/qcat-focus-policy-ctest.log`）、重新安装后的独立消费者离屏运行通过。文档、OpenSpec strict 及差异空白检查随本轮收尾执行；OpenSpec apply 已确认进度 31/41。


## 2026-10-30 Gallery 切换应用后桌宠层级修复

用户反馈点击其他应用后桌宠落到后面。核对本机 Qt Cocoa 创建 Tool/NSPanel 时默认设置 hidesOnDeactivate；既有代码只对输入面板禁用，角色视觉面板未处理。修复为同步时对视觉 NSPanel 同样关闭 hidesOnDeactivate 与 floatingPanel；不请求激活应用，不修改非桌宠窗口。既有 4.5 检查验证了背后窗口初始已激活与气泡焦点切换，缺少“先激活普通宿主，再激活独立应用”的独立可见性检查，现补入 nativeLifecycle。

新增检查真实激活普通 Qt 宿主后启动独立 helper 获取前台，等待本应用失活；确认角色视觉与输入面板仍可见、原生层级保持且系统命中桌宠面板。不发送鼠标／键盘系统事件，不申请辅助功能权限。macOS 26.6.2 / Qt 6.10.2 / Metal，原生三组检查含 init/cleanup 为 5 passed、0 failed、0 skipped（2050 ms，`/tmp/qcat-deactivation-native.txt`）。库、测试与 Gallery 已重新构建。此修复不提升完整平台矩阵结论，进度保持 31/41。

补充：QT_SCALE_FACTOR=1.25 的 nativeLifecycle 为 3 passed（1081 ms，含 init/cleanup）；受影响 mascot_contracts、gallery_mascot_shutdown、mascot_macos_reject_headless 三项 CTest 全通过（13.80 秒）。文档检查 38 文件／522 本地链接、OpenSpec strict 及差异空白检查通过。未将此次不发送系统输入的检查写为新的完整输入验收。
