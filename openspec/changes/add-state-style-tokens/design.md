# Design

## 目标

让应用层通过 `ThemePalette` 定义普通表面控件的全部常用状态，而不是逐组件替换 `background` 或 `delegate`。

## Token 语义

| Token | 语义 | Light 默认 | Dark 默认 |
| --- | --- | --- | --- |
| `surfaceHover` | 指针悬停背景 | `#f0f5ff` | `#303030` |
| `surfaceSelected` | 选中、当前、键盘高亮、打开态背景 | `#e6f0ff` | `#26344d` |
| `surfacePressed` | 指针按下背景 | `#d3e3ff` | `#3a4a63` |
| `focusRing` | 键盘焦点描边 | 派生自最终 `primary` | 派生自最终 `primary` |
| `disabledOpacity` | 禁用内容透明度（0–100） | 60 | 60 |

`surfaceHover`、`surfaceSelected`、`surfacePressed` 构成由浅到深的三级状态底色。深色主题下"更深"表现为更亮或更偏主色，保持可辨识而非机械降低亮度。

`focusRing` 与 `onPrimary`、`primaryHover`、`primaryPressed` 一样在 `colorPalette` 和 `overrides` 之后按最终 `primary` 派生；显式提供时优先。

`disabledOpacity` 使用整数百分比，沿用现有数值 token 的整数校验路径，QML 侧除以 100 得到透明度。

## 状态映射

| 状态 | 组件映射 |
| --- | --- |
| 悬停 | `surfaceHover` |
| 按下 | `surfacePressed` |
| 选中 / 当前 / 高亮 / 打开 | `surfaceSelected` |
| 键盘焦点 | `focusRing` |
| 禁用 | `disabledOpacity / 100` 或 `disabled` 颜色 token |

主色系控件（`primary=true`、选中态开关）继续使用 `primary`、`primaryHover`、`primaryPressed`，不受本次映射影响。

`CatTableCell` 的 `selectedColor` 默认值改为 `surfaceSelected`，`hoverColor` 保持 `surfaceHover`；两者仍是可覆盖的公开属性。

## 兼容性

- 新增 token 不改变现有键；`colorPalette` 只覆盖色板未提供项时仍取内置值。
- `overrides` 对 `surfacePressed`、`surfaceSelected`、`focusRing` 走颜色分支；`disabledOpacity` 走数值分支并钳制到 0–100。
- 视觉变化：按下和选中状态由原来的 `surfaceHover` 变为更深的 `surfacePressed` / `surfaceSelected`；页签选中背景由 `surfaceHover` 改为 `surfaceSelected`。这是本次目标，需在浅色和深色下复核。
- 现有悬停断言不变；页签选中相关规约需同步。

## 非目标

- 不新增 `CatButton.hoverColor` 之类的逐组件状态色属性；统一入口是主题 token。
- 不引入运行时样式表或 QML 插件式 style。
- 不改变触摸密度下 `hoverEnabled=false` 的既有设计。
