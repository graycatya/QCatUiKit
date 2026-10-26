# vector Specification

## Purpose

为 Qt Quick 提供 SVG 显示、Lottie 动画播放和声明式矢量画布。公开组件为 VectorItem、CatSvgImage、CatLottieAnimation 和 CatVectorCanvas，共享加载状态、尺寸适配、错误反馈及播放控制。

本文件为当前实现的基线，不定义新增能力。

## Dependencies

- Qt Core、Gui、Qml、Quick 和支持 QQuickRhiItem 的 QRhi 图形环境。
- ThorVG，默认构建版本为 v1.1.3；内部适配模块 adapters/qcatthorvgqrhi。
- 构建配置 QCATUIKIT_ENABLE_THORVG 和源码覆盖配置 QCATUIKIT_THORVG_SOURCE_DIR。
- QML 包装组件依赖 ThemePalette、Theme、主题 tokens、reducedMotion、CatBusyIndicator 和 CatLabel。
- 内容来自本地文件、qrc 资源或内存字节数据；无数据库和业务网络接口依赖。
- 无模块专用运行时环境变量；实际图形环境由宿主 Qt Quick 配置决定。

## Requirements

### Requirement: R01 ComponentDefaults

系统 SHALL 提供共享 VectorItem 接口的三个 QML 包装组件。

#### Scenario: 原生对象初始状态
- WHEN 创建未设置内容的 VectorItem
- THEN 对象 SHALL 为 Null 状态、Svg 格式、128 × 128 隐式尺寸。

#### Scenario: 包装组件默认值
- WHEN 分别创建 CatSvgImage、CatLottieAnimation 和 CatVectorCanvas
- THEN 三者 SHALL 分别使用 Svg、Lottie 和 Canvas；仅 CatLottieAnimation 的 autoPlay 默认值 SHALL 为 true。

### Requirement: R02 ContentSelection

SVG 和 Lottie SHALL 优先读取非空 sourceData，否则读取 source；Canvas SHALL 使用 shapes 和 viewBox。

#### Scenario: 内存优先
- WHEN sourceData 非空且 source 非空
- THEN SVG 或 Lottie SHALL 使用 sourceData，SHALL NOT 读取 source 文件。

#### Scenario: 空来源
- WHEN SVG 或 Lottie 重新加载时 sourceData 和 source 均为空
- THEN 系统 SHALL 进入 Null。

#### Scenario: Canvas 来源
- WHEN Canvas 重新加载且形状列表通过校验
- THEN 系统 SHALL 使用形状列表，SHALL NOT 解析 source 或 sourceData。

### Requirement: R03 SourceAndLimits

系统 SHALL 支持本地文件、qrc 和相对地址；单份文档 SHALL 不超过 67,108,864 字节。

#### Scenario: 相对地址
- WHEN 加载相对 source
- THEN 系统 SHALL 按 QML 上下文解析；无 QML 上下文时 SHALL 按当前工作目录解析为本地绝对路径。

#### Scenario: 不支持的协议
- WHEN 已启用后端且通过 source 加载非本地文件、非 qrc URL
- THEN 系统 SHALL 进入 Error，错误为 Vector source supports local files and qrc URLs only。

#### Scenario: 打开失败
- WHEN 已启用后端但文件或资源无法打开
- THEN 系统 SHALL 进入 Error，错误 SHALL 以 Cannot open vector source: 开头并包含文件错误说明。

#### Scenario: 文件超限
- WHEN 文档文件大小超过 64 MiB
- THEN 系统 SHALL 进入 Error，错误为 Vector source exceeds 64 MiB。

#### Scenario: 数据为空或超限
- WHEN 读取结果为空或内存文档超过 64 MiB
- THEN 系统 SHALL 进入 Error，错误为 Vector data is empty or exceeds 64 MiB。

#### Scenario: 数据等于上限
- WHEN 文档恰好为 64 MiB
- THEN 系统 SHALL 允许继续解析，SHALL NOT 仅因等于上限而拒绝。

### Requirement: R04 BackendAndStatus

系统 SHALL 暴露 Null、Loading、Ready、Error 和 errorString；backendAvailable SHALL 表示编译支持。

