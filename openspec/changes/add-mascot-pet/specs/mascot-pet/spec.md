# Spec Delta

## Purpose

为宿主应用提供外部事件驱动的桌宠角色展示契约，统一精灵图与 Lottie 角色包、动作调度、气泡和交互通知。宿主负责业务服务与存储，组件负责可观察的表现状态，不接入 Agent 或音频服务。

## ADDED Requirements

### Requirement: R01 LocalCharacterPackage

系统 SHALL 加载本地目录或 qrc 中的版本化声明式角色清单；每包 SHALL 固定选择精灵图或 Lottie，声明默认动作、逻辑尺寸、锚点及资源。系统 SHALL NOT 下载网络资源、导入压缩包或执行包内脚本。

#### Scenario: 两类角色包
- WHEN 分别加载有效的精灵图包和 Lottie 包且对应渲染能力可用
- THEN 系统 SHALL 通过相同的动作与状态接口提供角色展示。

#### Scenario: 无效角色包
- WHEN 版本不支持、资源缺失、尺寸无效、默认动作缺失或资源引用越出角色包边界
- THEN 系统 SHALL 报告可定位的加载错误，SHALL NOT 将该包报告为 Ready。

### Requirement: R02 RenderingAndLifecycle

系统 SHALL 暴露 Null、Loading、Ready、Error 和错误信息。隐藏或减少动效时 SHALL 停止动画时钟推进，恢复时 SHALL NOT 补放隐藏时间；渲染后端错误 SHALL 向宿主报告，SHALL NOT 被静态占位掩盖。

#### Scenario: 后端不可用
- WHEN Lottie 包无法在当前图形环境渲染
- THEN 系统 SHALL 报告 Error 并显示包内可用静态占位或组件默认占位，SHALL NOT 假报动作完成。

#### Scenario: 动作切换不叠加通用加载圈
- WHEN 桌宠已有可用画面并切换到缓存或后台准备的 Lottie 动作
- THEN 系统 SHALL 保留旧画面至新场景可用，SHALL NOT 叠加通用 Loading 指示圈；错误和加载状态 SHALL 继续向宿主报告。

#### Scenario: 隐藏恢复
- WHEN 正在播放的实例隐藏后再次显示
- THEN 动画 SHALL 从冻结位置继续，SHALL NOT 因隐藏时间跳到末尾。

### Requirement: R03 ActionRequests

系统 SHALL 提供命名动作请求、暂停、恢复和恢复默认动作接口，支持替换、FIFO 排队、忙时忽略策略。请求 SHALL 有可关联标识与明确结果；无限默认动作 SHALL NOT 阻塞排队请求。外部接口 SHALL NOT 提供连续口型参数或调用业务服务。

#### Scenario: 动作自然结束
- WHEN 有限动作自然完成且存在待执行请求
- THEN 系统 SHALL 对完成请求通知一次并启动队首动作；队列为空时 SHALL 回到默认动作。

#### Scenario: 替换和忽略
- WHEN 忙碌时分别提交替换请求或忙时忽略请求
- THEN 替换 SHALL 打断当前请求并取消旧队列后启动新请求；忽略 SHALL 保留当前动作和队列并向调用方报告忽略结果。

#### Scenario: 无效动作
- WHEN 请求不存在的动作
- THEN 系统 SHALL 报告拒绝原因并保留当前有效动作，SHALL NOT 悄悄换成其他动作。

### Requirement: R04 DragFeedback

系统 SHALL 支持配置拖拽反馈动作。启用时拖拽 SHALL 临时抢占当前动作；结束后 SHALL 回到默认动作，SHALL NOT 自动恢复被打断动作。

#### Scenario: 拖拽抢占
- WHEN 启用有效拖拽反馈动作并开始拖拽
- THEN 系统 SHALL 通知当前请求被打断、取消旧队列并播放反馈；拖拽期间的新动作请求 SHALL 明确被拒绝为交互忙碌；结束后 SHALL 回到默认动作。

