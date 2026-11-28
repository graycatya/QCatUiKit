# Validation

执行环境：Windows 11 x64，Qt 6.12.0 msvc2022_64，Release 单配置，Ninja，MSVC 19.39，`build-led-release`。

## 修复

- `DockTabStrip.qml`：RTL 下 `leftPadding = closeButton.width + 4`，文字从关闭目标右侧开始。
- RTL 下 `cell.width` 不再叠加关闭目标宽度；LTR 保持原计算，避免标签间出现额外空隙。
- `tst_dock.qml::test_rtlTabStrip` 增加 `leftPadding >= closeButton.width + 4` 断言。

## 自动测试

```sh
cmake --build build-led-release --parallel
ctest --test-dir build-led-release -R "^(qml_contracts|gallery_contracts)$" --output-on-failure
```

结果：**2/2 通过**。`qml_contracts` 40.05 s，`gallery_contracts` 127.05 s。

## 静态检查

```sh
cmake --build build-led-release --target QCatUiKit_qmllint
python tools/check_docs.py
openspec validate fix-dock-rtl-tab-overlap --strict --no-interactive
git diff --check
```

均通过。

## 视觉检查

`qcat-gallery --page dock-workspace --rtl --language zh_CN --width 900 --screenshot <临时路径>`：

- 标签栏显示「× 图表预览」「编辑器」「× 属性」，关闭目标在左、文字在右，无重叠。
- 第二个标签「编辑器」位于最右侧，顺序仍为 RTL。
- 标签间无异常空隙，下划线宽度与标签宽度一致。
- 临时截图为一次性证据，已删除，未加入仓库。

## 未覆盖

- 未复核 LTR 下关闭目标与文字尾部的间距；本次未改 LTR 分支。
- 未截图验证浮动窗口与触摸密度下的 RTL 标签栏。
- 未覆盖 RTL 下的拖动换序预览坐标，沿用上一变更的记录。