#### Scenario: 禁用后端
- WHEN 未编译 ThorVG 支持且提交通过前置内容校验的非空文档或 Canvas
- THEN 组件 SHALL 可创建并进入 Error，错误为 ThorVG support is disabled in this QCatUiKit build。

#### Scenario: software 后端
- WHEN 已启用 ThorVG 且 Loading 组件关联使用 software 后端的窗口
- THEN 系统 SHALL 进入 Error，错误为 ThorVG QRhi requires a Qt Quick graphics backend。

#### Scenario: 等待渲染
- WHEN 内容通过加载前检查但尚无渲染结果
- THEN 系统 SHALL 保持 Loading，SHALL NOT 因文件读取成功直接进入 Ready。

#### Scenario: 绘制成功
- WHEN 当前内容成功记录绘制命令
- THEN 系统 SHALL 进入 Ready 并清空 errorString。

#### Scenario: 解码失败
- WHEN 文档无法解码
- THEN 系统 SHALL 进入 Error，错误为 Cannot decode SVG/Lottie data。

#### Scenario: 无效尺寸
- WHEN 文档宽高非有限值或不大于零
- THEN 系统 SHALL 进入 Error，错误为 Vector source has invalid dimensions。

#### Scenario: 后端失败
- WHEN 初始化、场景构建、内容挂接或绘制失败
- THEN 系统 SHALL 进入 Error 并返回对应非空错误说明；绘制失败 SHALL 包含数值结果。

#### Scenario: 错误恢复
- WHEN Error 状态下替换为有效内容且图形环境可用
- THEN 系统 SHALL 重新加载并在绘制成功后进入 Ready。

#### Scenario: 旧结果返回
- WHEN 被新内容替代的旧内容返回加载反馈
- THEN 系统 SHALL NOT 用该反馈覆盖当前内容状态。

### Requirement: R05 Reload

系统 SHALL 在实际改变 source、sourceData、format 或调用 reload() 时重新加载；Canvas 的 shapes 和有效 viewBox 变化 SHALL 触发重新加载。

#### Scenario: 重载复位
- WHEN 重新加载内容
- THEN 系统 SHALL 清空源尺寸、时长和帧数，将进度和已完成循环次数归零，将 playing 和 paused 设为 false，并按 autoPlay 设置后续播放请求。

#### Scenario: 重复赋值
- WHEN 来源、格式或形状列表被赋予与当前相同的值
- THEN 系统 SHALL NOT 因该次赋值重新加载。

### Requirement: R06 Layout

系统 SHALL 支持居中的比例适配、比例覆盖和拉伸，并裁剪超出组件范围的绘制内容。

#### Scenario: 比例适配
- WHEN fillMode 为 PreserveAspectFit 且源和目标尺寸有效
- THEN 内容 SHALL 等比居中并完整位于组件范围内。

#### Scenario: 比例覆盖
- WHEN fillMode 为 PreserveAspectCrop 且源和目标尺寸有效
- THEN 内容 SHALL 等比居中覆盖组件，超出部分 SHALL 被裁剪。

#### Scenario: 拉伸
- WHEN fillMode 为 Stretch 且源和目标尺寸有效
- THEN contentRect SHALL 等于组件范围。

#### Scenario: 空尺寸
- WHEN 源或目标尺寸为空
- THEN contentRect SHALL 为空矩形。

#### Scenario: 元数据返回
- WHEN 当前内容返回有效源尺寸
- THEN 系统 SHALL 更新 sourceSize 和隐式尺寸；Canvas 源尺寸 SHALL 为 viewBox 宽高。

### Requirement: R07 PropertyValidation

系统 SHALL 忽略无效枚举、无效 viewBox、无效循环次数及无效速率；进度 SHALL 截断到有效范围。

#### Scenario: 枚举无效
- WHEN format 或 fillMode 超出其公开枚举范围
- THEN 系统 SHALL 保留原值。

#### Scenario: viewBox 无效
- WHEN viewBox 含非有限值或宽高不大于零
- THEN 系统 SHALL 保留原值。

