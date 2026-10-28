# CatGray 角色播放开发工具

启用 `BUILD_TESTING=ON` 与 `QCATUIKIT_BUILD_MANUAL_TESTS=ON` 构建 `qcat-mascot-player`，默认播放内置的 CatGray Voxel 精灵角色。支持动作切换、暂停、缩放、减少动效。该普通预览窗口用于验证素材和播放器，尚不提供桌面悬浮、穿透或托盘。

```sh
cmake --build <build-dir> --target qcat-mascot-player
<build-dir>/tests/manual/mascot-player/qcat-mascot-player --action working
```

参数：`--package` 指向本地目录或清单（也接受 qrc URL），`--action` 设置起始动作，`--screenshot` 等待素材就绪及起始动作生效后保存 PNG 并退出（超时 30 秒返回失败），`--width` / `--height` 指定预览尺寸（最小 420×520），`--scale` 设置角色缩放，`--dark` 使用深色主题，`--import-path` 指定已安装 QML 模块根目录。

安装库后也可独立配置此工具：

```sh
cmake -S tests/manual/mascot-player -B /tmp/qcat-player-consumer -DCMAKE_PREFIX_PATH=<install-prefix>
cmake --build /tmp/qcat-player-consumer
/tmp/qcat-player-consumer/qcat-mascot-player --import-path <installed-qml-root>
```

公开接口和包格式见 [角色播放文档](../../../src/uikit/doc/MASCOTPET.md)。此工具通过 Loader 加载库内私有视觉，不能作为应用依赖私有 URL 的推荐接入方式。

`catgray/` 是仓库所有者提供的 `CatGrayWiki/logo/ip/model_voxel/desktop_pet` 运行素材副本，只复制 manifest.json、spritesheet.png 与 LICENSE.txt。保留源清单，不改写模型、制作工程或外部目录。图集及角色素材适用 [专用许可](catgray/LICENSE.txt)，不得作为独立素材商品售卖；工具代码适用 Apache-2.0。

## Lottie 示例与制作

`--lottie` 加载自带的 `catgray-lottie/mascot.json`。窗口中的两个角色按钮可在精灵与 Lottie 包之间切换。Lottie 需要有效 QRhi 窗口和启用的 ThorVG 后端；software 下会报告错误并显示包内 fallback.png，不会伪报播放完成。

`catgray-lottie/` 直接由所有者提供的 `desktop_pet/CatGray_DesktopPet.blend` 求值网格生成，包含全部十动作：idle、thinking、working、waiting、speaking、success、error、walk、run、jump。它和 IP 目录的 `desktop_pet_lottie` 使用相同动作文件。下拉框可直接切换全部动作，`--lottie --action working --scale 2` 可预览打字动作。

动画使用原模型的正交相机，768×832 坐标画布，24 FPS。对模型表面做矢量投影和遮挡裁剪，合并同色可见面；没有描摹精灵图、颜色量化或嵌入位图。体素造型本身保留棱角；矢量平面着色与 Cycles 渲染的光照、投影和材质效果存在差异。`fallback.png` 是单独渲染的高清图片，仅用于后端不可用时。角色授权见 [LICENSE.txt](catgray-lottie/LICENSE.txt)。

素材生成脚本保留在 IP 项目的 `desktop_pet_lottie/export_catgray_blend_lottie.py`，不作为测试程序源码分发。重复生成需要 Blender、Python、numpy / shapely 及原 IP 项目 scripts 目录，命令见该生成脚本头部。`generation.json` 记录模型、辅助脚本、生成器与输出动作的 SHA-256；角色包保持 192×208 的逻辑尺寸，缩放通过 `petScale` 控制。
