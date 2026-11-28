# gallery Specification (delta)

## ADDED Requirements

### Requirement: R08 RtlPreview

Gallery SHALL 提供从右到左布局预览，并 SHALL 通过同一状态驱动设置页开关与命令行入口。该预览 SHALL 只镜像布局方向，SHALL NOT 新增 RTL 语言或翻译。

#### Scenario: 设置页切换

- WHEN 在设置页打开「从右到左布局」开关
- THEN 根布局 SHALL 启用 `LayoutMirroring` 并让子项继承
- THEN 侧边栏 SHALL 移到窗口右侧，内容区 SHALL 移到左侧
- WHEN 关闭开关
- THEN 布局 SHALL 恢复原始方向

#### Scenario: 命令行入口

- WHEN 使用 `--rtl` 启动 Gallery
- THEN 首帧 SHALL 以镜像布局显示，无需再操作设置页

#### Scenario: 范围

- WHEN RTL 预览开启
- THEN 界面语言 SHALL 保持当前选择，SHALL NOT 因镜像切换翻译
- THEN 库组件的 `LayoutMirroring` 与 `mirrored` 分支 SHALL 按既有契约工作