#### Scenario: 进度越界
- WHEN progress 为有限数值且超出 [0,1]
- THEN 系统 SHALL 将其截断至 [0,1]。

#### Scenario: 进度非有限
- WHEN progress 为 NaN 或无穷值
- THEN 系统 SHALL 保留原值。

#### Scenario: 循环次数无效
- WHEN loops 不是 -1 且小于 1
- THEN 系统 SHALL 保留原值。

#### Scenario: 速率无效
- WHEN playbackRate 非有限、不大于零或大于 16
- THEN 系统 SHALL 保留原值。

### Requirement: R08 PlaybackControls

系统 SHALL 为 Lottie 提供播放、暂停、停止、重新播放和进度定位。

#### Scenario: 播放
- WHEN Ready 的 Lottie 时长大于零、帧数大于一且调用 play()
- THEN playing SHALL 为 true，paused SHALL 为 false。

#### Scenario: 加载中播放
- WHEN 对 Loading 的 Lottie 调用 play()
- THEN 系统 SHALL 预约播放，并在 Ready 后按动画元数据决定能否开始播放。

#### Scenario: 暂停
- WHEN 对 Lottie 调用 pause()
- THEN 系统 SHALL 取消播放请求，令 playing 为 false、paused 为 true，保留进度。

#### Scenario: 停止
- WHEN 调用 stop()
- THEN 系统 SHALL 清除播放请求，将 playing 和 paused 设为 false，将进度和已完成循环次数归零。

#### Scenario: 重新播放
- WHEN 对可播放的 Lottie 调用 restart()
- THEN 系统 SHALL 从零进度开始播放。

#### Scenario: 末尾播放
- WHEN Lottie 进度为一且调用 play()
- THEN 系统 SHALL 将进度和已完成循环次数归零后请求播放。

#### Scenario: 非动画控制
- WHEN 对 SVG 或 Canvas 调用 play() 或 pause()
- THEN 系统 SHALL NOT 改变播放状态。

#### Scenario: 帧定位
- WHEN 设置有效 progress
- THEN currentFrame SHALL 为 progress × max(0, frameCount − 1)。

#### Scenario: 自动播放设置
- WHEN 仅修改 autoPlay
- THEN 系统 SHALL 更新设置，SHALL NOT 立即启动或停止当前播放。

### Requirement: R09 AnimationTiming

系统 SHALL 按有效时长、播放速率和循环次数推进可播放的 Lottie。

#### Scenario: 正常推进
- WHEN 正在播放的 Ready Lottie 时长大于零、animationEnabled 为 true、组件可见且所属窗口已暴露
- THEN 系统 SHALL 按经过时间和播放速率推进进度。

#### Scenario: 推进被禁止
- WHEN animationEnabled 为 false、组件隐藏、没有窗口或窗口未暴露
- THEN 系统 SHALL 暂停自动推进，SHALL NOT 仅因此将 playing 改为 false；恢复时 SHALL NOT 补放隐藏期间的完整时间。

#### Scenario: 无限循环
- WHEN loops 为 -1 且到达循环末尾
- THEN 系统 SHALL 继续循环。

#### Scenario: 有限循环完成
- WHEN 已完成循环次数达到正整数 loops
- THEN 系统 SHALL 将进度保持为一、停止播放并发出一次 finished()。

#### Scenario: 非动画元数据
- WHEN 成功加载 SVG 或 Canvas
- THEN duration 和 frameCount SHALL 为零。

### Requirement: R10 ShapeManagement

系统 SHALL 支持按列表顺序绘制形状、整表赋值、校验追加和清空。

#### Scenario: 有效追加
- WHEN appendShape(map) 追加后的完整列表有效
- THEN 系统 SHALL 更新列表并返回 true。

#### Scenario: 无效追加
- WHEN 追加后的完整列表无效
- THEN appendShape(map) SHALL 返回 false 并保留原列表。

#### Scenario: 无效整表赋值
- WHEN Canvas 的 shapes 被赋予无效列表
- THEN 系统 SHALL 保存该属性值并进入 Error，返回校验说明。

#### Scenario: 清空
- WHEN 调用 clearShapes()
- THEN shapes SHALL 为空列表。

