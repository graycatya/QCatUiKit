# 飞机姿态与设备无关体感输入

[文档索引](README.md) · [组件目录](COMPONENTS.md)

`CatAttitudeIndicator` 显示**飞机姿态**；`TiltController` 处理应用提交的**手机控制姿态**；
`CatTiltControlPanel` 显示体感操作与参数。三者均通过 `import QCatUiKit` 使用。
库、Gallery、安装包和公开头不依赖 Qt Sensors 或 MAVLink。应用自行采集传感器、解码遥测、转换坐标系、
管理控制权及发送控制消息。Gallery 两个独立模拟页面不读取硬件、不发送飞控消息。

## 飞机姿态球

```qml
import QtQuick
import QCatUiKit

CatAttitudeIndicator {
    width: 280; height: 280
    pitchDegrees: 12    // 应用中绑定飞机遥测，抬头为正
    rollDegrees: 25     // 右滚为正
    attitudeValid: true // 应用根据遥测质量及接收时效设置
}
```

| 属性 | 默认值与行为 |
| --- | --- |
| `pitchDegrees` | 0；度；接受规范化俯仰 −90…90 |
| `rollDegrees` | 0；度；有限值，绘制时按 360° 归一化 |
| `attitudeValid` | false；应用提供的遥测有效性 |
| `displayValid` | 只读；同时检查有效标志、角度、显示跨度 |
| `visiblePitchSpanDegrees` | 60；10…180；圆盘直径对应的俯仰显示跨度 |
| `pitchLadderVisible` / `rollScaleVisible` / `readoutVisible` | true；分别控制俯仰刻度、横滚刻度与数字读数 |
| `skyColor` / `groundColor` | `#2775a5` / `#80543b` |
| `aircraftColor` / `scaleColor` | `#ffe278` / `#ffffff` |
| `theme` | Theme；外框、失效提示、文字尺寸由 Token 提供 |

默认隐式大小 280×280，在可用矩形中居中绘制内接圆。5° 小刻度、10° 数字刻度。
先在仪表局部竖轴作俯仰位移，再绕中心反向横滚；固定飞机标记不运动。
抬头时地平线下移，右滚时地平线逆时针旋转。支持倒飞和 ±90° 俯仰；
大俯仰时圆内可以全为天空或地面。输入不合法时显示“姿态数据不可用”，不显示归零的假姿态。
应用须将飞控弧度及姿态表示转换为上述规范；飞机坐标转换不属于组件内部职责。

圆形天地区域直接使用圆及圆弓形几何，刻度按圆边界约束；不使用离屏蒙版或每帧 Canvas 重绘。
默认无追赶动画，不修改输入属性。QML Shape 的平台渲染支持仍需在目标设备验证。
此组件没有操作信号，只有标准属性变更通知；不采集传感器，不推断遥测超时。

## TiltController

C++ 使用 `<QCatUiKit/TiltController>`、`<QCatUiKit/TiltController.h>` 或
`<QCatUiKit/qcattiltcontroller.h>`，类为 `QCatUiKit::TiltController`。
对象采用 Pimpl；所有调用在其 QObject 所属线程执行，跨线程通过 queued connection 交接。

### 输入坐标与时间

`submitOrientation(QQuaternion orientation, qint64 timestampUs)` 接受从**设备控制坐标系到固定参考坐标系**
的旋转。控制坐标约定为 +X 向屏幕右侧，+Y 向屏幕顶部，+Z 向屏幕外侧。
应用先完成传感器旋转顺序、轴符号、屏幕方向及坐标系转换，不能直接把任意 x/y/z 欧拉角当飞控姿态。

四元数顺序为 w,x,y,z，非单位四元数会归一化；零长度、非有限值被拒绝。
时间戳必须非负、严格递增，单位微秒，来自单调时钟且属于同一输入源；QML 中使用小于 2^53 的整数，
建议从采集会话起点计时。不要使用易跳变的墙钟或在处理积压队列时伪造新时间戳。
采样实际发生时间与提交时刻的差异由应用检查；控制器检测时间戳间隔与本地到达间隔，无法识别源端任意时间原点下的绝对陈旧程度。