#### Scenario: 不启用反馈
- WHEN 未启用拖拽反馈动作而用户拖动窗口
- THEN 系统 SHALL 保持动作调度并发出拖拽事件。

### Requirement: R05 RegionsFacingAndScale

角色包 SHALL 能声明矩形、椭圆、多边形命名区域及点击／拖拽用途，并按动作覆盖。系统 SHALL 支持左右朝向和正有限统一缩放，交互区域与锚点 SHALL 同步变换；仅在包允许时 SHALL 使用镜像回退。

#### Scenario: 命名区域点击
- WHEN 用户点击当前动作中声明的交互区域
- THEN 系统 SHALL 向宿主发送区域标识及角色逻辑坐标；重叠区域 SHALL 按清单后声明者优先命中。

#### Scenario: 精灵连续播放稳定
- WHEN PNG 图集按声明帧率播放，包含非整数缩放或镜像
- THEN 系统 SHALL 只采样当前帧，SHALL NOT 混入相邻帧或因帧切换改变窗口位置与尺寸；缩放采样 SHALL 可配置。

#### Scenario: 朝向素材缺失
- WHEN 请求的朝向没有独立素材且包不允许镜像
- THEN 系统 SHALL 保留原朝向并报告该请求不可用。

### Requirement: R06 TextBubble

系统 SHALL 提供纯文本气泡的显示、替换、增量追加、关闭、可选操作按钮和自动关闭时限；按钮 SHALL 仅发送标识事件。气泡 SHALL 支持手动关闭与屏幕边缘避让，SHALL NOT 解释 Markdown、执行链接或播放语音。

#### Scenario: 增量文本和按钮
- WHEN 宿主追加包含标记字符的文本并设置按钮
- THEN 气泡 SHALL 按纯文本展示；点击按钮 SHALL 发送对应标识，SHALL NOT 自行执行业务操作。

#### Scenario: 超时或手动关闭
- WHEN 到达设置的关闭时限或用户点击关闭
- THEN 气泡 SHALL 关闭并通知关闭原因；过期气泡的定时器 SHALL NOT 关闭新气泡。

### Requirement: R07 IndependentInstances

系统 SHALL 支持多个独立实例，每个实例 SHALL 独立维护角色、窗口、队列和气泡；系统 SHALL NOT 建立自主行为或实例间互动规则。

#### Scenario: 实例隔离
- WHEN 两个实例播放不同动作且其中一个切换角色或销毁
- THEN 另一个实例的角色、动作队列和气泡 SHALL 保持不变。

### Requirement: R08 StateTransfer

系统 SHALL 提供版本化状态导出／恢复，覆盖位置、屏幕标识、缩放、朝向、置顶与穿透配置。宿主 SHALL 负责存储；恢复 SHALL 校正失效屏幕位置，SHALL NOT 自动读写持久化存储或恢复业务请求队列。

#### Scenario: 屏幕已移除
- WHEN 恢复状态引用的屏幕不存在
- THEN 实例 SHALL 移至当前可用屏幕内并报告校正结果。

#### Scenario: 状态无效
- WHEN 状态版本不支持或字段含无效值
- THEN 系统 SHALL 拒绝恢复并报告原因，SHALL NOT 部分修改当前有效状态。

### Requirement: R09 HostServiceBoundary

系统 SHALL 通过动作、气泡及交互事件与宿主协作，SHALL NOT 实现 Agent 接入、TTS、音频采集、分析或播放。宿主 SHALL 决定实例创建与销毁、开机自启、应用启停、系统集成和持久化；组件 SHALL NOT 管理桌面扩展。

#### Scenario: 外部说话事件
- WHEN 宿主收到第三方说话开始和结束事件并调用动作接口
- THEN 系统 SHALL 仅执行对应表现请求，SHALL NOT 发起服务请求或操作音频设备。

### Requirement: R10 AsynchronousResourceLoading

