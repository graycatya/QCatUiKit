# 架构与维护

```text
应用 QML / C++
  ├─ QCatUiKit QML 模块：稳定属性、信号、组合组件
  │    ├─ Qt Quick Templates：焦点、输入、可访问性、状态机
  │    ├─ Theme / ThemePalette：Seed → 语义 Token → 控件
  │    └─ SurfaceImpl：私有视觉实现
  └─ libQCatUiKit（共享库，SONAME 1）
       ├─ ThemePalette → Impl：主题种子、覆盖、派生值
       ├─ ChatModel → Impl：消息值集合
       └─ TableModel → Impl：列、表头、行值集合
```

C++ 公开头位于 `include/QCatUiKit`；状态、容器与算法位于 `src/*.cpp`。`qt_add_qml_module(SHARED)` 自动生成薄插件和类型注册，使用 `/qt/qml` 资源前缀；Debug 与 Release 不切换源码绝对路径。Qt 6.8 是功能下限，Qt 5 分支与手写注册入口已移除。

模型只管理数据，不设置鼠标光标、单元格宽度或窗口尺寸。`TableModel` 使用真正的 `QAbstractTableModel`，列顺序由 `columns` 确定；缺少字段返回无效值。`ChatModel` 存储值类型 map，避免 QObject 父子所有权与共享指针双重管理。所有 QObject 模型操作应在其所属线程执行；从工作线程通过 queued connection 交付数据。

控件优先使用 Templates 的点击、键盘、焦点和禁用协议，不在按钮上再叠加 MouseArea 手动发出 clicked。表格与聊天使用虚拟化视图，页面布局按可用宽度响应，不根据设备名称硬编码尺寸。

## 开发约定

1. 先更新或确认公开行为，再实现；先复现再修复具体缺陷。
2. 测试覆盖可观察结果：信号、数据、选择、输入及主题变化，不断言私有 id 或视觉树层级。
3. 每次改动审查明暗模式、禁用、加载、焦点、键盘、触摸、RTL、边界值和对象生命周期中与本次改动有关的行为。
4. 模型使用 `QAbstractItemModelTester` 检查协议，控件使用 Qt Quick Test；运行 CMake 生成的 qmllint 目标。
5. ABI 相关修改按 ABI 文档检查，再运行安装消费者与动态替换测试。
6. 保留最小依赖，新增组件先复用 Token，不复制固定颜色和尺寸。

参考文章讨论的是开源维护流程，故这里采用其公开契约和回归测试思想。没有照搬 React 的 DOM、Hooks 或 CSS 运行时。

## 迁移分析辅助资料

graphify 用于旧表格模型的结构定位。该次图提取包含 143 个节点、218 条构建后边；诊断有 4 条悬空端点与 3 组同端点关系合并，故关系图仅作导航，关键结论均回到源码核对。`graphify-out/` 为本地分析产物，不进入安装包，也不是迁移完备性证明。
