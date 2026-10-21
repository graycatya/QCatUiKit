# 三维图表

[文档索引](README.md) · [二维图表](CHARTS.md) · [渲染架构](CHART_RENDERING.md)

三维图表可显示空间散点、分组柱阵、轨迹和规则曲面。用 `CatChartSeries3D` 提供 XYZ 数据，加入 `CatChart.scene3D.series`，再设置 `scene3D.enabled: true`。

`scene3D` 是图表拥有的 `CatChartScene3D`，无需自行创建。关闭三维后恢复原二维系列；两套数据和选择分别保留。三维需要硬件图形后端；软件后端会显示不可用提示。内部渲染不依赖 Qt Graphs。

```qml
import QtQuick
import QCatUiKit

CatChart {
    width: 720; height: 480
    title: qsTr("3D measurements")
    scene3D.enabled: true
    scene3D.lightingModel: CatChartScene3D.BlinnPhong
    scene3D.ambient: 0.25
    scene3D.diffuse: 0.7
    scene3D.specular: 0.5
    scene3D.series: [CatChartSeries3D {
        name: qsTr("Measurements")
        shape: CatChartSeries3D.Sphere
        pointSize: 0.12
        color: Theme.tokens.primary
        points: [{x: 0, y: 0, z: 0}, {x: 1, y: 2, z: 1},
                 {x: 2, y: 1, z: 3, size: 1.5, label: "A"}]
    }]
}
```

三维需要支持硬件实例化的 Qt Quick RHI 后端。software 保留二维回退，但不会模拟三维：`scene3D.supported` 为 false，`error` 和图表文字明确报告原因。资源分配／管线错误也可查询。错误状态周期检查，间隔 250 ms。QRhi 是私有依赖，升级 Qt 小版本后需重新构建和验证。

## 数据与图表类型

`CatChartSeries3D` 是独立的新类型；旧 `CatChartSeries.Sample.value` 仍用于气泡大小／热度，没有改成 Z。公开 C++ 头为 `qcatchartseries3d.h`、`qcatchartscene3d.h`，同时安装 `ChartSeries3D[.h]`、`ChartScene3D[.h]` 转发头。两个类使用 Pimpl。

- `points`：记录数组，每项必须有有限 double `x/y/z`。可选 `size` 默认为 1，范围 `(0,100]`；`color`、`rotation: Qt.vector3d(...)`（欧拉角，度）、`label`、`valid`。缺失点用 `valid:false`，坐标仍须有限，不以 NaN 表示缺口。
- `setGrid(rows, columns, points)`：完整行优先数组，长度必须等于行数乘列数。`rows/columns/count` 只读。网格点的 XYZ 由调用方指定，行列是拓扑与选择身份，不自动改写坐标。
- C++ `setSamples(QVector<Point>, rows=0, columns=0)` 保留 double 数据；`samples()` 返回只读引用。几何先在 CPU 用 double 归一化，再进入 float GPU 缓冲，避免直接把大数据坐标交给 GPU。
- `append(record)`、`remove(index,count=1)` 仅用于非网格；`replace(index,record)` 支持两者；`clear()` 同时清除尺寸。修改失败返回 false、保留之前的数据；读取 `error`，监听 `errorChanged`。`points` 属性赋值失败同样保留旧数据。
- 每系列最多 1,000,000 条记录。这是输入上限，不是流畅帧率保证。所有变更应在 QObject 所属线程执行，跨线程变更被拒绝。
- `scene3D.series` 支持 QML 列表；C++ 可 `addSeries/removeSeries/clearSeries/seriesCount/seriesAt`。加入列表不转移 C++ 对象所有权，销毁自动移除。一个系列可供多个场景使用，选择、相机和切片保持场景独立。

| type | 行为与设置 |
| --- | --- |
| `Scatter`（默认） | XYZ 散点；`pointSize` 默认 0.06，是归一化场景中的直径／边长，范围 `(0,2]`，再乘每点 size；`shape` 为 Sphere／Cube／Cylinder／CustomMesh／RoundedCube。逐点颜色与旋转可选 |
| `Bars` | 需要网格。Y 是高度，`baseline` 默认 0；正负高度分开延伸；`barWidth` 默认 0.7，范围 `(0,1]`，控制行列空隙。可见柱系列在 X 方向并排分组，支持 Cube／Cylinder／CustomMesh／RoundedCube；默认 Sphere 在柱阵中使用 Cube。不是堆叠柱阵 |
| `Curve` | 按记录顺序生成 Catmull-Rom / Hermite 轨迹，`tension` 0–1，默认 0；`closed` 默认 false；每段 `subdivisions` 1–64，默认 12；`pointSize * 0.15` 是管线半径；`nodesVisible` 默认 false。无效点断开；重复点形成的退化段省略 |
| `Surface` | 需要规则网格；`surfaceMode` 为 Filled／Wireframe／FilledWireframe。填充共享顶点与索引，法线由邻接面累计；线框使用略高于表面的细管，避免共面闪烁。四角任一无效则整个网格单元省略 |

