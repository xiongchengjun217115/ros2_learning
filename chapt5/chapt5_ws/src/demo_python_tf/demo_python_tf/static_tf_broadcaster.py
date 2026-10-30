import math
import rclpy
from rclpy.node import Node
from tf2_ros import StaticTransformBroadcaster
from geometry_msgs.msg import TransformStamped
from tf_transformations import quaternion_from_euler

class StaticTFBroadcaster(Node):
    def __init__(self):
        super().__init__('static_tf2_broadcaster')
        self.static_broadcaster = StaticTransformBroadcaster(self)
        self.publish_static_tf()

    def publish_static_tf(self):
        t = TransformStamped()

        # Set the header information
        t.header.stamp = self.get_clock().now().to_msg()
        t.header.frame_id = "base_link"  # Parent frame
        t.child_frame_id = "camera_link"  # Child frame 

        # Set the translation (x, y, z)
        t.transform.translation.x = 0.5
        t.transform.translation.y = 0.3
        t.transform.translation.z = 0.6

        rotation_quat=quaternion_from_euler(math.radians(180),0,0)
        t.transform.rotation.x = rotation_quat[0]
        t.transform.rotation.y = rotation_quat[1]
        t.transform.rotation.z = rotation_quat[2]
        t.transform.rotation.w = rotation_quat[3]

        # Broadcast the static transform
        self.static_broadcaster.sendTransform(t)
        self.get_logger().info(f'发布TF:{t}')

def main():
    rclpy.init()
    static_tf_broadcaster= StaticTFBroadcaster()
    rclpy.spin(static_tf_broadcaster)
    rclpy.shutdown()