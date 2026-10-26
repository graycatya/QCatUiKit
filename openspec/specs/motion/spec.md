# motion Specification

## Purpose

将应用提供的设备姿态采样转换为校准后的双轴控制输入，并管理授权、时效和故障状态。

该基线用于约束后续变更所涉及的现有对外行为，记录已核查的正常流程、输入边界和错误结果，不以模块名称代表全部接口均已逐项覆盖。

## Dependencies

- Qt Core、Gui、Qml，QQuaternion 和对象事件循环。
- 应用提供设备到参考系的姿态和单调微秒时间戳；不依赖 Qt Sensors 或 MAVLink。

## Requirements

### Requirement: R01 Lifecycle

TiltController SHALL 初始禁用，outputValid SHALL 仅在 Active 为 true。

#### Scenario: 启用

- WHEN enabled 从 false 改为 true
- THEN 系统 SHALL 清除采样与校准并进入 Uncalibrated。

#### Scenario: 禁用或复位

- WHEN 关闭 enabled 或调用 resetCalibration()
- THEN 系统 SHALL 清除错误、校准、采样和输出，进入 Disabled 或 Uncalibrated。

### Requirement: R02 InputValidation

系统 SHALL 接受非零有限四元数及非负严格递增时间戳。

#### Scenario: 非单位姿态

- WHEN 提交合法非单位四元数
- THEN 系统 SHALL 归一化并接受采样。

#### Scenario: 禁用或故障

- WHEN Disabled 或 Fault 时提交采样
- THEN 方法 SHALL 返回 false。

#### Scenario: 输入错误

- WHEN 启用后提交零长度、非有限姿态或非递增时间戳
- THEN 系统 SHALL 返回 false 并进入 Fault，清除校准和有效输出并发出故障说明。

### Requirement: R03 Calibration

校准 SHALL 需要新鲜输入和完整稳定窗口。

#### Scenario: 开始校准

- WHEN Uncalibrated 或 Ready 有新鲜输入且调用 calibrate()
- THEN 系统 SHALL 返回 true 并进入 Calibrating。

#### Scenario: 稳定完成

- WHEN 采样相对起始姿态保持 2 度范围内，持续达到校准时长且累计至少三帧
- THEN 系统 SHALL 进入 Ready 并发出 calibrationFinished(true, 空说明)。

#### Scenario: 校准运动

- WHEN 采样偏离稳定范围
- THEN 系统 SHALL 重新开始稳定窗口。

#### Scenario: 校准拒绝

- WHEN Active、Fault、Disabled、已在校准或无新鲜数据时请求校准
- THEN 系统 SHALL 返回 false。

### Requirement: R04 Engagement

接管 SHALL 要求 Ready、已校准、新鲜数据和 controlAllowed。

#### Scenario: 接管成功

- WHEN 全部条件满足并调用 engage()
- THEN 系统 SHALL 进入 Active 并返回 true。

#### Scenario: 撤销授权

- WHEN Active 时将 controlAllowed 改为 false
- THEN 系统 SHALL 进入 Ready 并撤销有效输出；重新授权 SHALL NOT 自动接管。

#### Scenario: 主动退出

- WHEN Active 时调用 disengage()
- THEN 系统 SHALL 保留预览杆量并进入 Ready。

### Requirement: R05 TimeoutAndMapping

系统 SHALL 检查输入时效、限制双轴输出至 [-1,1] 并应用映射参数。

#### Scenario: 输入超时

- WHEN 已收到采样后本地到达间隔达到超时，或后续时间戳间隔大于超时
- THEN 系统 SHALL 进入 Fault 并撤销输出。

#### Scenario: 姿态超限

- WHEN 校准后相对屏幕法线倾角超过 75 度
- THEN 系统 SHALL 进入 Fault。

#### Scenario: 修改参数

- WHEN 有效改变满量程、死区、滤波、expo 或反向配置
- THEN 系统 SHALL 退出 Active 并清零滤波与杆量，SHALL NOT 自动重新接管。

#### Scenario: 参数无效

- WHEN 参数超出公开范围
- THEN 系统 SHALL 保留原值。

## Data Model

状态：Disabled=0、Uncalibrated=1、Calibrating=2、Ready=3、Active=4、Fault=5。

默认：enabled=false、controlAllowed=false、fullScaleAngleDegrees=25（5..60 且大于死区）、deadzoneDegrees=2（0..满量程，不含上界）、filterTimeConstantMs=100（0..1000）、expo=0.2（0..1）、invertRoll/invertPitch=false、sampleTimeoutMs=250（20..5000）、calibrationDurationMs=500（100..3000）。输入为 QQuaternion 和 qint64 timestampUs；输出为 double roll/pitch 和 bool valid。

## Known Deviations / Technical Debt

- 看门狗依赖事件循环，不是硬实时保障。
- 无法识别任意时间原点下采样的绝对陈旧程度。
- 已排队的有效信号不会因后续失效自动从应用队列撤回。

## Non-Goals

- 本模块 SHALL NOT 采集传感器、转换屏幕坐标、仲裁飞控控制权或发送协议消息。
- 本模块 SHALL NOT 负责油门、偏航或飞行模式。

## Assumptions

- 应用 SHALL 提供统一坐标系、同源单调时间及有效事件循环；发送端 SHALL 独立检查控制权和样本时效。
