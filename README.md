# ROS 2 学习

跟着教程逐章敲下来的 ROS 2 代码，从手写 CMake 一路做到自定义服务接口 + 巡游客户端。

- **环境**：Ubuntu 24.04 + **ROS 2 Jazzy**
- **构建**：`colcon`（每个 `*_ws` 是一个独立工作空间）
- **语言**：C++ 与 Python 双份对照

---

## 目录

```
chapt1/   CMake 基础 —— 不依赖 ROS，手写 CMakeLists 编译 hello world
chapt2/   话题通信 —— 发布/订阅、多线程、C++ 新特性练习
chapt3/   服务与参数 —— 话题实战 + 给 turtlesim 换脸（自研功能）
chapt4/   自定义接口 —— 自己写 .srv，实现海龟巡逻与人脸检测服务
```

---

## chapt1 · CMake 基础

不涉及 ROS，先把 CMake 本身弄明白。

| 文件 | 说明 |
|---|---|
| `hello_world.cpp` | C++ 版 hello world |
| `hello_world.py` | Python 版 |
| `CMakeLists.txt` | 手写的构建脚本 |

```bash
cd chapt1
cmake -B build && cmake --build build
```

---

## chapt2 · 话题通信

`chapt2_ws/src/` 下两个功能包，C++ 和 Python 各写一遍同样的逻辑。

**`demo_python_pkg`**

| 节点 | 说明 |
|---|---|
| `person_node.py` | 自定义消息的发布/订阅 |
| `writer_node.py` | 话题写入 |
| `python_node.py` | 基础节点骨架 |
| `learn_thread.py` | Python 多线程 |

**`demo_cpp_pkg`**

| 文件 | 说明 |
|---|---|
| `cpp_node.cpp` / `person_node.cpp` | 发布订阅 |
| `learn_lambda.cpp` | lambda 练习 |
| `learn_function.cpp` | `std::function` 练习 |
| `learn_auto.cpp` | `auto` 类型推导 |
| `learn_shared_ptr.cpp` | 智能指针 |
| `learn_thread.cpp` | 多线程（用到 `include/cpp-httplib/httplib.h`） |

> `include/cpp-httplib/` 只保留了编译需要的 `httplib.h`（单个头文件库），
> 上游仓库里的测试语料库（6MB）没有收进来。

```bash
cd chapt2/chapt2_ws
colcon build && source install/setup.bash
ros2 run demo_python_pkg person_node
```

---

## chapt3 · 服务与参数

**`chapt3_ws/src/`** —— 话题实战

| 包 | 说明 |
|---|---|
| `demo_cpp_topic` | `turtle_circle.cpp`（画圆）、`turtle_control.cpp`（控制） |
| `demo_python_topic` | Python 版对应实现 |

**`turtlesim_face_ws/`** —— 自研：**把 turtlesim 里的小海龟换成自己的照片**

把 `face_source.jpg` 换成贴图，尺寸与官方一致（45×45 像素、1 米），支持随时切回原版小海龟。

```bash
bash chapt3/turtlesim_face_ws/run_turtlesim.sh
```

切换贴图的三种方式：

- Turtlesim 窗口里按 <kbd>T</kbd>
- `ros2 service call /toggle_skin std_srvs/srv/Empty "{}"`
- `ros2 param set /turtlesim use_face false`（`false` = 原版，`true` = 照片）

细节见 `chapt3/turtlesim_face_ws/README.md`。

---

## chapt4 · 自定义接口 + 服务

**`chapt4_interfaces`** —— 自己定义的 `.srv`

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

**`demo_cpp_service`** —— `turtle_control.cpp`、`patrol_client.cpp`（让海龟按坐标巡游）

**`demo_python_service`** —— `face_detect_node.py`、`face_detect_client_node.py`（人脸检测服务端/客户端）

```bash
cd chapt4/chapt4_ws
colcon build && source install/setup.bash
ros2 run demo_python_service face_detect_node
```

---

## 使用说明

1. **每个 `*_ws` 是独立工作空间**，进到该目录再 `colcon build`
2. 编译产物（`build/` `install/` `log/`）都已被 `.gitignore` 排除，clone 下来是干净的源码
3. `.vscode/settings.json` 里的路径用 `${workspaceFolder}` 写的，用 VS Code 直接打开 `chaptN` 目录即可
4. 依赖：`ros-jazzy-desktop`（含 turtlesim）、`cv_bridge`（chapt4 人脸检测）、`std_srvs`

---

## 许可

学习代码，随意取用。
