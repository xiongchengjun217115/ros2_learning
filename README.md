# ROS 2 学习

跟着教程逐章敲下来的 ROS 2 练习代码：从手写 CMake 开始，依次是节点与话题、话题实战、服务/参数/
自定义接口，最后到 TF2 坐标变换。C++ 和 Python 各写一遍对照，每章对应一个独立工作空间
（chapt1 直接用 CMake、chapt3 有两个），可以按需单独构建和运行。

- **环境**：Ubuntu 24.04 + **ROS 2 Jazzy**
- **构建**：`colcon`（每个 `*_ws` 是一个独立工作空间）
- **语言**：C++ / Python

---

## 章节一览

| 章节 | 主题 | 工作空间 | 内容 |
|---|---|---|---|
| `chapt1` | CMake 基础 | —（纯 CMake） | 不依赖 ROS 的 hello world |
| `chapt2` | 节点与话题 | `chapt2_ws` | 节点继承、发布/订阅、多线程下载、C++ 新特性 |
| `chapt3` | 话题实战 | `chapt3_ws` / `turtlesim_face_ws` | 控制小海龟、小说发布/朗读；**给 turtlesim 换脸（自研）** |
| `chapt4` | 服务、参数与自定义接口 | `chapt4_ws` | 自定义 `.srv`、参数回调、海龟巡逻、人脸检测服务 |
| `chapt5` | TF2 坐标变换 | `chapt5_ws` | 静态/动态坐标广播、tf 查询 |

---

## 环境与依赖

系统依赖以 `ros-jazzy-desktop` 为主（已包含 `rclcpp`、`rclpy`、`turtlesim`、`tf2_ros`、
`example_interfaces`、`std_srvs` 等）：

```bash
sudo apt install ros-jazzy-desktop

# chapt4 人脸检测
sudo apt install ros-jazzy-cv-bridge python3-opencv python3-pil fonts-noto-cjk

# Python 侧依赖（chapt2 下载、chapt3 朗读、chapt4 人脸检测、chapt5 tf）
pip install requests espeakng face_recognition tf_transformations
```

- `espeakng` 需要系统里装有 `espeak-ng`（`sudo apt install espeak-ng`），朗读才有声音
- `face_recognition` 依赖 dlib，第一次安装要编译一会儿
- 若 `pip` 报 `externally-managed-environment`，加 `--break-system-packages` 或改用虚拟环境
- chapt2 的 C++ 下载例程用到的 `cpp-httplib`（单头文件 v0.12.2）已随仓库完整收录，不用另外安装

## 通用构建

每个 `*_ws` 是独立工作空间：进到该目录 `colcon build`，再 source 对应的 `install/setup.bash`：

```bash
cd chapt2/chapt2_ws
colcon build
source install/setup.bash
```

- 只构建某个包：`colcon build --packages-select demo_cpp_pkg`
- **开多个终端跑节点时，每个终端都要先 source** `/opt/ros/jazzy/setup.bash` 和该工作空间的
  `install/setup.bash`（或直接写进 `~/.bashrc`）
- 编译产物（`build/`、`install/`、`log/`）都在 `.gitignore` 里，clone 下来是干净的源码

---

## chapt1 · CMake 基础

不涉及 ROS，先把 CMake 本身弄明白。

| 文件 | 说明 |
|---|---|
| `hello_world.cpp` | C++ 版 hello world（目标名 `learn_cmake`） |
| `hello_world.py` | Python 版 |
| `CMakeLists.txt` | 手写的构建脚本 |

```bash
cd chapt1
cmake -B build && cmake --build build
./build/learn_cmake        # Hello, World!
python3 hello_world.py     # Hello World!
```

---

## chapt2 · 节点与话题

`chapt2_ws/src/` 下两个功能包，同样的逻辑 C++ 和 Python 各写一遍。

**`demo_cpp_pkg`**

| 可执行文件 | 说明 |
|---|---|
| `cpp_node` | `rclcpp::Node` 最小节点 |
| `person_node` | 继承 `Node`，构造参数 + 成员函数 |
| `learn_auto` / `learn_shared_ptr` / `learn_lambda` / `learn_function` | C++ 语法练习（与 ROS 无关） |
| `learn_thread` | `std::thread` + `cpp-httplib` 多线程下载 |

**`demo_python_pkg`**

| 可执行文件 | 说明 |
|---|---|
| `python_node` | 最小节点 |
| `person_node` | 继承 `Node` |
| `writer_node` | 二次继承（父类构造 + 方法复用） |
| `learn_thread` | `threading` + `requests` 多线程下载 |

```bash
cd chapt2/chapt2_ws
colcon build && source install/setup.bash

ros2 run demo_cpp_pkg cpp_node         # 最小节点
ros2 run demo_python_pkg person_node   # 节点继承练习
ros2 run demo_cpp_pkg learn_auto       # C++ 语法练习，直接跑
```

