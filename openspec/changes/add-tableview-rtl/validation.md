# CatTableView RTL 验证

日期：2026-11-20。环境：Windows 11、Qt 6.12.0 MSVC Debug、offscreen/software。

本轮完成任务 1.1-1.3、2.1-2.3、3.1-3.4、4.1、4.3 和 4.4：公开列反转代理、`CatTableView` 代理接入与映射方法、四区/表头镜像、初始右端滚动、垂直滚动条左侧、跨区选择、键盘跨区和表头左侧列宽手柄。

- `ColumnReverseProxyModel`：公开头 `QCatUiKit/ColumnReverseProxyModel`，进入三组头文件列表和 `MODELS.md`；映射、`setData`、表头、列插入/删除、空模型和非法索引由 C++ 单测覆盖。
- `ctest --test-dir <build> -R '^models$' --output-on-failure`：通过。
- `tests/auto/qml/data/tst_tablertl.qml` 覆盖：
  - LTR 下 `view.model === table.model`、映射方法恒等；
  - RTL 下 `view.model` 是代理，`mapToSource`/`mapFromSource` 往返；
  - 冻结 1/1 时主体在左、冻结列在右，表头 x 和列宽正确；
  - 初始 `contentX` 位于右端；
  - 垂直滚动条 x=0、跨内容区高度；
  - 冻结区与主体的选择索引都是代理索引，映射回源列正确；
  - `Right` 从主体进入右侧冻结列，`Left` 返回，`Home`/`End` 按视觉首尾；
  - 表头左边界手柄拖动改变冻结列宽，四区同步。
- `tst_tablefreeze.qml` 20 个用例通过，LTR 冻结行为未回归。
- `ctest --test-dir <build> -R '^qml_contracts$' --output-on-failure`：445 passed、0 failed、4 skipped。
- Gallery 数据页新增 `Right-to-left layout` 开关，中英文 TS 已补；`GalleryDataExamples::test_tableRtl` 验证开关、代理切换、主体 x 和冻结列右侧位置。完整 Gallery 契约 861 passed、0 failed、3 skipped（用生成的 `gallery_contracts-Debug.cmake` 绕过 CTest 的 300 秒超时）。
- `qmllint -I <Qt qml> -I <build>/qml src/uikit/qml/CatTableView.qml`：通过。
- `python tools/check_docs.py`、`git diff --check`、OpenSpec 严格校验：通过。

## 未覆盖

- 4.2 RTL 编辑写回：依赖 `add-tableview-customization` 的 `editDelegate`。
- 5.4 真实桌面/多屏/读屏检查；Gallery RTL 开关已加，但未做人工视觉确认。
- 水平滚动条的视觉镜像通过 `LayoutMirroring.enabled` 设置，尚未做像素级位置断言；需要 Gallery 视觉确认。
- 一次完整套件运行可能命中既有 `ChartContracts::test_touchCoincidentPointsAndData` 触摸时序偶发失败；该用例在表格测试之前执行，与本变更无依赖，重跑通过。
