# 数据模型与对话

[文档索引](README.md) · [组件目录](COMPONENTS.md)

模型通过 `import QCatUiKit` 使用。TableModel、ChatModel 同时提供公开 C++ 类；TreeModel 是 QML 可用的层次模型辅助类型，其 C++ 实现不属于安装接口。

所有模型在其 QObject 所属线程操作。工作线程可准备数据，再通过 queued connection 提交给模型。模型只管理数据，不负责网络、数据库、文件持久化或视图尺寸。

## TableModel

`columns` 定义字段键和列顺序，`headers` 提供显示表头，`count` 返回行数。

```qml
import QtQuick
import QCatUiKit

Item {
    width: 600
    height: 320

    TableModel {
        id: rows
        columns: ["name", "state", "value"]
        headers: [qsTr("名称"), qsTr("状态"), qsTr("数值")]
        Component.onCompleted: {
            append({name: "Record 1", state: "Ready", value: 12})
            append({name: "Record 2", state: "Pending", value: 8})
        }
    }
    CatTableView {
        anchors.fill: parent
        model: rows
        resizableColumns: true
    }
}
```

| 方法 | 行为 |
| --- | --- |
| `append(map)` | 添加一行；列配置为空时返回 false |
| `update(row, changes)` | 合并指定行的字段 |
| `get(row)` | 返回行数据副本；非法行返回空 map |
| `setCell(row, column, value)` | 按当前列顺序修改单元格 |
| `remove(row)` | 删除指定行 |
| `clear()` | 清空所有行，保留列配置 |

索引从 0 开始。非法索引返回 false 或空结果，不发虚假的模型通知。列键不能为空或重复；改变列配置保留行值并重置模型。字段缺失时返回无效值，表头不足时使用列键。

TableModel 继承 QAbstractTableModel，提供标准 display／edit 角色和 `setData(EditRole)`。列宽、行高、选择和滚动由 CatTableView 或调用方视图控制，`view` 属性可访问其标准 TableView 配置。

## ChatModel

ChatModel 继承 QAbstractListModel，`count` 表示消息数。提供 `append(map)`、`update(row, changes)`、`get(row)`、`remove(row)` 和 `clear()`；索引行为与 TableModel 一致。

| 角色 | 含义 |
| --- | --- |
| `messageId` | 应用消息标识 |
| `username` | 显示名称 |
| `sender` / `recipient` | 发送方／接收方标识 |
| `timestamp` | 时间戳，默认当前 Unix 毫秒 |
| `content` | 消息内容，默认空文本 |
| `type` | Text=0、Image=1、Audio=2、Video=3 |
| `status` | Pending=0、Sent=1、Failed=2，默认 Pending |
| `senderAvatar` / `recipientAvatar` | 头像信息 |

角色从 `Qt::UserRole + 1` 按上表中各字段的顺序排列。未知字段不加入消息；非法 type 或 status 会使追加／更新失败。模型不自动发送消息或确认送达，状态由应用更新。

## 对话视图与输入

```qml
import QtQuick
import QtQuick.Layouts
import QCatUiKit

ColumnLayout {
    width: 480
    height: 480

    ChatModel { id: messages }

    CatChatView {
        Layout.fillWidth: true
        Layout.fillHeight: true
        model: messages
        currentUser: "me"
        followTail: true
    }
    CatChatInput {
        Layout.fillWidth: true
        onSubmitted: text => messages.append({
            sender: "me",
            username: qsTr("我"),
            content: text,
            type: ChatModel.Text,
            status: ChatModel.Pending
        })
    }
}
```

CatChatInput 的 `submit()` 由按钮或回车调用，也可由应用调用。提交先去除首尾空白；非空时发出 `submitted(string)` 并清空输入，纯空白不提交。

CatChatView 根据 sender 与 currentUser 的匹配决定消息方向。默认 delegate 展示正文、发送者和状态；图片、音频、视频消息以附件信息文字显示，需要媒体预览时由应用替换 delegate。

`followTail=true` 在消息数量变化后定位到末尾；阅读历史时可设为 false。需要立即跳到最新消息时调用标准 ListView 的 `positionViewAtEnd()`。滚动和定位 API 继承自 ListView。

## TreeModel

```qml
import QtQuick
import QCatUiKit

CatTreeView {
    width: 400
    height: 300
    model: TreeModel {
        items: [
            {text: "Workspace", children: ["Documents", "Images"]},
            {text: "Archive", enabled: false}
        ]
    }
}
```

`items` 接受字符串叶子，或带 `text`、`enabled`、`children` 的对象。替换整个 items 列表会重建模型；数组成员的原地修改不会作为增量模型编辑接口。节点用于只读展示、展开与选择。

## 通知、容量与性能

TableModel 和 ChatModel 使用标准 dataChanged、rowsInserted、rowsRemoved、modelReset 信号；仅数量变化时发 countChanged。get 返回值副本，修改副本后应调用相应更新方法。对空模型 clear 不触发无意义的重置。

模型本身没有 Gallery 的十万行／一万条容量限制，也没有自动淘汰策略。大量数据应由应用控制容量、分批提交并让出 GUI 线程。虚拟化减少可见 delegate 数量，原始记录仍占用内存；长文本、更新频率和删除位置都会影响开销。

[Gallery](../../../examples/uikit/gallery/README.md) 提供表格 100,000 行、对话和提交历史各 10,000 条的可停止持续测试，分别演示满容量更新、最旧消息淘汰及真实输入框提交。
