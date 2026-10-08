# 组件与公开契约

所有带主题的组件默认 `theme: Theme`。其 `tokens` 是解析视图。继承自 Qt 控件的类型保留其标准属性、信号和可访问性语义。未列出的内部 id、子对象结构以及 `SurfaceImpl` 不属于公开契约。

| 组件 | 主要 API / 行为 |
| --- | --- |
| CatButton | 标准 Button；`primary`, `danger`, `loading`, `iconSource`；loading 临时禁用并恢复 enabled；primary 决定填充，danger 决定危险边框/普通文字 |
| CatLabel | 标准 Label，纯文本默认；可显式选择其他 textFormat |
| CatPane | 标准 Pane，Token 边框、圆角、padding |
| CatTextField | 标准 TextField；`invalid` 控制错误边框；可配置 validator / inputMethodHints |
| CatIpAddressField | 单输入框 IPv4；使用 `acceptableInput` 判断可提交性；聚焦时允许中间输入状态 |
| CatToolTip | 标准 ToolTip，delay=600ms、timeout=3000ms；应用显式绑定 hovered 或其他可见条件 |
| CatPopup | 标准 Popup；默认模态、focus=true，Escape 或外部按下关闭 |
| CatDrawer | 标准 Drawer；默认左侧；触摸打开手势和位置属性采用 Qt 行为 |
| CatAlert | `text`, `severity`（0–3）, `closable`；关闭时隐藏自身并发出 `closed()` |
| CatProgressBar | 标准 ProgressBar；from/to/value 和 indeterminate；镜像布局从右向左填充 |
| CatCircularProgress | `from=0`, `to=100`, `value=0`, `strokeWidth=6`；`position` 钳制为 0–1，零跨度为 0 |
| CatBusyIndicator | `running`；不可见或减少动效时停止转动 |
| CatPagination | `total`, `pageSize`, `currentPage`, `pageCount`；`requestPage()` 钳制且仅实际变页发出 `pageRequested`；总数变小会收敛当前页但不发用户请求事件；空数据页数为 1 |
| CatCalendar | `selectedDate`, `month`（0–11）, `year`, `minimumDate`, `maximumDate`；`moveMonth()`、`selectDate()`；只接受范围内日期并发 `dateSelected` |
| CatTableView | `model`, `delegate`, `columnWidthProvider`, `rowHeightProvider`, `resizableColumns`；`view` 可配置标准 TableView 选择与滚动属性 |
| CatChatView | 标准 ListView；`currentUser`, `followTail=true`，delegate 可替换；新增消息默认滚动到底部 |
| CatChatInput | `text`, `placeholderText`，`submitted(string)`；忽略纯空白，提交后清空 |
| CatImage | 标准 Image；异步加载与等比填充，保留 sourceSize 等标准属性 |
| CatImageMotion | `source`, `sourceSize`, `running`, `duration`, `fromScale/toScale`, `fromOpacity/toOpacity`, `fromOffset/toOffset`；往返动画，应用选择移动／缩放／闪烁参数 |
| CatFlipView | 标准 Flipable；`flipped`，front/back 为调用方内容 |
| CatMarquee | `text`, `font`, `running`, `duration`；文本溢出才滚动 |
| CatTypewriter | `fullText`, `running`, `interval`, `revealed`, `restart()`；减少动效时直接显示全文 |
| CatDragHandle | `target`, `active`；为无 anchors 锁定位置的目标提供拖动区域 |
| CatResizeHandle | `target`, `minimumWidth/Height`, `maximumWidth/Height`；应用不要同时用 anchors 或 Layout 强制覆盖目标尺寸 |
| CatFpsCounter | `fps`；只表示 Quick 帧回调速率 |
| CatEffect | 标准 MultiEffect 的 source / mask / shadow；应用按 Qt MultiEffect 约定隐藏源项、设置纹理源，核心控件无效果依赖 |
| CatConicalGradient | `startColor`, `endColor`, `angle`；矩形区域锥形渐变 |

## 库身份

`LibraryInfo` 只读单例提供实际加载库的 `version`、`versionMajor/Minor/Patch`、`releaseChannel`、`isStable`、`isExperimental` 和 `abiVersion`。C++ 使用 `<QCatUiKit/Version.h>` 中的导出查询函数。详见 [版本管理](VERSIONING.md)。

## C++ / QML 模型

ChatModel 公开 role（从 `Qt::UserRole+1` 顺序分配）：`messageId`, `username`, `sender`, `recipient`, `timestamp`, `content`, `type`, `status`, `senderAvatar`, `recipientAvatar`。`type`：Text=0、Image=1、Audio=2、Video=3；`status`：Pending=0、Sent=1、Failed=2。缺省文本内容为空、时间戳为当前毫秒、状态 Pending。未知字段不加入消息；非法枚举使 append/update 返回 false。

ChatModel：`append(map)`, `update(row,map)`, `get(row)`, `remove(row)`, `clear()`, `count`。非法行返回 false/空 map，不发虚假的增删通知；update 只通知实际修改角色，clear 对空模型不发 reset。

TableModel：`columns`, `headers`, `count`, `append(map)`, `update(row,map)`, `get(row)`, `setCell(row,column,value)`, `remove(row)`, `clear()`。列不能为空字符串且不可重复；非法配置被忽略，空列列表合法但不可 append。改变列顺序保留行数据并 reset 模型。表头不足时使用列键。角色为 Qt 标准 `display` 和 `edit`，支持标准 `setData(EditRole)`。

所有模型都保留 Qt 的 `dataChanged`、rowsInserted/rowsRemoved、modelReset 等标准信号。`countChanged` 仅在条目数量变化时发出。get 返回副本，不通过外部对象指针暴露 Impl。

## 平台交互

桌面可以 Tab 聚焦、Space 激活按钮；触摸密度使用更大的命中区域，并默认关闭 hover 样式。输入控件使用 Qt 原生文本编辑与输入法。组合布局使用 Qt 镜像机制；应用可在根 Item 设置 `LayoutMirroring.enabled` 和 `childrenInherit`。实际屏幕阅读器、输入法和平台窗口行为应在目标设备验收。