两个 `learn_thread` 都从本地 HTTP 服务下载 `novel1~3.txt`（文件就在 `chapt2_ws/` 下），
所以先起服务，再跑节点：

```bash
# 终端 1：起本地 HTTP 服务
cd chapt2/chapt2_ws && python3 -m http.server 8000

# 终端 2
cd chapt2/chapt2_ws && source install/setup.bash
ros2 run demo_python_pkg learn_thread
ros2 run demo_cpp_pkg learn_thread
```

> `chapt2/` 根目录的 `ros2_cpp_node.cpp`、`ros2_python_node.py` 是最早不建功能包、手写 CMake 直接
> 编译节点的版本，留着作对照；现在统一走 `chapt2_ws`。

---

## chapt3 · 话题实战 + turtlesim 换脸

### `chapt3_ws` —— 话题实战

| 包 | 可执行文件 | 说明 |
|---|---|---|
| `demo_cpp_topic` | `turtle_circle` | 定时向 `/turtle1/cmd_vel` 发 Twist，让海龟画圆 |
| | `turtle_control` | 订阅 `/turtle1/pose`，追到目标点 |
| `demo_python_topic` | `novel_pub_node` | 下载小说，按行发布到 `novel` 话题 |
| | `novel_sub_node` | 订阅 `novel`，用 espeakng 朗读出来 |

```bash
# 终端 1：海龟模拟器
source /opt/ros/jazzy/setup.bash
ros2 run turtlesim turtlesim_node

# 终端 2：控制海龟（两个选一个跑，同时跑会抢 cmd_vel）
cd chapt3/chapt3_ws && colcon build && source install/setup.bash
ros2 run demo_cpp_topic turtle_circle      # 画圆
ros2 run demo_cpp_topic turtle_control     # 追目标点
```

小说朗读同样需要本地 HTTP 服务提供 `novel1.txt`（在 `chapt3_ws/` 下）：

```bash
# 终端 1
cd chapt3/chapt3_ws && python3 -m http.server 8000

# 终端 2：发布
cd chapt3/chapt3_ws && source install/setup.bash
ros2 run demo_python_topic novel_pub_node

# 终端 3：订阅并朗读（会真的出声，注意音量；新终端记得先 source）
cd chapt3/chapt3_ws && source install/setup.bash
ros2 run demo_python_topic novel_sub_node
```

### `turtlesim_face_ws` —— 给 turtlesim 换脸（自研）

把 turtlesim 里的小海龟换成 `face_source.jpg` 里的照片，尺寸和原版贴图一致（45×45 像素、1 米大小），
并且可以在「照片」和「原版小海龟」之间随时切换。

```bash
cd chapt3/turtlesim_face_ws
source /opt/ros/jazzy/setup.bash
colcon build --packages-select turtlesim   # 首次需要编译定制版 turtlesim
bash run_turtlesim.sh
```

切换贴图的三种方式：

- 在 Turtlesim 窗口里按 <kbd>T</kbd>
- `ros2 service call /toggle_skin std_srvs/srv/Empty "{}"`
- `ros2 param set /turtlesim use_face false`（`false` = 原版小海龟，`true` = 照片）

换别的照片、调整取景和实现细节见 [`chapt3/turtlesim_face_ws/README.md`](chapt3/turtlesim_face_ws/README.md)。

---

## chapt4 · 服务、参数与自定义接口

### `chapt4_interfaces` —— 自己定义的 `.srv`

```text
FaceDetector.srv
    sensor_msgs/Image image
    ---
    int16 number          # 检测到几张脸
    float32 use_time      # 耗时
    int32[] top / right / bottom / left   # 人脸框

Patrol.srv
    float32 target_x
    float32 target_y
    ---
    int8 SUCCESS=1
    int8 FAIL=0
    int8 result
```

### 海龟巡逻 + 参数

| 包 | 可执行文件 | 说明 |
|---|---|---|
| `demo_cpp_service` | `turtle_control` | 订阅 pose 追目标点；提供 `/patrol` 服务；`k`、`max_speed` 两个参数 + 参数回调 |
| | `patrol_client` | 每 10 秒随机请求一个目标点 |

```bash
# 终端 1：海龟模拟器
source /opt/ros/jazzy/setup.bash
ros2 run turtlesim turtlesim_node

# 终端 2：服务端
cd chapt4/chapt4_ws && colcon build && source install/setup.bash
ros2 run demo_cpp_service turtle_control

# 终端 3：客户端
cd chapt4/chapt4_ws && source install/setup.bash
ros2 run demo_cpp_service patrol_client

# 调参数（会触发参数回调，立即生效）
ros2 param list /turtle_controller
ros2 param set /turtle_controller k 2.0
ros2 param set /turtle_controller max_speed 2.0
```

