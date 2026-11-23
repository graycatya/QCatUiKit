# gallery Specification (delta)

## ADDED Requirements

### Requirement: R06 IconFont

Gallery SHALL 使用随附图标字体渲染图标控件，SHALL NOT 依赖平台默认字体提供符号字形。

#### Scenario: 字体加载

- WHEN Gallery 完成模块加载
- THEN 随附图标字体 SHALL 成功加载，图标控件使用的字体族 SHALL 为该字体的族名。
- WHEN 宿主未经过应用启动入口加载 Gallery，例如自动测试
- THEN 图标控件 SHALL 仍能解析到同一字体族。

#### Scenario: 图标按钮尺寸

- WHEN 渲染表格示例的列移动按钮
- THEN 按钮宽高 SHALL 等于主题的 `controlHeight`，内边距 SHALL 为零，SHALL NOT 出现字形溢出内容区的情况。

#### Scenario: 移动边界

- WHEN 列为第一列
- THEN 左移按钮 SHALL 禁用，右移按钮 SHALL 可用。
- WHEN 列为最后一列
- THEN 右移按钮 SHALL 禁用，左移按钮 SHALL 可用。
- WHEN 移动列后顺序改变
- THEN 边界状态 SHALL 按新顺序重新计算。

#### Scenario: 图标按钮可访问名称

- WHEN 图标按钮不含可见文本
- THEN 控件 SHALL 提供说明其操作的辅助名称。

#### Scenario: 第三方资源登记

- WHEN 分发包含该字体的 Gallery 示例
- THEN 仓库 SHALL 保留 `LICENSES/RemixIcon-1.0.txt`，并在 `NOTICE` 中说明来源、版本与许可。