`visible`、`name`、`color` 是通用属性，颜色默认 `#1677ff`。`colorByHeight` 默认 false，开启后用归一化高度从蓝到红着色，逐点颜色优先。当前三维为不透明材质，颜色 alpha 不产生透明排序。自定义 `customMesh` 是本地空间的 `QVector3D` 三角形列表，每三个点一面，最多 300,000 顶点，顶点距原点不超过 100；零面积三角形省略，法线自动生成。应用自行加载外部模型并转换为该列表，本库不提供模型文件下载或解析器。

`meshDetail` 为 LowDetail／MediumDetail（默认）／HighDetail，控制共享球体、圆柱和倒角立方体网格；不改变原始数据、普通 Cube、自定义网格或曲面。球体分别使用 8×12／12×24／20×40 经纬划分，省略极点退化三角形；圆柱为 12／24／40 边。RoundedCube 是单位边长、局部倒角半径 0.08 的网格，面、边和角使用连续法线；实例缩放会同时缩放倒角。轨迹管线使用平滑侧面法线。高精度增加每个实例的 GPU 顶点处理量，大规模散点可选 LowDetail；当前没有自动随距离变化的 LOD。

曲面 `textureSource` 接受本地／qrc 图片，最大 4096×4096，按行列 UV 拉伸，与点颜色相乘。加载失败保留无纹理填充并报告场景错误。`setHeightMap(url, heightScale=1)` 把 2×2 至 1000×1000 图片转为网格：X=像素列，Z=像素行，Y=灰度/255×heightScale，完全透明像素为缺失点。它只更新数据，不自动改变 type。没有网络下载路径。

曲线细分和曲面生成设有 6,000,000 顶点预算；超限时省略该系列并报告错误。应减少网格、细分或使用切片；当前没有三维 LOD。线框比填充需要更多顶点，因此其完整网格预算更低。

## 极坐标

Scatter 和 Surface 的 `polar: true` 将原记录解释为 **X=角度、Y=高度、Z=半径**；`degrees` 默认 true，false 表示弧度。负半径不绘制，零半径允许。CPU 转换为 `(r*cos(angle), height, r*sin(angle))` 后进入同一渲染／拾取管线。拾取返回原始角度和半径，不返回变换后的 XYZ。

极坐标曲面在 `closed: true` 时连接每行最后一列与第一列。调用方可以省略重复的 360° 列；零半径中心允许多个重合顶点，退化三角形不产生可选面积。Bars 和 Curve 忽略 polar；没有极坐标柱阵模式。

## 相机、轴与光照

| 场景属性／方法 | 默认与语义 |
| --- | --- |
| yaw / pitch | 45° / 25°；yaw 环绕，pitch 限制在 ±89.9° |
| distance / target | 5 / `(0,0,0)`；distance 范围 0.2–100，target 位于归一化场景空间 |
| projection | Perspective 或 Orthographic；透视使用 40° 基础垂直视角，并预留轴标签空间 |
| nearPlane / farPlane | 0.05 / 100；近面至少 0.001，远面至多 10000，始终 near < far |
| setPreset | Isometric、Front、Top、Right；仅设置角度 |
| resetView | 恢复角度、target、distance、裁剪与透视，不修改数据、轴范围、切片和材质 |
| rotateBy / panBy / zoomBy | 角度增量／逻辑像素平移／缩放因子；大于 1 放大 |
| lightingModel | BlinnPhong（默认）或 Phong（旧版兼容模式）；均保留环境／漫反射／镜面三个分量 |
| ambient / diffuse / specular | 0.25 / 0.7 / 0.5，各在 0–2 |
| shininess / lightDirection | 32（1–256）／`(0.5,1,1)`；方向指向光源，设置时归一化。BlinnPhong 内部采用 4× 指数，使高光宽度与旧参数大致接近 |
| ambientGroundRatio | 0.6（0–1）；BlinnPhong 的下半球补光相对上半球强度，沿世界 Y 轴按法线平滑混合；1 为均匀环境光 |
| exposure | 1（0–8）；BlinnPhong 在线性光照后、亮部压缩前的曝光倍率；0 为黑色 |
| gridVisible | true；底部网格和三条轴参与深度测试 |

