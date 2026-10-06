# turtlesim 贴图：照片小海龟

把 turtlesim 里的小海龟换成 `face_source.jpg` 里那张照片，尺寸和原版贴图一致（45x45 像素、
1 米大小），并且可以在“照片”和“原版小海龟”之间随时切换。

## 启动

```bash
bash ~/chapt3/turtlesim_face_ws/run_turtlesim.sh
```

脚本会自动 source `/opt/ros/jazzy` 和本工作区，启动的 turtlesim 默认显示照片贴图。
遥控终端照旧用 `ros2 run turtlesim turtle_teleop_key`，topic 名字和原版完全一样。
跑之前记得先关掉已经在运行的 turtlesim 窗口：两个实例共用 `/turtle1` 话题会互相打架。

## 切换贴图

- 在 TurtleSim 窗口里按 `T`
- `ros2 service call /toggle_skin std_srvs/srv/Empty "{}"`
- `ros2 param set /turtlesim use_face false`（false = 原版小海龟，true = 照片）
- 启动时就想显示原版小海龟：`bash run_turtlesim.sh --ros-args -p use_face:=false`

## 换成别的照片 / 调整取景

1. 用新照片覆盖 `face_source.jpg`
2. 需要的话改 `make_face_sprite.py` 顶部的 `CROP`（裁剪区域）、`MASK`（圆形/圆角/方形）等参数
3. `python3 make_face_sprite.py`
4. `colcon build --packages-select turtlesim`

## 说明

- 贴图会像原来的小海龟一样随朝向旋转：贴图的正上方 = 小乌龟前进方向。
- 这里是源码编译的 turtlesim 覆盖版本（overlay），只有 source 过本工作区的 `install/setup.bash`
  之后才生效；没 source 的时候 `ros2 run turtlesim turtlesim_node` 仍然是系统自带的原版。
- 课件工作区 `chapt3_ws` 里的代码没有改动；不想要这套贴图时，删掉 `turtlesim_face_ws` 目录即可。
