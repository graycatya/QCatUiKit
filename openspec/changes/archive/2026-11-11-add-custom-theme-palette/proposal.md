# Proposal

## Why

应用无法在不修改库源码的情况下定义自己的主题。当前 Light/Dark 两套色板硬编码在 `qcatthemepalette.cpp`，`overrides` 只接受 22 个内置 token 且拒绝未知键，覆盖 `primary` 也不会重新推导 `onPrimary`、`primaryHover`、`primaryPressed`。

结果是调用方只能调整品牌色、圆角、字号和密度，做不出"第三套主题"；想做也做不到一致性，因为派生色不会跟随。同时两类输入静默失效：未知 token 键被直接丢弃，越界数值被忽略且回退为原值，调用方无法诊断为什么覆盖没有生效。

目标是把主题从"库内置的两套配色"变成"库提供计算规则、调用方提供色板"。

## What Changes

- `ThemePalette` 增加 `colorPalette` 属性：调用方提供完整颜色色板替换内置色板，缺失项回退到当前 `colorScheme` 的内置值，空 map 表示回到内置色板。
- `overrides` 允许扩展 token：未知键保留到 `tokens` 供调用方读取，并对无效输入发出警告，不再静默丢弃。
- 越界数值覆盖从"忽略"改为"按该 token 的有效范围钳制"，与 `radius`／`fontSize` setter 的策略统一。
- 派生颜色改为基于最终生效的品牌色：未显式提供的 `onPrimary`、`primaryHover`、`primaryPressed` 从最终 `primary` 推导。
- 更新 `src/uikit/doc/THEMING.md` 的配置与覆盖说明。
- Gallery 设置页增加主题方案选择，并新增双主题混排演示，证明同一应用可并存两套视觉。

## Capabilities

### New Capabilities

无。

### Modified Capabilities

- `theme`: 增加自定义颜色色板要求；修改覆盖语义（扩展键、钳制、诊断警告）；修改派生颜色要求。

## Impact

影响 `src/uikit/theme/qcatthemepalette.h`、`qcatthemepalette.cpp`、`src/uikit/doc/THEMING.md`、Gallery 设置页与新增演示页。

公共接口：新增 QML/C++ 属性 `colorPalette`，属加法，`ThemePalette` 使用 PIMPL 不改变对象布局，不涉及 ABI 主版本调整。

行为变更（须在变更说明中单列）：

- `overrides` 中的未知键从"被丢弃"改为"保留到 `tokens`"。现有测试 `tests/auto/models/tst_models.cpp::themeContract` 中的 `QVERIFY(!theme.tokens().contains("unknown"))` 与 `radius=-1` 保持 6 的断言按新契约改写。
- 覆盖或色板修改 `primary` 后，`primaryHover`、`primaryPressed`、`onPrimary` 会随之变化。此前显式覆盖 `primary` 的调用方会看到派生色改变，需要在变更说明中提示对比度复核。

不涉及：组件结构、QML 类型注册、字号／圆角分级、组件解剖注入、内置主题成品。这些留待后续独立变更。

## 待确认

1. 属性名 `colorPalette` 是否采用；备选 `palette`、`colorOverrides`。
2. 未知键是否必须发警告；若担心噪声可改为仅在 `QT_LOGGING_RULES` 开启时输出。
3. 第一批是否包含字号／圆角分级 token；本提案默认不含，避免同时改动 20 个裸 `Item` 组件。