#### Scenario: 空画布
- WHEN 空形状列表在有效图形环境中加载
- THEN Canvas SHALL 允许渲染为空画布，SHALL NOT 将空列表视为无文档而进入 Null。

### Requirement: R11 ShapeValidation

系统 SHALL 校验形状、路径、样式和渐变的既有输入约束。

#### Scenario: 形状无效
- WHEN 超过 4096 个形状、类型不受支持、矩形或椭圆宽高不大于零，或参与校验的数值非有限或绝对值超过 10⁷
- THEN 系统 SHALL 拒绝该列表作为有效画布内容并提供校验说明。

#### Scenario: 路径无效
- WHEN 路径不含 1..65536 条命令、首命令不是 M、命令不是 M/L/C/Z 或参数数量不匹配
- THEN 系统 SHALL 拒绝该路径。

#### Scenario: 样式无效
- WHEN radius 或 strokeWidth 小于零、opacity 不在 [0,1]、fillRule 不为 nonzero 或 evenodd，或显式填充描边无效
- THEN 系统 SHALL 拒绝该形状。

#### Scenario: 渐变无效
- WHEN 渐变不是 linear 或 radial、必填坐标无效、径向半径不大于零、色标不在 2..256 项、偏移不在 [0,1] 或递减、或颜色无效
- THEN 系统 SHALL 拒绝该渐变。

#### Scenario: 默认样式
- WHEN 有效形状省略样式字段
- THEN 系统 SHALL 使用黑色填充、无描边、透明度一、nonzero 填充规则；存在描边但省略宽度时 SHALL 使用宽度一。

### Requirement: R12 NotificationsAndPresentation

系统 SHALL 暴露属性通知、元数据、进度、播放通知及累计绘制次数；包装组件 SHALL 提供主题化状态提示。

#### Scenario: 状态变化
- WHEN 状态或错误说明实际变化
- THEN 系统 SHALL 发出 statusChanged()；进入 Error 或 Null 时 SHALL 清除 playing 和 paused。

#### Scenario: 进度变化
- WHEN 进度实际变化
- THEN 系统 SHALL 发出 progressChanged()。

#### Scenario: 元数据变化
- WHEN 源尺寸、时长或帧数更新
- THEN 系统 SHALL 通过 metadataChanged() 通知调用方。

#### Scenario: 绘制计数
- WHEN 新的成功绘制计数反馈至对象
- THEN 系统 SHALL 更新 renderedFrames 并发出 renderedFramesChanged()；该计数 SHALL NOT 表示 GPU 完成或屏幕呈现次数。

#### Scenario: 加载与错误提示
- WHEN QML 包装组件处于 Loading 或 Error
- THEN 包装组件 SHALL 分别显示居中忙碌指示器或可换行的居中错误文本。

#### Scenario: 减少动效
- WHEN 包装组件保持默认绑定且主题启用 reducedMotion
- THEN animationEnabled SHALL 为 false。

#### Scenario: 可访问性
- WHEN 创建包装组件
- THEN 组件 SHALL 提供 Graphic 可访问性角色及对应的可翻译名称。

## Data Model

本模块无数据库实体或持久化表；公开运行时模型如下。

| 字段 | 类型 | 默认值及约束 |
| --- | --- | --- |
| source | QUrl | 空；本地、qrc 或相对地址 |
| sourceData | QByteArray | 空；非空时优先 |
| format | Format | Svg；另有 Lottie、Canvas |
| fillMode | FillMode | PreserveAspectFit；另有 Stretch、PreserveAspectCrop |
| viewBox | QRectF | (0,0,100,100)；有限且宽高为正 |
| shapes | QVariantList | 空；最多 4096 项 |
| status / errorString | Status / QString，只读 | Null / 空 |
| backendAvailable | bool，只读 | 由构建决定 |
| sourceSize / contentRect | QSizeF / QRectF，只读 | 初始为空 |
| duration / frameCount | qreal / int，只读 | 0 / 0；时长单位为秒 |
| progress / currentFrame | qreal / qreal | 0 / 0；后者只读 |
| autoPlay / animationEnabled | bool / bool | false / true；包装组件覆盖或绑定 |
| playing / paused | bool，只读 | false / false |
| loops / playbackRate | int / qreal | -1 / 1；循环为 -1 或正整数，速率 (0,16] |
| renderedFrames | quint64，只读 | 0；累计成功绘制记录次数 |

