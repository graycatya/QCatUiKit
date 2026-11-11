# Tasks

## 1. 单元测试

- [x] 1.1 在 `tests/auto/models/tst_models.cpp` 增加色板用例：完整自定义色板、部分色板回退、空色板恢复内置值、非法颜色项回退、非颜色键忽略、色板不改变尺寸与动效。验证：`customColorPalette` 通过。
- [x] 1.2 改写 `themeContract` 中与新契约冲突的断言：未知键保留到 `tokens`、越界数值钳制到有效范围。验证：`models` 通过。
- [x] 1.3 增加派生用例：经 `overrides` 与 `colorPalette` 改变 `primary` 后 `onPrimary`、`primaryHover`、`primaryPressed` 重算；显式提供的派生值优先。验证：`derivedColorsFollowBrand` 通过。
- [x] 1.4 增加并存用例：两个 `ThemePalette` 实例各持不同色板，互不影响。验证：`customPaletteIsolation` 通过。
- [x] 1.5 确认既有语义未被破坏：`tokensChanged` 仅在值实际变化时发出、`reducedMotion` 优先于 `overrides`、seed 变化后缓存失效。验证：`themeContract`、`reducedMotionOverridesAndSignals` 通过。

## 2. 实现

- [x] 2.1 `qcatthemepalette.h` 增加 `colorPalette` 属性、访问器与 `colorPaletteChanged`。
- [x] 2.2 `qcatthemepalette.cpp` 按 design.md 的六步顺序改写 `tokens()`：内置色板 → 色板覆盖 → overrides → 派生重算 → 减少动效 → 冻结。
- [x] 2.3 无效输入输出 `qcatuikit.theme` 类别警告；同一输入集合下不重复输出（`reportedWarnings` 比较）。
- [x] 2.4 默认路径行为不变：空 `colorPalette`、空 `overrides` 时 token 取值与改动前一致。验证：`models` 断言 `controlHeight` 32/48/44、`surface` `#1f1f1f`、`onPrimary` 推导均通过。

## 3. QML 层验证

- [x] 3.1 `tst_controls.qml::test_colorPaletteIsolation`：同页两个 `CatButton` 传不同 `theme`，颜色、圆角、字号互相隔离，且不泄漏到默认 `Theme`。
- [x] 3.2 `tst_controls.qml::test_colorPaletteSwitchRefreshesControl`：运行中修改 `colorPalette`，已创建控件刷新；清空后回到内置色板。
- [x] 3.3 `tst_controls.qml::test_destroyedPaletteFallsBackToDefaultTheme`：局部 `ThemePalette` 销毁后 `theme` 变为 null，控件回退到 `Theme` 且不残留旧色。

## 4. Gallery

- [x] 4.1 设置页增加主题方案选择：亮色、暗色、自定义色板（Solarized 风格），切换后全站生效；`objectName` 为 `gallerySchemeLight`／`gallerySchemeDark`／`gallerySchemeCustom`。
- [x] 4.2 设置页新增"两套主题并排显示"分区（`galleryThemeDefaultPane`／`galleryThemeCustomPane`），右侧使用局部 `ThemePalette`，证明同一窗口可并存两种视觉。原计划的独立演示页改为设置页分区，避免新增导航路由而演示内容是主题机制而非组件。
- [x] 4.3 分区说明标注"只使用公开 API"；新增文案已加入 `qcatgallery_en.ts` 与 `qcatgallery_zh_CN.ts`。未新增 `GalleryCatalog` 条目：本变更没有引入新的 QML 类型，入口位于既有设置页。

## 5. 文档

- [x] 5.1 更新 `src/uikit/doc/THEMING.md`：新增"自定义色板"一节，说明 `colorPalette`、扩展键、派生规则与告警行为；修正原先"覆盖 primary 不会重新推导"的描述。
- [x] 5.2 更新 `src/uikit/doc/COMPONENTS.md` 的 Theme／ThemePalette 条目、`README.md`／`README.en.md` 的主题能力行、`skills/qcatuikit-ui/references/ui-patterns.md`。
- [x] 5.3 运行 `python tools/check_docs.py`：通过。

## 6. 集成检查

- [x] 6.1 Release 构建、`ctest`、`QCatUiKit_qmllint`、`qcat-gallery_qmllint` 通过。
- [x] 6.2 离屏渲染设置页并核对亮色与自定义色板两种状态；采集方式与限制记录于 validation.md。
- [x] 6.3 新增属性属加法，`ThemePalette` 使用 PIMPL，对象布局不变，不涉及 ABI 主版本调整。
- [x] 6.4 `openspec validate add-custom-theme-palette --strict --no-interactive` 通过。

## Workflow follow-up

- 已创建 `validation.md` 记录本轮真实验证结果。
- 归档时同步 `theme` 主规格，并删除 `Known Deviations` 中"覆盖 primary 不会重新推导 onPrimary 等关联 token"一条。
- 归档后运行 `openspec validate --specs --strict --no-interactive`。
