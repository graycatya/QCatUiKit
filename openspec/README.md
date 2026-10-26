# QCatUiKit OpenSpec 基线与使用范围

本目录区分现有行为规约 `specs/` 和后续变更 `changes/`。基线来自当前源码、现有测试及接口文档核对；测试文件存在不代表本轮已执行测试。现有 API 文档继续作为接入与完整接口目录，不由 OpenSpec 替代。

项目当前架构、模块依赖、数据流和线程边界见 [项目设计基线](design.md)。该文件是项目自定义上下文，具体变更仍使用 `changes/<变更名称>/design.md`。

## 按能力组织

不按每个源文件或测试文件建立 spec。独立可观察的功能、交互、构建入口和工具行为各自形成能力规约；同一需求的 C++ 与 QML 行为放在其所属能力下。

| 能力 | 基线 | 核查入口与现有证据 | 本轮范围 |
| --- | --- | --- | --- |
| 版本身份 | [core](specs/core/spec.md) | `src/uikit/core/`、`tests/auto/version/tst_version.cpp` | 数字版本、渠道、ABI、QML 查询 |
| 主题 | [theme](specs/theme/spec.md) | `qcatthemepalette.cpp`、`qml/Theme.qml`、`tests/auto/models/tst_models.cpp` | 配置、密度、覆盖、通知 |
| 数据模型 | [models](specs/models/spec.md) | `src/uikit/models/`、`tests/auto/models/tst_models.cpp` | 表格、消息 CRUD、树快照 |
| 体感 | [motion](specs/motion/spec.md) | `qcattiltcontroller.cpp`、`tests/auto/motion/tst_tiltcontroller.cpp` | 输入、校准、授权、失效、参数 |
| 停靠 | [docking](specs/docking/spec.md) | `qcatdockcontroller.cpp`、`CatDockWorkspace.qml`、`tests/auto/dock/` | 注册、停靠、恢复、内容与界面权限边界 |
| LED | [led](specs/led/spec.md) | `qcatledsource.cpp`、`qcatledmixer.cpp`、`qcatledstorage.cpp`、`qcatlightingframe.h`、`tests/auto/led/` | Source、帧、混合、仓库、输出/预览边界 |
| 图表 | [charts](specs/charts/spec.md) | `qcatchartaxis.cpp`、`qcatchartseries.cpp`、`qcatchartseries3d.cpp`、`qcatchartfit.cpp`、`qcatchartitem.cpp`、`tests/auto/charts/` | 核心数据/轴、网格、拟合失败与 PNG 导出 |
| 矢量 | [vector](specs/vector/spec.md) | `src/uikit/vector/`、三个 Cat 包装组件、`tests/auto/vector/tst_vector.cpp` | 既有基线保持不变 |
| 基础 UI | [ui-components](specs/ui-components/spec.md) | `CatButton.qml`、`CatTextField.qml`、`CatCheckBox.qml`、`CatChatInput.qml` | 主题、加载禁用、输入、复选和提交 |
| 构建安装 | [build-install](specs/build-install/spec.md) | 根与模块 CMake、`cmake/QCatUiKitBuildTools.cmake`、`tests/auto/cmake/` | 配置开关、包/头入口、渠道和工具校验 |
| 示例应用 | [gallery](specs/gallery/spec.md) | Gallery `main.cpp`、`Main.qml`、`gallerylanguage.cpp`、`tests/auto/gallery/` | CLI、导航搜索、翻译、截图和基准入口 |
| 测试基础设施 | [validation-infrastructure](specs/validation-infrastructure/spec.md) | `tests/CMakeLists.txt`、`tests/runqttest.cmake`、各套件 CMake | 注册条件、日志、时限、GPU 边界 |

本表列出可复核的证据入口，不宣称每个列出的测试已覆盖该能力全部场景。

## 其他项目内容如何处理

- **单元测试**：作为需求的可执行证据。后续 change 的 tasks 记录新增或受影响检查，不为每个测试文件建立镜像 spec。测试运行工具本身的行为归 validation-infrastructure。
- **UI 组件**：按可观察交互能力组织。基础通用控件归 ui-components；Dock、图表、LED、体感和矢量专用行为归对应模块。更细的独立组件契约可后续拆分，但不把内部对象树当作 API。
- **项目构建与安装**：对外可用的选项、依赖、包发现、安装产物和失败结果归 build-install；不用规约记录构建实现步骤。
- **examples**：Gallery 的命令行、导航、语言、截图和演示运行行为归 gallery；示例内部 helper 不成为库 API。示例覆盖不了的库功能不能由示例推导。
- **API 文档**：继续描述完整接口和接入方式；规约描述可验证行为。变更同时触及接口说明时，在 tasks 中列出文档更新与文档检查。
- **私有实现与第三方**：仅在影响公开行为、运行前提或限制时进入规约，不逐个为私有类和第三方源码建 spec。

## 覆盖边界

当前形成的是各能力的首轮基线，不是全部公开接口逐项完成的审计。尤其下列内容尚未在新规约中细化：

- 全部 Cat 控件的键盘、焦点、RTL、禁用、可访问性及窄屏场景；当前基础 UI 仅覆盖表中四个组件。
- 图表全部扩展系列、坐标转换、拾取、触摸、委托、材质和统计适配器参数。
- LED 各种布局生成规则、全部 Deck/Provider 生命周期、项目恢复字段及预览设置。
- Dock 原生平台交互的完整分支；体感姿态仪和控制面板的完整显示/交互契约。
- 构建缓存、各平台工具链与独立安装消费者的全部失败分支。

准备修改这些行为时，先核对相应源码与测试补充现状，再编写 change 的差异规约；不将缺少基线条目解释为该功能未实现。

## 后续变更

1. 需求不明确时用 `openspec-explore` 只读分析。
2. 用 `openspec-propose` 创建 proposal、差异 specs、必要 design 和 tasks。
3. 明确开始实施后用 `openspec-apply-change` 执行，并保留真实验证结果。
4. 完成后核对差异，使用归档工作流处理主规约同步及历史归档。

基线中的技术债是现状记录，不是自动授权修复的任务。

## 检查

可重复执行的检查见[验证方法](../src/uikit/doc/VALIDATION.md)。具体变更的验证结果记录在对应 change 中并随变更归档，不另设独立历史记录。

```sh
openspec list --specs
openspec validate --specs --strict --no-interactive
python tools/check_docs.py
git diff --check
```

OpenSpec 严格校验通过只证明结构满足校验器，不证明代码行为、测试覆盖或运行平台全部验收。