Shape 字段：type 为 rectangle、ellipse 或 path；矩形和椭圆的 x/y 默认零，width/height 必填且为正；rectangle 的 radius 默认零。path 的 commands 为列表，格式为 [M,x,y]、[L,x,y]、[C,x1,y1,x2,y2,x,y] 或 [Z]，命令名称以字符串提供。

通用样式字段：fill 默认黑色，stroke 默认无，strokeWidth 默认一，opacity 默认一，fillRule 默认 nonzero。数值字段 SHALL 满足既有范围约束；调用方 MAY 提供可转换为数值的值。

Gradient 字段：type 为 linear 时包含 x1/y1/x2/y2；type 为 radial 时包含 cx/cy/radius；stops 为包含 offset 和 color 的列表。offset SHALL 非递减并位于 [0,1]，color SHALL 为有效 Qt 颜色。

## Known Deviations / Technical Debt

- backendAvailable 为 true 不代表当前设备或图形环境可用；无 CPU 回退。
- 文件读取占用 GUI 线程，公开接口无读取进度、取消和加载超时能力。
- 未获得可渲染窗口时，Loading 无超时终止规则。
- Ready 和 renderedFrames 表示成功记录绘制，不证明 GPU 完成或用户已看到画面。
- 无效 shapes 整表赋值保留无效值并报错；无效追加保留旧值，两种入口的失败后属性不同。
- 数值字段接受可转换输入，不执行严格原始类型校验；未识别字段不触发统一拒绝。
- viewBox 无形状数值采用的 10⁷ 绝对值上限。
- autoPlay 修改不立即更新当前已记录的播放请求。
- playing 为 true 时，隐藏、窗口未暴露或禁用动画仍会停止进度推进。
- 对 Lottie 调用 pause() 不检查是否就绪或正在播放；空内容或错误状态下也能设置 paused 为 true。
- stop() 对所有格式执行复位，play() 和 pause() 仅对 Lottie 生效。
- reload() 不清零累计 renderedFrames，也不立即将旧隐式尺寸随 sourceSize 清空。
- 手动设置 progress 不清零已完成循环次数。
- 各数据和数量上限固定，公开接口不提供配置项。
- 本地 SVG 按源目录解析外部资源；qrc 和内存内容不提供等价的本地目录解析上下文。
- 错误使用文本而非独立错误码枚举，文件错误包含底层系统文本。
- QRhi 矢量支持为实验功能，格式覆盖受当前 ThorVG 及适配后端限制。

## Non-Goals

- 本模块 SHALL NOT 提供文档网络下载、业务网络请求或数据库访问。
- 本模块 SHALL NOT 提供脚本执行、SVG/Lottie 编辑器、保存或导出接口。
- 本模块 SHALL NOT 提供 CPU 渲染回退或全部 SVG/Lottie 特性兼容承诺。
- 本模块 SHALL NOT 负责 Gallery 导航、演示业务、应用鉴权、文件选择或资源下载管理。
- 本模块 SHALL NOT 提供超出既有形状、路径命令和渐变模型的通用绘图编辑能力。

## Assumptions

- 宿主 SHALL 提供已初始化的 Qt GUI、Qt Quick 和有效事件循环。
- QML 使用方 SHALL 能导入 QCatUiKit 及包装组件依赖。
- 实际渲染 SHALL 使用启用 ThorVG 支持的库及可用 QRhi 图形环境。
- 观察加载完成和动画推进 SHALL 提供可实际渲染的窗口。
- 调用方 SHALL 在对象所属线程操作公开属性和方法。
- 文件和 qrc 内容 SHALL 预先存在且对进程可读。
- 文档引用的外部资源 SHALL 由调用方以对应来源可解析的方式提供。
- 默认主题绑定 SHALL 能访问有效主题、tokens 和 reducedMotion。
- Qt、编译器和运行库 SHALL 与宿主应用兼容。
