# Tasks

## 1. 结构与属性

- [x] 1.1 在 `CatTableView` 中实现四区布局、零尺寸 provider 包装和 `frozenColumnCount`/`frozenRowCount` 属性，默认 `0/0` 时走现有单视图路径；用 QML 测试验证无冻结布局不变。
- [x] 1.2 实现越界静默钳制、模型减少自动收敛、冻结全部时隐藏对应滚动条；用 QML 测试验证负值、超界和模型重置。
- [x] 1.3 实现主体驱动、左侧/顶部跟随的滚动同步与范围钳制；用 QML 测试验证横向和纵向滚动互不串扰。

## 2. 共享状态与交互

- [x] 2.1 建立共享列宽表并实现冻结模式下的列宽调整手柄，四区 provider 读取同一宽度；用 QML 测试验证冻结区与主体同步调宽。
- [x] 2.2 共享 `selectionModel` 和行高结果；用 QML 测试验证跨区选择和同一行在四区高度一致。
- [ ] 2.3 待 `add-tableview-customization` 提供 `editDelegate` 后，验证冻结四区共享编辑态，并在提交或取消时关闭其他区编辑器。
- [x] 2.4 实现方向键跨冻结边界的焦点转移和可见性定位；用 QML 测试验证四个方向的边界导航。

## 3. 表头与抓手

- [x] 3.1 将表头拆为冻结段和主体段并保持列宽、文本同步；用 QML 测试验证横向滚动时冻结段不动、主体段跟随。
- 3.2 已取消（2026-11-20）：表头拖拽抓手经确认鸡肋，冻结数量改由属性和 `freezeTo`/`clearFreeze` 控制。
- [ ] 3.3 实现 RTL 镜像下的冻结侧和滚动条位置；用 QML 测试或 Gallery RTL 检查验证。实现由 [add-tableview-rtl](../add-tableview-rtl/tasks.md) 承担，待该变更归档后再勾选；背景见 [rtl-spike.md](rtl-spike.md)。
- [x] 3.4 将垂直滚动条改为跨四区统一，隐藏主体视图自带滚动条；用 QML 测试验证滚动范围和位置。

## 4. API 与演示

- [x] 4.1 实现 `freezeTo(row, column)` 与 `clearFreeze()` 并保持方法即属性写入；用 QML 测试验证与属性赋值等价、后写覆盖。
- [x] 4.2 在 Gallery 数据页增加冻结列/行控件和说明，补齐中英文 TS；用 Gallery 测试验证页面装配和语言切换。
- [x] 4.3 更新 `COMPONENTS.md` 与 `VALIDATION.md`，并在 `src/uikit/CMakeLists.txt` 登记新增 QML 文件（本轮无新增文件）。

## 5. 回归与验收

- [x] 5.1 运行 QML 行为测试、库与 Gallery qmllint、`python tools/check_docs.py` 和 `git diff --check`，记录结果。
- [ ] 5.2 在现有 10 万行压力演示开启冻结做冒烟，确认无滚动卡顿或 delegate 异常，并记录未覆盖的真实桌面、多屏和读屏场景。
