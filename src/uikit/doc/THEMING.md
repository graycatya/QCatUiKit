# 主题与交互密度

[文档索引](README.md) · [组件目录](COMPONENTS.md)

修改 `Theme` 可以统一调整控件的颜色、字号、圆角和交互密度。主题配置（Seed）会生成一组有明确用途的值（Token），例如 `text` 表示正文颜色、`controlHeight` 表示推荐控件高度；组件通过绑定使用这些值。

`Theme` 是每个 QML 引擎独立的默认 `ThemePalette` 单例。下面的赋值写在应用根对象的 `Component.onCompleted` 中：

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

单行输入框的内容与占位提示默认垂直居中，运行时切换密度时保持输入和选区；显式设置 `verticalAlignment` 时，两者使用相同的对齐方式。密度由应用显式选择，窗口宽度和设备类型不会自动改变 Theme.density；布局响应应根据可用空间实现。

颜色 Token：`primary`、`primaryHover`、`primaryPressed`、`onPrimary`、`background`、`surface`、`surfaceHover`、`text`、`textSecondary`、`textDisabled`、`border`、`disabled`、`success`、`warning`、`danger`。尺寸及行为 Token：`radius`、`fontSize`、`controlHeight`、`spacing`、`padding`、`motionDuration`、`hoverEnabled`。

`onPrimary` 根据品牌色相对亮度选择黑或白。未显式提供的 `onPrimary`、`primaryHover` 和 `primaryPressed` 从最终生效的 `primary` 推导，因此通过色板或覆盖修改品牌色后，这三项会随之重新计算。显式提供的值优先于推导结果。使用透明品牌色时，最终对比度仍由应用背景共同决定。

## 自定义色板

应用可以在自己的代码中定义一套颜色，不必修改库源码。把 `colorPalette` 设为一个颜色映射即可，Light/Dark 仍作为内置色板与缺失项的来源：

```qml
ThemePalette {
    id: solarized
    colorScheme: ThemePalette.Dark
    colorPalette: ({
        background: "#002b36",
        surface: "#073642",
        surfaceHover: "#0a4453",
        text: "#93a1a1",
        textSecondary: "#839496",
        border: "#586e75",
        success: "#859900",
        warning: "#b58900",
        danger: "#dc322f"
    })
}
CatPane { theme: solarized; /* ... */ }
```

- 未在 `colorPalette` 中出现的颜色使用当前 `colorScheme` 的内置值；设为空 map 即回到内置色板。
- `colorPalette` 只接受颜色 token。尺寸和行为仍由 `colorScheme`、`density`、`radius`、`fontSize` 与 `reducedMotion` 决定，避免出现"色板决定控件尺寸"的双重来源。
- 不同 `ThemePalette` 实例的色板互不影响，因此同一窗口可以并排显示两种视觉风格。
- 无效颜色或非颜色键会被忽略，并输出 `qcatuikit.theme` 类别的警告，不会影响其它 token。

`overrides` 用于单点调整。已识别的 token 会按类型和范围校验：越界数值钳制到有效范围，无法应用的值保留原值，均输出警告。未知键会原样保留在 `tokens` 中，供应用暴露自己的设计变量；它不参与内置 token 计算：

```qml
Theme.overrides = ({ radius: 10, brandAccent: "#ff8800" })
// 应用自己的组件可以读取 Theme.tokens.brandAccent
```

`colorPalette` 与 `overrides` 中的无效项只告警一次，输入集合变化后才重新提示，避免每次读取 `tokens` 重复输出。

## 局部主题

每个带主题的组件具有 `theme` 属性。自定义 `ThemePalette` 通过属性显式传递；组合组件会继续把该对象传给其内部子控件。应用自己添加的子控件需要显式绑定，避免隐式遍历任意对象树。

下面两个对象声明在同一页面中，使用时添加 `import QtQuick` 和 `import QCatUiKit`：

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
