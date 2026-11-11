# Design

## 目标

让调用方在自己的应用中定义并切换主题，而不修改库源码；同时保持默认行为与既有接入方式可用。

## token 计算顺序

`tokens` 由固定顺序合成，顺序本身是契约的一部分：

1. **内置色板**：按 `colorScheme` 选择 Light/Dark 的颜色值。
2. **`colorPalette`**：逐项覆盖颜色 token，缺失项保留第 1 步的值。
3. **`overrides`**：已识别 token 按类型和范围校验后覆盖；未知键原样放入结果。
4. **派生重算**：对第 2、3 步中**未被显式提供**的 `onPrimary`、`primaryHover`、`primaryPressed`，从最终 `primary` 重新计算。
5. **减少动效**：`reducedMotion=true` 时强制 `motionDuration=0`。
6. 冻结为只读 `tokens`。

第 4 步需要记录"哪些 token 被显式提供"，因此 `colorPalette` 与 `overrides` 的写入集合要保留到计算结束，不能边写边算。

## 为什么 `colorPalette` 只管颜色

尺寸和行为 token 由 seed（`density`、`radius`、`fontSize`）确定性推导，语义清晰、组合可控。若色板也能改 `controlHeight`，会出现"色板决定密度"的双重来源，用户改密度时行为不确定。

因此分工固定：

- 颜色 → `colorPalette`（整套）或 `overrides`（单点）
- 尺寸与行为 → seed 属性
- 新增语义 → `overrides` 未知键

## 为什么保留未知键，而不是引入注册表

考虑过三种扩展方式：

| 方案 | 代价 | 结论 |
| --- | --- | --- |
| 未知键保留到 `tokens` | 几行代码，无需注册 | 采用 |
| `registerToken(name, type, default)` 注册表 | 需要类型系统、生命周期、跨引擎单例管理 | 超出当前需求 |
| 每个组件增加 `style` 枚举 | N 组件 × M 风格的组合爆炸 | 否决 |

未知键不参与内置 token 计算，只作为调用方自用扩展位：应用写 `overrides: ({ brandAccent: "#..." })`，自己的组件读 `tokens.brandAccent`。

## 静默失效的修复取舍

基线 `R02` 对 setter 采用"非法值保留原值、越界钳制"，而 `R03` 对 `overrides` 采用"越界丢弃、未知键丢弃"，同一概念两套策略。本次统一为：**保留原值／钳制 + 警告**，并让"无法应用"这件事可观察。

警告使用 `qWarning` 并带类别，避免每次读取 `tokens` 重复输出：只在输入集合变化时告警一次。

## 派生色重算的影响

基线 `Known Deviations` 已记录"覆盖 primary 不重新推导 onPrimary"。这既是技术债也是实际缺陷：用户覆盖品牌色后，`onPrimary` 仍按旧色推导，浅色品牌配白字。

修复后需注意：此前显式覆盖 `primary` 的调用方，其 `primaryHover`、`primaryPressed`、`onPrimary` 会变化。这是有意的行为变更，须在变更说明中提示复核对比度。

## ABI 与兼容性

- 新增 `Q_PROPERTY(QVariantMap colorPalette ...)`：加法，既有消费者不受影响。
- `ThemePalette` 使用 PIMPL，新增字段不改变对象布局。
- 不新增 QML 类型，`import QCatUiKit` 用法不变。
- 默认值下（空 `colorPalette`、空 `overrides`）计算路径与现行为一致。

## 不做的部分

- 字号／圆角／间距分级：会牵动 20 个根类型为裸 `Item` 的组件，单独立项。
- 组件解剖注入（`surface`/`contentItem` 替换）：涉及 108 个类型的一致性设计，单独立项。
- 内置主题成品与运行时样式包注册表：需求未验证，避免先造机制。
