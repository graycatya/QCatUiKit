# validation-infrastructure Specification

## Purpose

注册并运行现有自动测试，区分离屏行为验证、原生图形验证和示例验证。单个测试文件作为功能规约的证据，不作为独立产品模块。

## Dependencies

- CMake/CTest、Qt Test、QuickTest、已构建测试目标。
- tests/CMakeLists.txt、tests/runqttest.cmake 和各套件 CMake 配置。
- 字体设置 QCATUIKIT_TEST_FONT_DIR、QT_QPA_FONTDIR；原生图形测试依赖窗口环境。

## Requirements

### Requirement: R01 SuiteRegistration

BUILD_TESTING 启用时 SHALL 注册既有自动测试子系统。

#### Scenario: 核心套件

- WHEN 启用测试配置
- THEN 系统 SHALL 添加 models、motion、charts、led、version、qml、dock、docknative 和 vector 测试子目录。

#### Scenario: 示例条件

- WHEN 测试开启但 QCATUIKIT_BUILD_EXAMPLES=OFF
- THEN 系统 SHALL NOT 注册 Gallery 测试或 gallery_smoke。

### Requirement: R02 GpuOptIn

需要真实图形窗口的套件 SHALL 按对应选项注册。

#### Scenario: 默认关闭

- WHEN GPU 测试选项未启用
- THEN chart_render、chart_3d_render、LED 和 vector 的原生渲染检查 SHALL NOT 因其二进制已构建而自动视为已执行。

#### Scenario: 矢量 GPU

- WHEN QCATUIKIT_TEST_VECTOR_GPU=ON 且 ThorVG 已启用
- THEN 系统 SHALL 注册 vector_render 并设置 QCAT_VECTOR_GPU=1。

#### Scenario: 窗口互斥

- WHEN 通过公共注册入口添加名称以 _render 结尾的测试
- THEN CTest SHALL 为其设置 qcat_gpu_window 资源锁。

### Requirement: R03 TestLogs

QtTest 包装运行 SHALL 生成当前测试和配置独立的文本报告。

#### Scenario: 旧日志

- WHEN 测试启动前已有旧文本报告
- THEN 运行器 SHALL 清空旧报告。

#### Scenario: 失败退出

- WHEN QtTest 退出码不为零
- THEN 运行器 SHALL 报告失败并包含日志位置。

#### Scenario: 缺少报告

- WHEN QtTest 返回零但未输出文本报告
- THEN 运行器 SHALL 报告失败。

### Requirement: R04 RuntimeBounds

公共注册入口 SHALL 设置默认测试时限和所需环境。

#### Scenario: 默认时限

- WHEN 套件未覆盖时限
- THEN 测试超时 SHALL 为 300 秒。

#### Scenario: Windows 字体

- WHEN Windows 未显式配置字体目录且无 QT_QPA_FONTDIR
- THEN 字体目录默认 SHALL 使用 SystemRoot/Fonts。

#### Scenario: 具体覆盖

- WHEN 套件显式设置更具体的 TIMEOUT
- THEN CTest SHALL 使用该套件值。

## Data Model

本模块无独立产品数据模型。配置：BUILD_TESTING；QCATUIKIT_TEST_CHART_GPU、QCATUIKIT_TEST_LED_GPU、QCATUIKIT_TEST_VECTOR_GPU 默认 OFF；QCATUIKIT_TEST_FONT_DIR 为路径。测试报告按测试名称和构建配置区分。

## Known Deviations / Technical Debt

- GPU 测试默认不注册，普通 ctest 全通过不证明 GPU 路径成功。
- CTest 超时与像素证据不构成所有平台、驱动和 DPI 的覆盖。
- 本轮基线整理仅运行文档与规约校验，不重写历史测试执行结果。

## Non-Goals

- 本规约 SHALL NOT 将每个测试函数复制为独立需求或宣称自动测试覆盖全部公开接口。
- 本模块 SHALL NOT 用离屏 software 测试替代真实 GPU 验收。

## Assumptions

- 测试二进制 SHALL 已构建；平台字体和动态库 SHALL 可用；原生 GPU 套件 SHALL 在有效图形会话执行。
