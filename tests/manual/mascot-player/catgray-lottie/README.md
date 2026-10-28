# CatGray Blender 矢量 Lottie

直接读取 desktop_pet/CatGray_DesktopPet.blend 的求值网格和动作，使用原正交相机投影，按面的仿射深度进行遮挡裁剪。不是精灵描摹。768×832 坐标画布，24 FPS，纯矢量面与固定方向光照；无纹理、嵌入位图或外链。体素轮廓本身保留棱角；材质和光照为矢量平面着色，与 Cycles 的阴影存在差异。

完整十动作包入口为 mascot.json；generation.json 记录 blend 和动作文件校验和。fallback.png 仅用于渲染不可用时。角色授权见 LICENSE.txt。生成器保留于 IP 项目的 desktop_pet_lottie/export_catgray_blend_lottie.py，用法见该脚本头部，第一阶段需要原项目 scripts 下的姿态辅助脚本，第二阶段需要 numpy、shapely。
