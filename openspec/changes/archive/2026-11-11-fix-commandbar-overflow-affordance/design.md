# Design

## 为什么不再用 CatDropDownButton

`CatDropDownButton` 面向“文字 + 下拉箭头”的按钮：右内边距固定为 `tokens.padding + 18`，箭头按 `width - 18` 定位。命令栏需要的是正方形图标按钮，两者约束冲突。`CatSplitButton` 仍使用该控件且文本为空，不受影响。

## 宽度

```qml
implicitWidth: Math.max(control.tokens.controlHeight, implicitContentWidth)
leftPadding: 0
rightPadding: 0
```

`CatButton` 自带 80 的最小宽度，命令栏必须覆盖它才能得到正方形按钮。宽度下限保持 `controlHeight`，上限随内容增长，因此字号或密度变化时不会再次出现负的可用宽度。这条是结构性约束：只要内边距为零，可用宽度就不可能为负。

## 菜单定位

沿用原实现：点击时把菜单父项设为按钮，`x = 0`、`y = height + 4`，再 `open()`。菜单内容仍由 `Instantiator` 根据 `secondaryActions` 生成，未改动。

## 测试钩子

`objectName` 只服务回归测试，不写入组件文档，不作为 API 承诺；测试限定在 `contentItem` 内查找并额外确认文本，避免同名对象误命中。

## 不做的部分

- 不把溢出动作折叠成二级菜单：需要新的数据契约，本次只修可见性。
- 不为省略号引入图标资源：字体字形在当前字号下已足够辨识，引入资源会牵动部署。
