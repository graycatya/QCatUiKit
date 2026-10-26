# led Specification

## Purpose

提供灯点布局、RGBA 数据源、多通道帧、独立 Deck、预设仓库、混合、设备映射和 GPU 预览。

## Dependencies

- Qt Core、Gui、Qml、Quick、QRhi；预览需要硬件图形环境。
- 内部 LedLayoutModel、LightingFrame、EffectRegistry、Storage、Deck、Mixer、Output 和可选 LedProject 装配。
- 文件操作使用本地 URL；外部设备发送通过 QCatOutputBackend 注入。

## Requirements

### Requirement: R01 SourceLayout

LedSource SHALL 接受独立 XYZ 布局并返回非零布局版本。

#### Scenario: 有效布局

- WHEN 提交至多一千万个有限且坐标绝对值不超过 1e12 的点
- THEN 系统 SHALL 深拷贝布局、产生新布局 ID，并重置为 frameId=0 的透明黑色帧。

#### Scenario: 无效布局

- WHEN 布局超限或坐标非法
- THEN 系统 SHALL 返回 0、保留旧布局并设置 lastError。

#### Scenario: 空布局

- WHEN 提交空点列表
- THEN 系统 SHALL 接受空布局。

### Requirement: R02 SourceFrames

LedSource SHALL 接受每点四字节的连续 RGBA8 帧。

#### Scenario: 帧接受

- WHEN 长度匹配、frameId 非零且递增、布局版本匹配
- THEN 系统 SHALL 深拷贝颜色、返回 true 并增加 submittedFrames。

#### Scenario: 帧拒绝

- WHEN 长度错误、帧版本旧、布局版本旧或提交期间布局变化
- THEN 系统 SHALL 返回 false、保留旧帧并设置 lastError。

#### Scenario: 默认布局版本

- WHEN 提交时 layoutId 为 0
- THEN 系统 SHALL 按当前布局校验该帧。

### Requirement: R03 ChannelFrames

LightingFrame SHALL 区分 RGB、RGBW 和 RGB_CW_WW 发光通道及独立透明度。

#### Scenario: 通道结构

- WHEN 构造有效 ChannelValues8 帧
- THEN stride SHALL 等于通道数量 3、4 或 5，通道缓冲长度 SHALL 与点数匹配。

#### Scenario: 透明度

- WHEN 含独立 opacity 的帧参与混合或设备映射
- THEN 系统 SHALL 按黑背景合成，SHALL NOT 将 alpha 解释为白光通道。

### Requirement: R04 Mixing

Mixer SHALL 按通道值混合 A/B 并应用 Master 和 Blackout。

#### Scenario: 端点与中点

- WHEN 以同布局同模型的 255 和 0 通道帧混合
- THEN crossfader 为 0、1、0.5 时对应输出 SHALL 为 A、B、128。

#### Scenario: 身份不匹配

- WHEN 输入不匹配项目布局或转换失败
- THEN 系统 SHALL 返回 false，保留上一有效输出并设置错误。

#### Scenario: 冻结

- WHEN freeze 启用且布局仍匹配
- THEN 系统 SHALL 保留混合通道，但 master 和 blackout SHALL 继续生效。

#### Scenario: 全黑

- WHEN blackout=true 或 master=0
- THEN 最终输出全部通道 SHALL 为零；预览亮度 SHALL NOT 改变最终通道。

### Requirement: R05 Storage

预设仓库 SHALL 支持版本化 JSON、只读保护和本地文件读写。

#### Scenario: 只读拒绝

- WHEN 向只读仓库提交修改
- THEN 系统 SHALL 返回失败并保留仓库内容。

#### Scenario: 结构校验

- WHEN 仓库超过 128 个 Storage、每组 4096 个 Place、合计 65536 个 Place，或身份重复缺失
- THEN 系统 SHALL 拒绝提交。

#### Scenario: 导入冲突

- WHEN 导入 Storage 的 ID 与现有集合冲突
- THEN 系统 SHALL 为该集合及其槽位生成新 ID。

#### Scenario: 槽位导入

- WHEN 导入 kind=place 到有效目标槽位
- THEN 系统 SHALL 替换目标内容并保留目标槽位稳定 ID。

#### Scenario: 文件限制

- WHEN 读取非本地 URL 或超过 16 MiB 的仓库文件
- THEN 系统 SHALL 拒绝读取。

### Requirement: R06 OutputAndPreview

系统 SHALL 将最终帧分别供给预览和输出后端。

#### Scenario: 映射失败

- WHEN applyProfiles 的映射校验失败
- THEN 系统 SHALL 返回 false 并保留原映射。

#### Scenario: 关闭输出

- WHEN enabled 设为 false
- THEN 系统 SHALL 取消旧事务，SHALL NOT 将此操作解释为已经向硬件发送全黑。

#### Scenario: 预览缩放

- WHEN 修改 previewBrightness
- THEN 仅预览 RGB SHALL 受影响，数据源和 alpha SHALL 保持不变。

#### Scenario: 二维相机

- WHEN Plane2D 下调用 orbit
- THEN 系统 SHALL 忽略旋转。

## Data Model

LedSource.Position：float x/y/z，12 字节；frameId/layoutId 为 quint64。

LightingFrame.Description：QString projectId；quint64 layoutRevision/frameId/instanceId 默认 0；qint64 timestampUsec=0；int pointCount=0、stride=3；schema=RGB、encoding=ChannelValues8；channels 和可选 opacity 为 QByteArray。

布局默认 Matrix2D、64×32×1、间距一，管线布局点数上限一百万，与旧 Source 上限不同。Mixer 默认 crossfader=0.5、master=1、blackout/freeze=false、workingSchema=RGB_CW_WW、conversion=Strict。仓库文档版本一，包含 repositoryId、storages 和 places。

## Known Deviations / Technical Debt

- 旧 RGBA Source 与项目布局的点数上限不同。
- 通道混合不执行隐藏的 sRGB 线性化；白光预览是技术近似而非物理校准。
- 默认仓库基类提交不自动写盘；文件持久化依赖明确入口或具体仓库类型。
- 内存后端接收和预览统计不证明物理设备接收。
- 预览无 software 回退；半透明灯点写深度，不提供透明物体排序合成。

## Non-Goals

- 本模块 SHALL NOT 内置 DMX、Art-Net、sACN、USB 或串口协议。
- 本模块 SHALL NOT 提供完整灯具库、Cue、第三方项目文件兼容或商业算法。

## Assumptions

- 外部生产者 SHALL 在 Source 析构前停止；状态配置 SHALL 在对象线程执行。
- 外部输出后端 SHALL 遵守取消协议；实际预览 SHALL 提供有效窗口和 QRhi。
