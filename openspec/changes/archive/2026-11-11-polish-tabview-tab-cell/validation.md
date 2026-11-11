# Validation

执行环境：Linux x86_64，Qt 6.12.0 gcc_64，Release 单配置，Ninja，`build-release`。

## 自动测试

```sh
cmake --build build-release --parallel
ctest --test-dir build-release --output-on-failure
```

结果：**21/21 通过**，总耗时 233.15 s。

相关套件：`qml_contracts` 含重写后的页签用例，覆盖标题切换、草稿保留、关闭信号参数，以及"点击关闭不改变当前页签"的新断言。

## 静态检查

```sh
cmake --build build-release --target QCatUiKit_qmllint
cmake --build build-release --target qcat-gallery_qmllint
python tools/check_docs.py
git diff --check
openspec validate polish-tabview-tab-cell --strict --no-interactive
```

全部通过。

## 复核发现的问题

首轮实现后，从 Gallery 页签示例截图复核发现关闭符号比页签标题高约 6 逻辑像素。

原因：`Row` 按顶部对齐子项，关闭目标只有 20 逻辑像素高，而所在行高由高 32 的标题按钮决定，于是关闭目标落在行的顶部。实测标题内容中心 16、关闭内容中心 10。

修正后关闭目标撑满单元格高度（32），20 像素悬停面改为在背景内居中绘制；实测标题内容中心 16、关闭内容中心 16、单元格中心 16，三者一致。

回归断言已加入 `test_tabsPreserveStateAndRequestClose`。为确认断言有效，临时把关闭目标改回 20 逻辑像素高后重跑，该用例在 `tst_extendedcontrols.qml` 的 `compare` 处失败，随后恢复实现并重跑通过。

## 视觉检查

离屏软件后端渲染 `CatTabView`（520×240，两个中文标题，`tabsClosable` 与 `addTabButtonVisible` 开启）：

- 亮色：两个页签各有独立圆角背景与边框，关闭符号位于边框内且与标题垂直居中；选中页签为浅蓝背景加主色边框。
- 暗色：同上，未选中页签为 `surface` 背景，选中页签为 `surfaceHover` 加主色边框。
- 关闭悬停：关闭目标显示居中的圆角危险色方块与浅色叉号，仍完全落在页签边框内；页签背景转为 `surfaceHover`，选中状态未改变。

采集限制：截图来自组件单独渲染，不是完整 Gallery 页面；未在原生图形后端、macOS 或 Windows 复核；未验证触摸密度与 RTL 下的页签排布。

## 已知变化

- 页签宽度改为按内容计算，不再沿用标题按钮的最小 80 逻辑像素，短标题的页签比改动前更窄。
- 使用绝对坐标点击页签的外部测试需要改为按内容或 `objectName` 定位；本仓库内的用例已同步。

## 未覆盖

- 自定义页签 delegate 不在本次范围。
- 页签过多时的横向滚动或换行行为沿用 `Flow`，未在本轮单独验证。

## 归档核查（2026-11-11）

差异规约已同步至对应主规格，并逐条核对需求与场景。主规格严格校验 14/14 通过，`python tools/check_docs.py` 与 `git diff --check` 通过。本次仅同步规约及归档，保留此前验证证据和未覆盖范围，未重新执行产品测试。
