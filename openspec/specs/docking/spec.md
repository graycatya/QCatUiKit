# docking Specification

## Purpose

管理面板注册、标签分组、分割、浮动、回停及布局快照恢复。

该基线用于约束后续变更所涉及的现有对外行为，记录已核查的正常流程、输入边界和错误结果，不以模块名称代表全部接口均已逐项覆盖。

## Dependencies

- Qt Core、Gui、Qml、Quick。
- CatDockWorkspace、CatDockPanel；原生拖动依赖平台能力，Wayland 协议探测为可选适配。

## Requirements

### Requirement: R01 PanelRegistry

控制器 SHALL 使用稳定面板 ID 管理注册和打开状态。

#### Scenario: 注册

- WHEN 注册非空且未使用的 ID
- THEN 方法 SHALL 返回 true，将面板加入首个主组并激活。

#### Scenario: 注册拒绝

- WHEN 注册空白或重复 ID，或注销未知 ID
- THEN 方法 SHALL 返回 false 并发出 operationRejected。

#### Scenario: 关闭打开

- WHEN 关闭已打开面板后再 openPanel(id)
- THEN 面板 SHALL 回到首个主组并激活，SHALL NOT 自动恢复关闭前位置。

### Requirement: R02 DockOperations

系统 SHALL 区分组内停靠与整个工作区外侧停靠。

#### Scenario: 组内停靠

- WHEN 有效 dock 请求使用 Center 或四方向位置
- THEN 面板 SHALL 合并到目标组或在目标组旁分割。

#### Scenario: 外侧停靠

- WHEN 有效 dockToWorkspace 请求使用四方向位置和有限 0<ratio<1
- THEN 面板 SHALL 位于整个主布局外侧，ratio SHALL 表示左侧或上侧占比。

#### Scenario: 浮动目标

- WHEN 向浮动组请求非 Center 停靠
- THEN 方法 SHALL 返回 false 并保持布局。

#### Scenario: 无效操作

- WHEN 源或目标无效、自身停靠、非法位置、标签索引或比例
- THEN 控制器 SHALL 拒绝对应操作并发出 operationRejected。

### Requirement: R03 LayoutPersistence

系统 SHALL 输出版本 2 JSON，并事务式读取版本 1 和 2。

#### Scenario: 有效恢复

- WHEN 面板已注册且快照有效
- THEN 系统 SHALL 恢复布局及版本 2 浮动信息，忽略未知面板并保持快照缺少的已注册面板关闭。

#### Scenario: 恢复失败

- WHEN JSON、版本、节点、比例或唯一性校验失败
- THEN 系统 SHALL 返回 false 并保持原布局。

#### Scenario: 旧版本

- WHEN 恢复有效版本 1 布局
- THEN 系统 SHALL 按无浮动窗口处理。

#### Scenario: 容量限制

- WHEN 布局字符串超过 1 Mi 个字符或树超过深度 64、节点数 4096 的限制
- THEN 系统 SHALL 拒绝恢复。

### Requirement: R04 ContentAndPermissions

工作区 SHALL 管理面板呈现并保留内容实例。

#### Scenario: 内容迁移

- WHEN 面板在主工作区和浮动宿主间迁移
- THEN 系统 SHALL 保留面板与业务内容实例。

#### Scenario: 关闭请求

- WHEN 用户请求关闭面板
- THEN 系统 SHALL 发出 closeRequested，由应用调用 closePanel 接受。

#### Scenario: 界面锁定

- WHEN layoutLocked 启用
- THEN 界面 SHALL 阻止用户布局操作，直接控制器操作 SHALL 不受此界面锁限制。

#### Scenario: 拖动取消

- WHEN 用户取消尚未提交的拖动
- THEN 系统 SHALL 保留原布局。

## Data Model

布局文档：version、root、floating。节点：id、kind；split 包含 orientation、ratio、first、second；tabs 包含 tabs 和 active。浮动项包含 geometry 与回停提示。面板 ID 为非空唯一字符串；面板默认 minimumDockWidth=120、minimumDockHeight=80，closable/movable/floatable=true；layoutLocked=false。

## Known Deviations / Technical Debt

- 浮动宿主只容纳标签组，不能内部四方向分割。
- 控制器不检查面板 closable/movable/floatable，界面入口与控制器权限不同。
- 布局快照不保存业务内容或窗口最大化/最小化状态。
- Wayland 交互依赖合成器协议能力及 Qt 内部桥接，平台路径不同。

## Non-Goals

- 本模块 SHALL NOT 跨独立工作区拖动面板、提供边缘自动隐藏或保存业务数据。

## Assumptions

- 操作 SHALL 在对象所属线程执行；恢复 SHALL 在面板注册完成后进行；面板 ID SHALL 在注册期间保持稳定。
