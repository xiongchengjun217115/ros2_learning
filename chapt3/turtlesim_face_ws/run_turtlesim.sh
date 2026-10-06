#!/usr/bin/env bash
# 启动带自定义贴图的 turtlesim。
# 窗口里按 T（或调用 ~/toggle_skin 服务 / 改 use_face 参数）可在照片和原版小海龟之间切换。
set -e

WS_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

if pgrep -f "turtlesim_node" > /dev/null; then
  echo "提示：已经有一个 turtlesim_node 在运行，两个实例都会用 /turtle1 话题，建议先关掉旧窗口。" >&2
fi

source /opt/ros/jazzy/setup.bash
source "${WS_DIR}/install/setup.bash"

exec ros2 run turtlesim turtlesim_node "$@"
