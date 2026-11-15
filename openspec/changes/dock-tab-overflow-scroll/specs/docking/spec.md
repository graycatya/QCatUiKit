# Spec Delta

## ADDED Requirements

### Requirement: R06 TabOverflowScroll

主工作区和浮动窗口的标签 SHALL 单行排列。标签超出栏宽时，左右两端 SHALL 提供滚动按钮，仅可继续滚动的方向可用。标签 SHALL 保持滚轮浏览能力。

#### Scenario: 浏览溢出标签

- **WHEN** 标签超出可见宽度且用户点击可用的滚动按钮
- **THEN** 标签栏 SHALL 向对应方向滚动，原有标签顺序和活动面板 SHALL 保持。

#### Scenario: 拖动越过可见范围

- **WHEN** 用户拖动可移动标签并停留在可继续滚动的边缘或同侧滚动按钮上
- **THEN** 标签栏 SHALL 持续向该侧滚动，被拖动的标签 SHALL 留在可见边缘附近，并随滚动更新组内插入预览。
- **WHEN** 用户移回中部、到达滚动极限或结束拖动
- **THEN** 自动滚动 SHALL 停止。用户在栏内松手时，布局 SHALL 采用预览顺序。

#### Scenario: 临时离开原栏

- **WHEN** 拖动明显离开原标签栏
- **THEN** 系统 SHALL 停止该栏的自动滚动并进入停靠预览。
- **WHEN** 用户在同一次拖动中返回原栏
- **THEN** 系统 SHALL 恢复组内换序预览。取消拖动 SHALL 保留原顺序。
