# 维护入口与最小上下文

[贡献约定](../../../CONTRIBUTING.md) · [API 索引](README.md)

本页用于定位文件和验证入口，不复制组件 API。先确定组件／子系统，只读对应专题和源码；实现事实以源码为准。

## 按改动定位

| 改动 | 首先读取 | 通常需要同步 |
| --- | --- | --- |
| QML 外观／交互 | `src/uikit/qml/Cat<Type>.qml`、COMPONENTS.md 对应条目 | QML 行为测试、Gallery 演示；必要时主题说明 |
| 新 QML 组件 | 相近组件、CONTRIBUTING.md、THEMING.md | 模块 CMake、COMPONENTS、GalleryCatalog／Preview／CMake、中英文 TS、行为测试 |
| 新公开 C++ 类 | 相近公开头、ABI.md 的接口约定 | CMake 三组头列表／sources、专题 API、C++ 测试、安装消费者 |
| Gallery 演示 | `examples/uikit/gallery/GalleryCatalog.qml` 中组件 ID，再定位 Preview | 演示页面、可复制示例、两种语言 TS、Gallery 测试 |
| Dock | DOCKING.md 的实现与验证入口 | 控制器／QML／Gallery 分别按实际改动读取 |
| 构建／安装 | BUILDING.md、模块 CMake | 安装消费者；公开 ABI 变更检查历史消费者 |

## 检索与读取

先 `rg --files` 定位，再限定路径和文件类型。不要把图片、SVG 内嵌数据、构建产物和整个仓库输出到上下文。例如：

```sh
rg --files src/uikit/qml tests/auto/qml | rg 'Dock|dock'
rg -n -g '*.qml' 'dock-workspace' examples/uikit/gallery
rg -n -g '*.h' -g '*.cpp' 'DockController' src/uikit tests/auto/dock
```

先读取命中附近的函数／组件，只有依赖关系需要时再扩大范围。已有架构图可作定位提示，事实须核对源码；常规单组件维护无需生成全仓库知识图谱。

## 验证与记录

遵循 CONTRIBUTING.md 的构建、lint 和 CTest 要求。开发迭代先跑受影响测试，最终运行要求的检查；已通过且没有新改动／新风险时不反复重复。将长日志写到临时文件，只读取失败段，修复后记录命令、结果和未覆盖平台。

构建目录和本机 Qt 路径从当前 CMakeCache 检查，不把个人路径写进共享说明。

专题文档记录当前接口、边界、关键设计原因和测试入口。任务结束记录已完成／未完成及证据，避免下次靠整段聊天历史重建进度。详细历史留在 Git，不持续追加进根 AGENTS.md。

## 提交任务时提供

“组件／目标行为 + 触发步骤或示例 + 不变的约束 + 验收标准”通常足够。已有计划直接引用专题路径和阶段；不要反复粘贴全套文档，也不要省略复现条件而让 agent 猜测。
