# 主题架构

主题从 Seed（品牌色、字号、圆角、明暗模式、密度）推导语义 Token，再由组件消费。`Theme` 是每个 QML 引擎独立的默认 `ThemePalette` 单例，不是跨引擎全局 QObject。

```qml
Component.onCompleted: {
    Theme.colorScheme = ThemePalette.Dark
    Theme.primaryColor = "#1677ff"
    Theme.density = ThemePalette.Touch
    Theme.reducedMotion = true
}
```

| 设置 | 默认值 | 契约 |
| --- | --- | --- |
| `colorScheme` | `Light` | `Light` / `Dark`；显式选择，不依赖平台样式 |
| `density` | `Desktop` | `Desktop` / `Touch` / `Mobile` |
| `primaryColor` | `#1677ff` | 非法颜色忽略，推导悬停、按压与前景颜色 |
| `radius` | 6 | 限制在 0–32 |
| `fontSize` | 14 | 限制在 10–32 |
| `reducedMotion` | false | 过渡时长为 0；循环装饰动画停止 |
| `overrides` | `{}` | 局部语义 Token 覆盖；未知键和非法类型不生效 |
| `tokens` | 只读 map | 配置变化发出 `tokensChanged`，绑定自动刷新 |

| 密度 | controlHeight | padding | spacing | hoverEnabled |
| --- | --- | --- | --- | --- |
| Desktop | 32 | 12 | 8 | true |
| Touch | 48 | 16 | 12 | false |
| Mobile | 44 | 16 | 12 | false |

高度是控件的隐式最小高度。调用方显式设置 `height`、`implicitHeight` 或布局限制时可以覆盖它；触摸界面应避免把尺寸压到推荐值以下。

颜色 Token：`primary`、`primaryHover`、`primaryPressed`、`onPrimary`、`background`、`surface`、`surfaceHover`、`text`、`textSecondary`、`textDisabled`、`border`、`disabled`、`success`、`warning`、`danger`。尺寸及行为 Token：`radius`、`fontSize`、`controlHeight`、`spacing`、`padding`、`motionDuration`、`hoverEnabled`。

`onPrimary` 根据品牌色相对亮度选择黑或白。语义颜色覆盖不会自动重新推导其关联 Token，例如覆盖 `primary` 时应同时考虑 `onPrimary`；通常应修改 Seed `primaryColor`。使用透明品牌色和自定义覆盖时，最终对比度由应用背景共同决定。

## 局部主题

每个带主题的组件具有 `theme` 属性。自定义 `ThemePalette` 通过属性显式传递；组合组件会继续把该对象传给其内部子控件。应用自己添加的子控件需要显式绑定，避免隐式遍历任意对象树。

```qml
ThemePalette {
    id: kioskTheme
    density: ThemePalette.Touch
    colorScheme: ThemePalette.Dark
    primaryColor: "#13c2c2"
    overrides: ({ surface: "#182020" })
}
CatPane {
    theme: kioskTheme
    CatButton { theme: kioskTheme; text: "确认"; primary: true }
}
```

默认主题与局部主题互不修改。主题对象应由页面或应用持有；若被销毁，控件 Token 回退到默认单例。组件的 `tokens` 为解析后的只读视图，不直接修改其成员；更新配置应写 `ThemePalette`。

`reducedMotion` 同时提供无动态过渡的模式，即使 overrides 指定 motionDuration 也强制为 0；图片动画回到无偏移、无缩放、完全可见的静态状态。嵌入式设备可不用 `CatEffect`，减少离屏纹理和阴影成本。核心控件不强制创建阴影或使用 MultiEffect；只有主动使用 `CatEffect` 时才引入该效果。动画图片应设置合理 `sourceSize`（通过 CatImage 使用），避免加载超大资源。
