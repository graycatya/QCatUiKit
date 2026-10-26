# models Specification

## Purpose

提供表格、消息和只读层次数据模型，向 Qt 视图暴露数据及标准模型通知。

该基线用于约束后续变更所涉及的现有对外行为，记录已核查的正常流程、输入边界和错误结果，不以模块名称代表全部接口均已逐项覆盖。

## Dependencies

- Qt Core、Gui、Qml 的模型接口。
- TreeModel 为 QML 辅助类型，其 C++ 实现不属于安装接口。

## Requirements

### Requirement: R01 TableColumns

TableModel SHALL 按 columns 顺序提供 display/edit 单元格。

#### Scenario: 无列追加

- WHEN columns 为空时 append(map)
- THEN 方法 SHALL 返回 false。

#### Scenario: 非法列配置

- WHEN columns 含空键或重复键
- THEN 系统 SHALL 保留原列配置。

#### Scenario: 改列

- WHEN 设置不同的有效列配置
- THEN 系统 SHALL 保留原行数据并重置模型。

#### Scenario: 表头回退

- WHEN headers 不足以覆盖当前列
- THEN 缺少的水平表头 SHALL 使用列键；垂直表头 SHALL 为行号加一。

### Requirement: R02 Mutation

TableModel 和 ChatModel SHALL 提供追加、更新、读取、删除和清空。

#### Scenario: 非法行

- WHEN 对越界行 update/remove/get
- THEN 更新或删除 SHALL 返回 false，get SHALL 返回空 map。

#### Scenario: 读取副本

- WHEN 读取 get(row) 后修改返回 map
- THEN 模型 SHALL NOT 因副本修改而改变。

#### Scenario: 清空

- WHEN 对非空模型 clear()
- THEN 系统 SHALL 清空记录并发送数量变化通知；TableModel SHALL 保留列配置。

#### Scenario: 空清空

- WHEN 对空模型 clear()
- THEN 系统 SHALL NOT 发出模型重置或数量变化通知。

### Requirement: R03 ChatFields

ChatModel SHALL 保存已知消息字段并校验 type/status。

#### Scenario: 缺省消息

- WHEN append({})
- THEN 系统 SHALL 创建空文本、Text、Pending 消息，并以当前 Unix 毫秒填充 timestamp。

#### Scenario: 未知字段

- WHEN 追加或更新包含未知字段的消息
- THEN 未知字段 SHALL NOT 加入消息记录。

#### Scenario: 非法类型状态

- WHEN type 无法转换为 0..3 或 status 无法转换为 0..2
- THEN 追加或更新 SHALL 返回 false，原消息 SHALL 保持不变。

### Requirement: R04 ModelNotifications

模型 SHALL 使用标准 Qt 插入、删除、重置和数据变化通知。

#### Scenario: 数量通知

- WHEN 成功追加或删除记录
- THEN 系统 SHALL 发出 countChanged。

#### Scenario: 内容更新

- WHEN 更新实际改变已存在消息字段
- THEN 系统 SHALL 发出对应角色的 dataChanged，SHALL NOT 发出 countChanged。

### Requirement: R05 TreeSnapshot

TreeModel SHALL 将 items 转换为不可编辑的层次节点。

#### Scenario: 树节点

- WHEN items 包含字符串或 text/enabled/children 对象
- THEN 系统 SHALL 显示对应文本、启用状态与层次。

#### Scenario: 替换快照

- WHEN 设置不同的 items 列表
- THEN 系统 SHALL 重建树并发出 itemsChanged。

## Data Model

TableModel：columns/headers 为 QStringList，初始空；行记录为 QVariantMap，count 初始 0。

ChatModel：messageId、username、sender、recipient、content、senderAvatar、recipientAvatar 默认空字符串；timestamp 默认当前 Unix 毫秒；type 为 Text=0/Image=1/Audio=2/Video=3；status 为 Pending=0/Sent=1/Failed=2。

TreeModel：items 为 QVariantList，默认空；节点对象包含 text、可选 enabled 和 children。

## Known Deviations / Technical Debt

- TableModel 行 map 可保存未出现在 columns 的字段。
- ChatModel 仅对 type/status 作范围转换检查，其他字段无业务格式校验。
- 模型无自动容量限制和淘汰策略；TreeModel 原地修改数组成员不是增量编辑入口。

## Non-Goals

- 本模块 SHALL NOT 负责网络、数据库、持久化、消息发送确认、选择、列宽或滚动。

## Assumptions

- 模型 SHALL 在 QObject 所属线程操作；调用方 SHALL 控制数据容量。
