# Validation

执行环境：Windows 11 x64，Qt 6.12.0 msvc2022_64，Release 单配置，Ninja，MSVC 19.39，`build-led-release`。

## 自动测试

```sh
cmake --build build-led-release --parallel
ctest --test-dir build-led-release -R "^(qml_contracts|gallery_contracts)$" --output-on-failure
```

结果：**2/2 通过**。`qml_contracts` 42.06 s，`gallery_contracts` 144.36 s。

新增用例 `tst_dock.qml::test_scrollIconOverride`：

- 220 像素宽、两个长标题时标签溢出，右侧滚动按钮出现。
- 默认 `dockScrollRightGlyph` 可见。
- 设置 `tabScrollLeftIcon` / `tabScrollRightIcon` 后默认绘制隐藏，`customDockScrollIcon` 存在，右侧按钮仍可用。
- 原 `test_overflowDragScrollsWhilePointerIsStill` 继续通过，说明按钮宽度、状态色和拖动自动滚动未受影响。

## 静态检查

```sh
cmake --build build-led-release --target QCatUiKit_qmllint
cmake --build build-led-release --target qcat-gallery_qmllint
python tools/check_docs.py
openspec validate add-dock-scroll-icons --strict --no-interactive
git diff --check
```

均通过。

## 视觉检查

原生图形会话运行 `qcat-gallery --page dock-workspace --width 480 --language zh_CN --screenshot <临时路径>`：

- 标签栏溢出时左右箭头显示 RemixIcon 的 chevron，位置和按钮宽度与之前一致。
- 标签、关闭按钮、浮动与回停按钮布局未变。
- 临时截图为一次性证据，已删除，未加入仓库。

## 未覆盖

- 未单独截图验证浮动窗口内的滚动箭头；该路径只做了属性传递，复用同一 `DockTabStrip`。
- 自定义图标不自动继承禁用与按下颜色；Gallery 示例固定使用 `text`。与 `CatTitleBar` 的覆盖契约一致。
- 未在 RTL、触摸密度、高 DPI、macOS 和 Linux 下复核。
