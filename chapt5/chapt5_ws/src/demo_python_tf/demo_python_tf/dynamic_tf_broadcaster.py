import rclpy
from rclpy.node import Node
from tf2_ros import TransformBroadcaster
from geometry_msgs.msg import TransformStamped
from tf_transformations import quaternion_from_euler

class DynamicTFBroadcaster(Node):
    def __init__(self):
        super().__init__('dynamic_tf2_broadcaster')
        self.broadcaster = TransformBroadcaster(self)
        self.timer = self.create_timer(0.1, self.publish_dynamic_tf)  # Publish at 10 Hz
        self.angle = 0.0  # Initialize the angle for rotation

    def publish_dynamic_tf(self):
        t = TransformStamped()

        # Set the header information
        t.header.stamp = self.get_clock().now().to_msg()
        t.header.frame_id = "camera_link"  # Parent frame
        t.child_frame_id = "bottle_link"  # Child frame 

        # Set the translation (x, y, z)
        t.transform.translation.x = 0.2
        t.transform.translation.y = 0.0
        t.transform.translation.z = 0.5

        rotation_quat = quaternion_from_euler(0, 0, 0)  # Rotate around Z-axis
        t.transform.rotation.x = rotation_quat[0]
        t.transform.rotation.y = rotation_quat[1]
        t.transform.rotation.z = rotation_quat[2]
        t.transform.rotation.w = rotation_quat[3]

        # Broadcast the dynamic transform
        self.broadcaster.sendTransform(t)

def main():
    rclpy.init()
    dynamic_tf_broadcaster = DynamicTFBroadcaster()
    rclpy.spin(dynamic_tf_broadcaster)
    rclpy.shutdown()
  