角色清单与完整资源校验、Lottie 文档读取和 CPU 场景准备 SHALL 在后台执行，GUI 线程 SHALL 能在这些阶段继续处理事件。QRhi 资源创建与绘制 SHALL 保留在所属渲染线程。异步加载 SHALL 保留原有来源边界和资源限制，SHALL NOT 通过跳过动作校验换取响应性。

#### Scenario: LargePackageLoading
- WHEN 加载大型本地或 qrc 角色包
- THEN 状态 SHALL 保持 Loading 直至本代资源可用；完整校验期间 GUI 事件 SHALL 可以被处理，Lottie SHALL 在实际首帧绘制后报告 Ready。

#### Scenario: PreserveAvailableVisual
- WHEN 提供角色视觉且已有可用画面时加载新包或新动作
- THEN 视觉 SHALL 在 Loading 期间保留先前画面及其逻辑尺寸，新场景可用后再切换；保留画面 SHALL NOT 被当作新包的 Ready 或可交互状态。清空来源或加载失败 SHALL 继续遵循 Null／Error 及占位契约。

#### Scenario: CancelOrDestroyWhileLoading
- WHEN 加载期间再次换源或销毁 UI 对象
- THEN 系统 SHALL 取消未开始或可协作中断的工作，忽略旧代次的状态和元数据；单个 UI 对象的销毁 SHALL NOT 等待解析完成。已经进入的单次解析可完成后再丢弃，应用退出阶段可等待任务安全收尾。

#### Scenario: ReloadSameActionPath
- WHEN 发起新一代包加载或显式资源重载，动作路径相同而内容已经改变
- THEN 系统 SHALL 加载本代内容，SHALL NOT 把为了保留画面而持有的旧动画误当作新加载结果。

### Requirement: R11 ActionSceneCache

桌宠 Lottie 渲染 SHALL 提供每实例有界的动作场景缓存及常用动作后台预热。系统 SHALL 区分缓存准入的源文档成本和实际内存，SHALL NOT 把文件大小声明为解码内存上限。普通 VectorItem SHALL 保持默认按源加载语义。

预算 SHALL 通过 actionCacheLimitMiB 配置，默认 150 MiB（每 MiB 为 1048576 字节），SHALL NOT 固定限制动作条目数。预算 SHALL 计入当前动作及非活动缓存的不同来源文档成本；当前动作超出预算时 SHALL 仍可播放，但 SHALL NOT 保留额外场景。

#### Scenario: ConfigurableCapacity
- WHEN 宿主配置预算为 0、负值或运行中缩小预算
- THEN 0 SHALL 关闭缓存与预热，负值 SHALL 被原子拒绝；缩小预算 SHALL 在后续渲染同步中按最近使用淘汰且不打断当前播放。

#### Scenario: AllActionsFit
- WHEN 全部动作来源文档成本之和不超过预算，且可见视觉保持有效
- THEN 系统 SHALL 在优先常用动作后逐个后台预热其余动作；暂停播放时钟 SHALL NOT 阻止预热推进。完成后跨动作切换 SHALL 复用场景而不重复 CPU 解析。

#### Scenario: ReuseAndPrewarm
- WHEN 请求已缓存或已完成预热的动作
- THEN 系统 SHALL 复用该实例已有的独占场景，跳过重复 CPU 解析；首次渲染仍 SHALL 经实际绘制后反馈 Ready。

#### Scenario: CacheLifetime
- WHEN 超出缓存预算、切包、显式重载、销毁或 QRhi 更换
- THEN 系统 SHALL 淘汰或失效相应缓存并安全释放资源，旧代任务 SHALL NOT 发布到当前包。

#### Scenario: SwitchLatency
- WHEN 诊断日志启用并执行动作切换
- THEN 系统 SHALL 报告缓存命中类型和动作执行开始到目标首帧渲染反馈的延迟，并说明测量边界；SHALL NOT 把解码完成等同于画面已显示。
