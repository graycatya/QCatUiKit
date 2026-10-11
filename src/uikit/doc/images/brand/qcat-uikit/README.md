# QCatUiKit Logo

项目标识由项目作者设计，导入自同工作区的 `CatCommStudio/assets/brand/qcat-uikit`。这里保存自包含的原始 SVG、Windows ICO 与 PNG 导出，gallery 和文档直接使用本仓库的副本。

- `qcat-uikit.svg`：完整标识，灰猫和阴影为内嵌 PNG，蓝色底座与组件角标为矢量。
- `qcat-uikit.ico`：Windows exe 图标，包含 16、24、32、48、64、128、256 px。
- `png/qcat-uikit-{size}.png`：16–2048 px 全部使用完整灰猫、蓝色底座和组件角标；圆角外透明。

品牌蓝为 `#1677FF`；深色与浅色主题共用原始标识。GalleryLogo 在所有位置显示完整标识，compact 仅控制布局尺寸。

运行时嵌入 16–512 px PNG，完整 SVG 和更大 PNG 供文档及后续设计使用。更新设计时替换相应导出并重新构建 gallery；Windows RC 显式依赖 ICO，图标更新会重新编译进 exe。