关闭、reset 或更换源后可重新从 0 计时。屏幕方向或参考坐标系变化时，应用必须先撤销控制并调用
`resetCalibration()`，不能将两套参考系连续送入同一校准会话。

### 属性与参数

| 属性 | 默认值 / 约束 |
| --- | --- |
| `enabled` | false；关闭清除采样、校准及输出；重新开启需重新校准 |
| `controlAllowed` | false；应用授权；false 立即退出 Active，重新设为 true 不自动恢复控制 |
| `state` | 只读 Disabled=0 / Uncalibrated=1 / Calibrating=2 / Ready=3 / Active=4 / Fault=5 |
| `calibrated` / `sampleFresh` | 只读，校准状态与到达时效 |
| `outputValid` | 只读，仅 Active 为 true |
| `rollInput` / `pitchInput` | 只读 −1…1；Ready 中也更新预览，但不可用作有效控制输出 |
| `fullScaleAngleDegrees` | 25；5…60，且必须大于死区；越小越灵敏 |
| `deadzoneDegrees` | 2；0 ≤ deadzone < fullScale |
| `filterTimeConstantMs` | 100；0…1000，0 禁用低通 |
| `expo` | 0.2；0…1，越大中心越柔和 |
| `invertRoll` / `invertPitch` | false |
| `sampleTimeoutMs` | 250；20…5000，同时约束到达间隔和时间戳间隔 |
| `calibrationDurationMs` | 500；100…3000 |
| `error` | 只读错误说明；reset 清除 |

参数非法赋值被忽略。参数改变会退出 Active、清零滤波和杆量，需要再次显式 engage；
改变校准时长会清除校准和采样。初值用于模拟调试，不代表特定飞机或设备的适用参数。

### 方法与信号

- `submitOrientation(q, timestampUs) → bool`：提交一帧；禁用或 Fault 中返回 false；合法输入返回 true。
- `calibrate() → bool`：仅新鲜数据的 Uncalibrated/Ready 状态可开始；Active 中拒绝。
- `engage() → bool`：仅 Ready、已校准、新鲜且 controlAllowed 才成功。
- `disengage()`：Active → Ready；保留预览数值，outputValid 立即撤销。
- `resetCalibration()`：清除错误、校准、时间基准和数据，回到 Uncalibrated 或 Disabled。
- `calibrationFinished(bool success, QString reason)`：成功、主动取消或故障中止；运动会重新开始稳定窗口。
- `controlSample(double roll, double pitch, bool valid)`：同一更新的完整控制快照；也发布失效通知。
- `faultOccurred(QString reason)`：输入、超时或姿态范围故障。其他属性有对应 Changed 通知。

正常流程是 Disabled → Uncalibrated → Calibrating → Ready → Active。
结束按压或授权撤销回 Ready；输入故障进入 Fault，清除校准与输出，必须 reset 后重新校准。
无新采样时由精确定时器撤销输出；QObject 事件循环必须正常运行，库不是硬实时看门狗。
发送线程仍须独立检查当前控制权、会话及样本年龄，不能盲目重放已排队的 valid=true 信号。

### 校准和映射

校准在完整时长内保持相对起始姿态不超过 2°，至少采集三帧。离开该范围则重启稳定窗口。
对四元数先统一符号，再平均并归一化；校准完成清零滤波状态。

```text
qRelative = conjugate(qZero) * qCurrent
normal = rotate(qRelative, [0, 0, 1])
rollAngle  = degrees(atan2(normal.x, normal.z))
pitchAngle = degrees(atan2(normal.y, normal.z))
```

控制方向定义在校准时的局部坐标中。右边缘下压为正 rollInput，顶部（朝前）边缘下压为正 pitchInput；
它们是杆量方向，不是飞控抬头为正的 pitch 遥测。绕设备局部 Z 的纯扭转不产生倾斜输入。
相对屏幕法线倾角超过 75° 则进入 Fault，避免倒置和提取奇异点；应用可按场景设更严格的限制。

每轴按真实时间间隔滤波，再限幅、连续死区及曲线映射：

