# Tasks

## 1. 主题 Token

- [x] 1.1 `colorTokenNames` 增加 `surfacePressed`、`surfaceSelected`、`focusRing`。
- [x] 1.2 内置 Light/Dark 色板增加三个状态色默认值。
- [x] 1.3 `disabledOpacity` 作为 0–100 整数 token 加入 tokens 与 `numericTokenRange`。
- [x] 1.4 `focusRing` 在 `colorPalette` 和 `overrides` 之后按最终 `primary` 派生，显式值优先。
- [x] 1.5 C++ 主题测试覆盖新 token 的默认值、覆盖、派生跟随和范围钳制。

## 2. 组件状态映射

- [x] 2.1 `SurfaceImpl`、`CatTextField`、`CatTextArea` 的焦点描边改用 `focusRing`，禁用透明度改用 `disabledOpacity / 100`。
- [x] 2.2 `CatButton`、`CatCheckBox`、`CatRadioButton`、`CatSwitch` 拆分按下、选中和悬停状态。
- [x] 2.3 `CatComboBox` 打开态和列表高亮改用 `surfaceSelected`，移除组件私有高亮色；`CatSpinBox` 指示器改用 `surfaceSelected` / `surfacePressed`，移除私有步进色。
- [x] 2.4 `CatMenuItem`、`CatMenuBarItem`、`TabCellImpl`、`CatTreeView`、`CatScrollBar` 的按下、选中和高亮改用新 token。
- [x] 2.5 `CatTableCell` 选中默认值改用 `surfaceSelected`。
- [x] 2.6 `CatTitleBar`、`CatRatingControl`、`DockTabButton`、`DockTabStrip` 的焦点描边改用 `focusRing`。
- [x] 2.7 确认主色按钮和主色系开关仍使用 `primary` / `primaryHover` / `primaryPressed`。

## 3. QML 测试

- [x] 3.1 断言按下状态读取 `surfacePressed`，选中和高亮读取 `surfaceSelected`，悬停读取 `surfaceHover`。
- [x] 3.2 断言焦点描边读取 `focusRing`。
- [x] 3.3 断言自定义 `ThemePalette` 覆盖状态 token 后控件生效。
- [x] 3.4 更新因页签选中色变化而受影响的既有断言。

## 4. 文档与验证

- [x] 4.1 更新 `THEMING.md` 的 token 列表、状态映射和 `disabledOpacity` 说明。
- [x] 4.2 `cmake --build build-release --parallel` 通过。
- [x] 4.3 `ctest --test-dir build-release --output-on-failure` 通过。
- [x] 4.4 `qmllint` 修改文件、`python tools/check_docs.py`、`git diff --check`、`openspec validate add-state-style-tokens --strict --no-interactive` 通过。
- [x] 4.5 在 `validation.md` 记录命令、结果、视觉检查和未覆盖平台。
