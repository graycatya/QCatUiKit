# CatTableView 冻结窗格验证

日期：2026-11-20。环境：Windows 11、Qt 6.12.0 MSVC Debug、offscreen/software。

本轮完成任务 1.1-1.3、2.1、2.2 的选择／行高部分、2.4、3.1、3.4、4.1 和 4.2：四区骨架、`frozenColumnCount`/`frozenRowCount`、越界静默钳制、冻结全部时隐藏滚动条、主体驱动滚动同步、共享列宽表与表头调宽手柄、跨区选择、行高一致、表头分段、跨四区统一垂直滚动条、键盘跨区、`freezeTo`/`clearFreeze` 和 Gallery 冻结控件；默认 `0/0` 保持原单视图路径。

- `ctest --test-dir <build> -R '^qml_contracts$' --output-on-failure`：455 passed、0 failed、4 skipped。
- 新增 `tests/auto/qml/data/tst_tablefreeze.qml`，覆盖默认路径、四区骨架、provider 宽度、负值与超界钳制、运行时切换、模型收缩与清空、全冻结滚动条、横向／纵向滚动同步、冻结列与主体列调宽、禁用调宽、宽度跨模式保持、跨区选择、行高一致、表头分段、统一垂直滚动条、键盘跨区、间距、自定义 delegate 尺寸和 `freezeTo`/`clearFreeze`。
- 冻结分区与表头统一 `cellSpacing=1`，冻结宽高计入内部间距；`test_cellSpacing` 覆盖。
- 冻结模式进入时从单视图继承 delegate 隐式列宽/行高，并在首次冻结布局后补捕获未加载列；`test_customDelegateSizes` 覆盖 120×28 自定义 delegate。无法捕获时仍回退 140/`controlHeight`。
- 垂直滚动条改为跨四区的外部 `ScrollBar`；`test_verticalScrollBarSpansContent` 验证高度、位置和滑块比例与主体一致。
- 冻结四区启用 `keyNavigationEnabled` 并用 `Keys.priority: Keys.BeforeItem` 拦截方向键；`test_keyboardCrossPanes` 验证左右跨冻结边界、上下行移动和边界钳制。
- 编辑委托接入后，`tst_tableediting.qml` 验证冻结列与主体编辑切换、提交/取消关闭其他区编辑器；Qt 6.12 的 `QQuickTableView::edit()` 注解告警记为已知上游噪声。
- 非正式 10 万行探针（offscreen/software，120×28 delegate）：冻结/无冻结存活 delegate 约 242/247，未出现翻倍；修复后冻结主体 `contentH` 约 2,899,970、`contentW` 604 + 冻结列 120，与单视图 725 基本对齐。
- 正式压力冒烟 `tests/auto/qml/data/tst_tablestress.qml`：10 万行、4 列、冻结 1/1；断言 `view.rows=100000`、存活 delegate < 2000、100 次纵向滚动后仍 < 2000。单文件约 7 秒，随 `qml_contracts` 一起通过（448 passed）。
- Gallery 数据页新增冻结列/行 SpinBox、冻结首行首列、清除冻结和 RTL 布局开关，补齐中英文 TS；`GalleryDataExamples::test_tableFreezeControls` 和 `GalleryDataExamples::test_tableRtl` 通过。完整 Gallery 契约测试 861 passed、0 failed、3 skipped。
- Gallery 表格页启动冒烟 `qcat-gallery --smoke-test --page table`：退出 0；`COMPONENTS.md` 与 `VALIDATION.md` 已补充冻结 API 和回归入口。
- 注意：`gallery_contracts` 的 CTest 超时为 300 秒，而完整套件实际约 1060 秒，直接运行 CTest 会超时；本轮通过生成的 `gallery_contracts-Debug.cmake` 脚本运行并保留结果。该超时属于现有验证基础设施配置，不在本变更内修改。
- 冻结模式列宽由共享宽度表驱动，表头 6px 手柄调整后四区同步；`resizableColumns=false` 同时禁用手柄；切回无冻结时把宽度回写到单视图。
- 2026-11-20 决定移除表头冻结抓手：发现性差、与列宽手柄抢空间，且冻结数量通常是应用配置；冻结数量只由属性和 `freezeTo`/`clearFreeze` 控制，交互入口由应用自己的菜单或工具条提供。
- 一次套件运行在 `ChartContracts::test_touchCoincidentPointsAndData` 失败（缩放范围 10 而非 5），该用例在 `TableFreeze` 之前执行，与本次改动无依赖；重跑整套通过，记为触摸时序偶发，不宣称已修复。
- 首轮套件 423 passed、1 failed：`CatTableView {}` 无模型时 `ItemSelectionModel` 接收 undefined 并告警。将 `model` 默认值改为 `null` 后全部通过。
- `qmllint -I <Qt qml> -I <build>/qml src/uikit/qml/CatTableView.qml`：通过，无告警。
- 完整 Debug 构建：通过。首次构建因 PDB 损坏报 C2471，重命名旧 `vc140.pdb` 后增量构建成功。
- RTL 设计 spike 已完成，见 [rtl-spike.md](rtl-spike.md)：Qt Quick `TableView`/`HorizontalHeaderView` 不读取布局方向，真正的 RTL 需要列反转代理模型、索引映射、初始右端滚动、选择/编辑写回和键盘方向。2026-11-20 决策：公开 `ColumnReverseProxyModel`、`CatTableView` 同时暴露 `mapToSource/mapFromSource`、方法映射、RTL 垂直滚动条左侧、RTL 作为归档门槛；实现拆到 [add-tableview-rtl](../add-tableview-rtl/tasks.md)，冻结变更 3.3 待其归档后勾选。
- 未验证：编辑、真实桌面/多屏/读屏。编辑依赖 `add-tableview-customization` 的 `editDelegate`；无 provider 且无法捕获隐式尺寸时仍回退 140/`controlHeight`。
