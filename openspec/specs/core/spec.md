# core Specification

## Purpose

提供已加载库的版本、发布渠道和 ABI 身份查询，并向 QML 暴露版本信息单例。

该基线用于约束后续变更所涉及的现有对外行为，记录已核查的正常流程、输入边界和错误结果，不以模块名称代表全部接口均已逐项覆盖。

## Dependencies

- Qt Core、Qml。
- 构建生成的版本数据、QCATUIKIT_RELEASE_CHANNEL、ABI 配置。

## Requirements

### Requirement: R01 RuntimeIdentity

系统 SHALL 返回当前已加载共享库的版本身份。

#### Scenario: 数字版本

- WHEN 调用 version()
- THEN 结果 SHALL 为不带渠道后缀的 QVersionNumber。

#### Scenario: 版本字符串

- WHEN 查询 experimental 构建的 versionString()
- THEN 结果 SHALL 在数字版本后包含 -experimental。

#### Scenario: 稳定渠道

- WHEN 查询 stable 构建的 versionString()
- THEN 结果 SHALL 为数字版本字符串。

### Requirement: R02 ChannelAndAbi

系统 SHALL 提供渠道枚举、布尔查询和 ABI 数字。

#### Scenario: 渠道一致性

- WHEN 查询 releaseChannel()、isStableRelease() 和 isExperimentalRelease()
- THEN 结果 SHALL 描述同一渠道，两个布尔结果 SHALL 互斥。

#### Scenario: ABI

- WHEN 查询当前构建 abiVersion()
- THEN 结果 SHALL 为 1。

### Requirement: R03 QmlIdentity

QML QCatUiKit 单例 SHALL 与 C++ 查询保持一致。

#### Scenario: QML 查询

- WHEN 读取单例 version、versionMajor、versionMinor、versionPatch、abiVersion 和 releaseChannel
- THEN 值 SHALL 与库查询一致，渠道字符串 SHALL 为 stable 或 experimental。

## Data Model

| 数据 | 类型 | 当前值或约束 |
| --- | --- | --- |
| 数字版本 | QVersionNumber | 1.0.0 |
| ReleaseChannel | enum class | Stable=0、Experimental=1 |
| ABI | int | 1 |
| QML 版本属性 | 只读属性 | 来自已加载库 |

## Known Deviations / Technical Debt

- 版本查询描述构建身份，不执行运行时二进制兼容性检测。
- 当前数字版本与 ABI 为构建固定值。

## Non-Goals

- 本模块 SHALL NOT 执行更新下载、包发布或自动兼容迁移。

## Assumptions

- 宿主 SHALL 加载与目标平台兼容的库；QML 查询 SHALL 已完成模块导入。