```text
alpha = 1 - exp(-dtSeconds / tauSeconds)  // tau=0 时 alpha=1
filtered += alpha * (angle - filtered)
a = clamp(filtered, -fullScale, fullScale)
v = sign(a) * max(abs(a) - deadzone, 0) / (fullScale - deadzone)
output = (1 - expo) * v + expo * v^3
```

### C++ 调用顺序

下面是调用片段，应用须提供 `deviceToReference`、`monotonicMicroseconds` 和 `applicationAllowsControl`，并持续提交采样；不是独立可运行程序。使用前包含 `<QCatUiKit/TiltController>`。

```cpp
QCatUiKit::TiltController tilt;
tilt.setEnabled(true);
// 应用持续将完成轴转换的融合姿态提交至 tilt 所在线程：
tilt.submitOrientation(deviceToReference, monotonicMicroseconds);
// 收到首帧后，由用户操作触发；后续持续提交完成稳定校准。
tilt.calibrate();
// 应用确认允许接管后设置授权；按住使能再调用 engage()。
tilt.setControlAllowed(applicationAllowsControl);
// 通过 outputValid/controlSample 交接到应用控制层；此处不发送任何协议消息。
```

应用负责 Qt Sensors 或其他采集方式、失活/后台处理、目标飞行器、控制仲裁、油门和偏航归属、
MAVLink 打包与固定发送节奏，以及释放/停止后的飞控行为。释放控制权、杆量归中、停止发送的语义不同。
库不会自动解锁、变更飞行模式或发送任何网络数据。平台传感器授权与许可由应用所选依赖决定。

## CatTiltControlPanel

继承 CatPane，默认隐式宽度 380。输入属性由应用绑定：
`controlEnabled`、`controlAllowed`、`sampleFresh`、`controllerState`、`rollInput`、`pitchInput`、
`fullScaleAngleDegrees`、`deadzoneDegrees`、`filterTimeConstantMs`、`expo`、`errorText`。
参数默认值与 TiltController 一致；状态默认 Disabled。`holding` 和 `canEngage` 为只读 UI 状态。

面板发出 `enabledRequested(bool)`、`calibrateRequested()`、`resetCalibrationRequested()`、
`engageRequested()`、`disengageRequested()`、`fullScaleAngleEdited(real)`、`deadzoneEdited(real)`、
`filterTimeConstantEdited(real)` 和 `expoEdited(real)`，由应用执行并回写真实状态。
程序更新输入属性不触发用户操作信号。完整可复制绑定见 Gallery 的 Tilt control 页面。

鼠标、触摸、空格键均可按住使能。释放、取消、隐藏、禁用、窗口失活、应用进入后台、
撤销授权或数据变旧时请求 disengage。面板销毁也释放其持有的按压。
应用仍须管理独立于 UI 生命周期的控制器与发送器；隐藏 UI 不应成为唯一退出保障。
参数区在 Active 时禁用，避免持杆中改变映射；高级参数折叠展示，窄屏数值编辑自动堆叠。
两轴预览明确标为手机控制输入，不使用飞机姿态球显示手机数据。

## Gallery 与验证入口

Gallery 的 Spatial 分类包含 Aircraft attitude 与 Tilt control 两个独立页面。
前者提供飞机 pitch/roll 和遥测失效模拟；后者提供手机倾斜、采样暂停、授权撤销和真实校准流程。
后者每 20ms 生成模拟四元数；离开页面停止模拟并禁用控制器。模拟能力不代表 Android 传感器已验证。

- `tests/auto/motion/tst_tiltcontroller.cpp`：坐标方向、任意零点、四元数符号、校准稳定性、时间相关滤波、死区、故障及控制状态。
- `tests/auto/qml/data/tst_attitude.qml`：实际画面方向、圆形边界、固定标记、无效输入、按压与键盘取消、窄屏。
- `tests/auto/gallery/data/tst_attitude.qml`：独立数据源、完整模拟流程、超时、翻译和参数回写。

已执行的平台和后端记录见 [验证说明](VALIDATION.md)，未进行真实飞控或手机传感器联调。
