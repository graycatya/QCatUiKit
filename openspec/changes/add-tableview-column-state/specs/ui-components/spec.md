## ADDED Requirements

### Requirement: R17 TableColumnState

`CatTableView` SHALL 允许应用通过 `columnWidthKeyProvider` 将列宽按稳定源列 key 保存在运行期内存中；Gallery 列配置代理 SHALL 在行数据重置时保持列顺序和隐藏状态。该状态 SHALL NOT 写入文件或由 UI 库持久化。

#### Scenario: 列宽按源列跟随

- WHEN 应用设置返回稳定源列 key 的 `columnWidthKeyProvider`
- THEN 调整列宽 SHALL 以该 key 存入内存宽度映射
- THEN 重排列、隐藏/恢复列后，源列宽度 SHALL 保持不变
- THEN RTL 和冻结布局 SHALL 使用同一 key
- WHEN 未设置 key provider
- THEN 默认仍按模型列号保存，显式/隐式 delegate 宽度行为保持不变

#### Scenario: 数据重置保留列配置

- WHEN 源模型仅重置行数据且列 header key 不变
- THEN 列顺序和隐藏状态 SHALL 保持不变
- THEN 列宽 SHALL 保持不变
- WHEN 源列新增或删除
- THEN 仍存在的列 SHALL 保留配置
- THEN 新列 SHALL 追加，删除列 SHALL 从配置中移除

#### Scenario: 纯内存边界

- WHEN 应用重启
- THEN UI 库 SHALL NOT 自动恢复列配置
- WHEN 应用需要持久化
- THEN 应用 SHALL 自行保存和加载列 key、顺序、隐藏状态和宽度
- THEN 库只消费 provider 和列代理状态，不执行文件 I/O

#### Scenario: 默认行为

- WHEN 未设置 key provider 且未发生数据重置
- THEN 布局、选择、编辑、排序、冻结、RTL 和现有宽度行为 SHALL 保持不变
