# build-install Specification

## Purpose

配置、构建及安装 QCatUiKit 共享库、QML 模块、公开头和消费方 CMake 包。

该基线用于约束后续变更所涉及的现有对外行为，记录已核查的正常流程、输入边界和错误结果，不以模块名称代表全部接口均已逐项覆盖。

## Dependencies

- CMake >=3.24、C++17、Qt >=6.8：Core、Gui、Qml、Quick、QuickControls2、ShaderTools 及匹配 GuiPrivate。
- 开启测试依赖 Test、QuickTest；Gallery 依赖 LinguistTools。
- ThorVG 工具链依赖 Python >=3.7、Meson >=1.3 和构建工具；自动准备路径可通过项目缓存变量覆盖。

## Requirements

### Requirement: R01 BuildDefaults

系统 SHALL 默认构建 examples 并启用 ThorVG 配置。

#### Scenario: 关闭示例

- WHEN 配置 QCATUIKIT_BUILD_EXAMPLES=OFF
- THEN 系统 SHALL NOT 添加 examples 子目录和 Gallery 测试。

#### Scenario: 关闭测试

- WHEN 配置 BUILD_TESTING=OFF
- THEN 系统 SHALL NOT 添加 tests 子目录或要求测试组件。

### Requirement: R02 ReleaseIdentity

构建 SHALL 区分发布渠道与编译配置。

#### Scenario: 默认渠道

- WHEN 未覆盖 QCATUIKIT_RELEASE_CHANNEL
- THEN 渠道 SHALL 为 experimental。

#### Scenario: 渠道无效

- WHEN 渠道既不是 stable 也不是 experimental
- THEN 配置 SHALL 以 FATAL_ERROR 终止。

#### Scenario: 编译模式

- WHEN 在 Debug 与 Release 间切换且渠道不变
- THEN 渠道身份 SHALL NOT 因编译模式而改变。

### Requirement: R03 PublicPackage

安装 SHALL 提供 QCatUiKit::QCatUiKit 和 QML URI QCatUiKit。

#### Scenario: 头文件入口

- WHEN 安装已列出的公开头
- THEN 系统 SHALL 提供真实头及类名转发入口，并将其置于 QCatUiKit 包含目录。

#### Scenario: 包查找

- WHEN 消费方查找安装包
- THEN 包 SHALL 提供 CMake 配置、导出目标和同主版本的包版本匹配信息。

#### Scenario: Qt 模块查找

- WHEN 通过 Qt6 COMPONENTS QCatUiKit 查找
- THEN 兼容包 SHALL 使用 Qt 版本匹配信息而非库数字版本。

### Requirement: R04 ToolValidation

启用相关依赖构建时 SHALL 校验工具可执行性及版本。

#### Scenario: Python 不可用

- WHEN 需要 Python 但无可用解释器且宿主不满足内置 Windows 引导条件
- THEN 配置 SHALL 终止并要求指定 Python。

#### Scenario: Meson 无效

- WHEN Meson 执行失败或版本低于 1.3
- THEN 配置 SHALL 终止。

## Data Model

本模块无独立数据模型。主要缓存配置：QCATUIKIT_BUILD_EXAMPLES=ON；QCATUIKIT_ENABLE_THORVG=ON；QCATUIKIT_BUILD_THORVG 默认跟随启用值；QCATUIKIT_RELEASE_CHANNEL=experimental；构建测试由 BUILD_TESTING 控制。安装包包含库、插件、QML 文件、类型信息及许可证。

## Known Deviations / Technical Debt

- 库实现依赖 Qt 私有图形接口，配置成功不等于任意 Qt 小版本的运行兼容。
- 自动依赖准备依赖网络或已准备缓存；非 Windows x64 宿主无相同的内置 Python 引导路径。
- 配置包的同主版本匹配不代替动态库 ABI 验证。

## Non-Goals

- 本模块 SHALL NOT 安装用户业务应用、自动发布发行包或修改宿主系统 Qt。

## Assumptions

- 宿主 SHALL 提供兼容的 Qt、编译器、运行库及依赖来源；安装前缀 SHALL 可写。
