import rclpy
from rclpy.node import Node
from tf2_ros import TransformListener, Buffer
from tf_transformations import euler_from_quaternion

class TFListener(Node):
    def __init__(self):
        super().__init__('tf_listener')
        self.tf_buffer = Buffer()
        self.tf_listener = TransformListener(self.tf_buffer, self)
        self.timer = self.create_timer(0.1, self.lookup_transform)  # Check for transform at 10 Hz

    def lookup_transform(self):
        try:
            # Look up the transform from 'base_link' to 'bottle_link'
            transform = self.tf_buffer.lookup_transform('base_link', 'bottle_link', rclpy.time.Time(seconds=0), timeout=rclpy.time.Duration(seconds=1))
            translation = transform.transform.translation
            rotation = transform.transform.rotation

            # Convert quaternion to Euler angles
            euler_angles = euler_from_quaternion([rotation.x, rotation.y, rotation.z, rotation.w])

            self.get_logger().info(f'Translation: x={translation.x}, y={translation.y}, z={translation.z}')
            self.get_logger().info(f'Rotation (Euler angles): roll={euler_angles[0]}, pitch={euler_angles[1]}, yaw={euler_angles[2]}')

        except Exception as e:
            self.get_logger().warn(f'Could not get transform: {e}')

def main():
    rclpy.init()
    tf_listener = TFListener()
    rclpy.spin(tf_listener)
    rclpy.shutdown()
    