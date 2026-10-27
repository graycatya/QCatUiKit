# Validation Record

## Planning Checks

本轮仅创建规划文件。实现、构建、性能和任何平台实机验证均未执行。

2026-10-27 规划检查：

- `openspec.cmd status --change add-mascot-pet`：4/4 规划产物完成，所有实现任务保持未完成。
- `openspec.cmd validate add-mascot-pet --strict`：通过。
- `python tools/check_docs.py`：通过；检查 35 个 Markdown 文件、490 个本地链接与 122 个公开 Cat 类型。该脚本的既定范围不等于覆盖所有 change 文件，change 结构由 OpenSpec 严格验证补充检查。

环境说明：PowerShell 禁止执行 npm 的 `.ps1` shim，使用现有 `openspec.cmd` 完成同一 CLI 操作，未修改执行策略。Git 只在命令级指定当前仓库 safe.directory，未修改全局配置。

## Platform Acceptance Matrix

以下为验收计划，不是当前支持声明。每格初始为 Pending；实机失败改为 Failed，通过需附证据。具体系统、Qt、桌面及配套版本在原型阶段确定。

| 场景 | Windows | macOS | X11 | Wayland KWin | Wayland Mutter |
| --- | --- | --- | --- | --- | --- |
| 透明无边框、显示不抢焦点 | Pending | Pending | Pending | Pending | Pending |
| 可配置置顶、菜单与气泡键盘操作 | Pending | Pending | Pending | Pending | Pending |
| 角色区域交互、空白点击到背后窗口 | Pending | Pending | Pending | Pending | Pending |
| 整窗穿透与宿主／托盘恢复 | Pending | Pending | Pending | Pending | Pending |
| 拖拽、定位、平滑移动及取消 | Pending | Pending | Pending | Pending | Pending |
| 跨屏、混合 DPI、移除屏幕及状态恢复 | Pending | Pending | Pending | Pending | Pending |
| 气泡边缘避让、长文本、输入区域同步 | Pending | Pending | Pending | Pending | Pending |
| 托盘可用或不可用诊断及宿主恢复路径 | Pending | Pending | Pending | Pending | Pending |
| 精灵／Lottie 播放、隐藏及减少动效 | Pending | Pending | Pending | Pending | Pending |
| 多实例隔离、关闭隐藏、不退出宿主 | Pending | Pending | Pending | Pending | Pending |
| 配套缺失、禁用、版本错误、断开恢复 | Pending | Pending | Pending | Pending | Pending |

配套故障场景在不依赖配套的环境记录“不适用及原因”；Wayland 两环境必须实测。完整支持仅针对矩阵通过的明确版本组合，不外推到所有系统版本或所有 Wayland 合成器。

## Evidence Template

每次运行记录：日期、提交标识、系统版本、Qt 与工具链、图形后端、会话类型（原生 Wayland/XWayland/X11）、桌面／合成器及配套版本、屏幕布局与 DPI、执行命令、预期／实际结果、日志或截图位置、失败和未覆盖场景。

重复操作步骤后续维护在 `src/uikit/doc/VALIDATION.md`，本文件仅保留本 change 的实际结果及验收状态，归档时一并保留。