`scene3D.xAxis/yAxis/zAxis` 是场景自己的 CatChartAxis，支持自动／手动范围、线性／对数、反向、分类文本及轴标题。对数轴跳过非正值；对数柱基线须为正。当前三维刻度为每轴五个候选标签，边界内布局并省略冲突刻度，DateTime 按数值显示；二维的多侧布局、次刻度和格式不是三维刻度 API。坐标框归一化到 `[-1,1]`，图形可伸出框；手动轴范围不是任意空间裁剪盒。

BlinnPhong 将 QColor／顶点颜色按 sRGB 解码到线性空间后插值，纹理采样后按 sRGB 解码，再计算主方向光与半球环境补光。纹理仍在 RGBA8 编码空间过滤，不提供 ICC／HDR 纹理管理。超过 0.8 的线性 RGB 峰值采用连续肩部曲线压缩，保持 RGB 比例，避免强光直接截白；最终编码一次写入普通 RGBA8 目标供 Qt Quick 合成。此路径是经验光照，不是 PBR，也不包含环境反射贴图。

Phong 保留旧版编码空间的环境常量、反射向量高光与截断输出，忽略 ambientGroundRatio／exposure；可用于已有应用的材质兼容。切换模型不会丢失设置值。新增属性拒绝越界／非有限值，复用 materialChanged 信号。默认改为 BlinnPhong，因此升级后需要严格保留旧光照的应用应显式设置 `scene3D.lightingModel: CatChartScene3D.Phong`。

法线经实例矩阵的逆转置变换，非均匀缩放柱体仍有正确光照。相机、光照模型和材质修改只更新统一缓冲，不重建几何或上传顶点／实例／纹理；系列（含 meshDetail／shape）、轴、选择和切片变更重建几何。没有光晕、Bloom 或实时阴影。阴影是单独的后续能力，不以表面高光代替阴影验收。

鼠标左拖旋转，Shift+左拖平移，滚轮缩放；双击／Home 复位。单指旋转、轻点选择，双指平移和缩放。继承 CatChart 的 `interactive`、`touchNavigationEnabled` 开关。三维不使用二维框选模式。`mapToPosition(x,y,z)` 接受转换后的笛卡尔数据坐标，返回图表逻辑像素；近远面外返回非有限坐标，不能直接用于 UI 定位。

## 选择、切片与诊断

`scene3D.selectionMode` 默认 PointSelection，另有 NoSelection、RowSelection、ColumnSelection。行列选择在网格上生效，普通散点退化为点选择。`selectPoint(seriesIndex,index)`、`clearSelection()`、只读 `selectedPoint` 和 `selectionChanged` 管理一个锚点；网格模式根据锚点突出整行／列。数据修改或被选系列删除后清除选择；样式修改不清除。

`dataAt(position)` 使用场景对象 BVH 加射线／真实三角形相交，选择裁剪区间内最近的数据表面；返回原始记录及 `seriesIndex/seriesName/index/selected`，网格附加 `row/column`。曲面返回命中三角形重心权重最大的原始顶点；线框和轨迹返回对应节点。点击触发 `scene3D.pointClicked`。被选点使用黄色高亮，原始标签投影后显示；被遮挡时不显示选中标签。

`sliceMode` 为 NoSlice／RowSlice／ColumnSlice，`sliceIndex` 从 0 开始。柱阵筛选对应行／列；曲面切片显示该行／列的连续折线，缺失点断开。越界切片为空；完整数据、自动范围和行列身份不变。普通非网格数据不应用切片。

`scene3D.renderingStatistics()` 和三维模式下的 `CatChart.renderingStatistics()` 返回：

- `geometryBuilds`、`preparationNanoseconds`：CPU 几何准备，不是 GPU 帧率。
- `pending`：数据或视图等待发布；`preparedRevision/renderedRevision`：发布与 RHI 提交版本。
- `vertexUploads/instanceUploads/uploadedBytes/bufferAllocations/textureUploads/instanceCount`：缓冲和纹理诊断。vertexUploads 包含索引缓冲；uploadedBytes 不计相机／材质统一缓冲。
- `pickObjects/pickTriangles`：最近一次拾取进入的候选对象与三角形数量。
- `frames/error/backend`：RHI 回调次数、渲染错误、rhi3d 或 unsupported。软件路径没有 RHI 帧。

Gallery → 图表 →「三维图表」提供四类图表、极坐标、形状、曲面模式、相机、光照、选择和切片控件；命令行可用 `--chart-demo 19`。已验证环境与未覆盖平台见 [验证记录](VALIDATION.md)。
