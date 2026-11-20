# Tasks

## 1. 代理模型

- [x] 1.1 新增公开 `ColumnReverseProxyModel`（`QAbstractProxyModel`）：列反转映射、`data`/`flags`/`headerData`/`setData`/`index`/`parent`、`mapToSource`/`mapFromSource`；列插入/删除/移动/重置通知。
- [x] 1.2 登记 `src/uikit/CMakeLists.txt` 的公开头、兼容头、类头三组列表；补 `MODELS.md` 和 ABI 审查（新增类为加法，ABI 1 不变）。
- [x] 1.3 新增 C++ 单测：映射往返、角色/表头、编辑写回、列结构变化、空模型、非法索引。

## 2. 表格接入

- [x] 2.1 `CatTableView` 读取 `LayoutMirroring.enabled`，RTL 下创建代理并让四区/共享选择模型绑定代理；LTR 保持源模型。
- [x] 2.2 新增 `mapToSource(index)` / `mapFromSource(index)`；LTR 恒等，RTL 转发代理；切换模型/方向时释放旧代理。
- [x] 2.3 用 QML 测试验证无 RTL 时 `view.model === table.model`，RTL 时 `view.model` 是代理且映射正确。

## 3. 几何与滚动

- [x] 3.1 按 RTL 重排四区：主体在左、冻结列在右，冻结行仍在顶部；表头段、列宽和间距同步。
- [x] 3.2 RTL 初始 `contentX` 定位右端；模型重置/列数变化时重新锚定，用户滚动后保持位置。
- [x] 3.3 RTL 下垂直滚动条移到左侧并跨四区；水平滚动条位于主体区并镜像位置/拖动方向；冻结全部时隐藏。
- [x] 3.4 用 QML 测试验证四区几何、表头段、滚动条位置和初始滚动。

## 4. 交互

- [x] 4.1 RTL 下共享选择模型使用代理索引，点击、整行高亮和 `currentIndex` 跨区一致；源索引经映射方法往返。
- [ ] 4.2 RTL 下编辑提交经代理写回源模型，取消/失败不写错列；跨区编辑器关闭与 LTR 一致。
- [x] 4.3 RTL 下方向键：`Right` 进入冻结区、`Left` 返回主体，`Home`/`End` 按视觉首尾；键盘导航测试覆盖四个边界。
- [x] 4.4 RTL 下把列宽手柄放到表头左边界并反转拖动方向，四区宽度同步；`resizableColumns=false` 禁用。

## 5. Gallery、文档与回归

- [ ] 5.1 Gallery 增加 RTL 语言/布局检查入口，表格页冻结控件在中英文下可用；补中英文文案。
- [x] 5.2 更新 `COMPONENTS.md`、`VALIDATION.md` 和 `rtl-spike.md` 的决策记录；说明 RTL 下 `view.model` 是代理。
- [x] 5.3 运行 QML 行为测试、代理 C++ 测试、库/Gallery qmllint、`python tools/check_docs.py`、`git diff --check` 和 OpenSpec 严格校验，写入 `validation.md`。
- [ ] 5.4 在 Gallery 表格页做 RTL 冒烟（无冻结和 1/1 冻结），确认初始右端、滚动条、选择、编辑、键盘和列宽手柄；记录未覆盖的真实桌面/多屏/读屏场景。