### 人脸检测服务

| 包 | 可执行文件 | 说明 |
|---|---|---|
| `demo_python_service` | `face_detect_node` | `/face_detect` 服务端，用 `face_recognition` 检测；参数 `face_locations_model`（hog/cnn）、`face_locations_upsample_times` |
| | `face_detect_client_node` | 分别用 hog / cnn 模型各发一次请求，打印人脸数和耗时 |
| | `learn_face_detect` | 单张测试图检测后弹窗画框 |

```bash
# 终端 1：首次构建 + 服务端
cd chapt4/chapt4_ws && colcon build && source install/setup.bash
ros2 run demo_python_service face_detect_node

# 终端 2：客户端（新终端记得先 source）
cd chapt4/chapt4_ws && source install/setup.bash
ros2 run demo_python_service face_detect_client_node

# 单图检测，弹窗显示结果（需要图形界面）
ros2 run demo_python_service learn_face_detect
```

`learn_face_detect` 画中文标题需要 Noto CJK 字体（`fonts-noto-cjk`），图片资源已随包安装到 share 目录。

---

## chapt5 · TF2 坐标变换

`chapt5_ws/src/` 下两个包，分别用 C++ 和 Python 实现静态/动态坐标广播和 tf 查询。

### C++（`demo_cpp_tf`）

| 可执行文件 | 坐标系 | 说明 |
|---|---|---|
| `static_tf_broadcaster` | `map` → `target_point` | 静态变换：x=5.0，y=3.0，yaw=60° |
| `dynamic_tf_broadcaster` | `map` → `base_link` | 每 10 ms 发一次：x=2.0，y=3.0，yaw=30° |
| `tf_listener` | 查询 `base_link` → `target_point` | 每 5 秒打印平移和 RPY 欧拉角 |

### Python（`demo_python_tf`）

| 可执行文件 | 坐标系 | 说明 |
|---|---|---|
| `static_tf_broadcaster` | `base_link` → `camera_link` | 静态变换：x=0.5，y=0.3，z=0.6，roll=180° |
| `dynamic_tf_broadcaster` | `camera_link` → `bottle_link` | 10 Hz 发一次：z=0.5 |
| `tf_listener` | 查询 `base_link` → `bottle_link` | 10 Hz 打印平移和 RPY 欧拉角 |

跑起来（开三个终端，先起广播再起监听）：

```bash
# 终端 1：首次构建 + 静态广播
cd chapt5/chapt5_ws && colcon build && source install/setup.bash
ros2 run demo_cpp_tf static_tf_broadcaster

# 终端 2：动态广播（新终端记得先 source）
cd chapt5/chapt5_ws && source install/setup.bash
ros2 run demo_cpp_tf dynamic_tf_broadcaster

# 终端 3：监听（新终端记得先 source）
cd chapt5/chapt5_ws && source install/setup.bash
ros2 run demo_cpp_tf tf_listener
```

Python 版把包名换成 `demo_python_tf` 即可（两个包的坐标系是两套独立练习，建议分开跑，
同时跑两套坐标会混进同一棵 tf 树）。

辅助工具：

```bash
ros2 run tf2_tools view_frames    # 画出当前坐标树，生成 frames.pdf（需要 graphviz 的 dot）
rviz2                             # 加 TF 显示，可视化坐标关系
```

---

## 使用说明

1. **每个 `*_ws` 是独立工作空间**：进到该目录再 `colcon build`；多终端运行时每个终端都要
   `source <工作空间>/install/setup.bash`
2. 编译产物（`build/`、`install/`、`log/`）都被 `.gitignore` 排除，clone 下来是干净的源码
3. `.vscode/settings.json` 里的 `cmake.sourceDirectory` 目前是写死的绝对路径（指向旧目录
   `/home/xiongchengjun/chaptN/...`），换机器或移动仓库后要同步改；也可以改成相对写法，例如
   `"${workspaceFolder}/chapt2_ws/src/demo_cpp_pkg"`
4. 如果代码是从旧目录（如 `~/chapt2`）整体搬过来的，旧 `build/` 里的 CMake 缓存仍指向旧路径，
   增量 `colcon build` 会报 `CMakeCache.txt ... different than the directory ...`。把该工作空间的
   `build/`、`install/`、`log/` 删掉重新构建即可（都被 gitignore 忽略，随时能重建）
5. 各包自带 ament lint 测试（flake8 / pep257 / copyright），练习代码没有刻意对齐风格，
   `colcon test` 可能报 lint 失败，可忽略；`colcon test-result --verbose` 看详情

---

## 许可

学习代码，随意取用。
