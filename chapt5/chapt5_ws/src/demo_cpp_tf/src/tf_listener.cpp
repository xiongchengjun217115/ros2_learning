#include <memory>
#include "geometry_msgs/msg/transform_stamped.hpp"
#include "rclcpp/rclcpp.hpp"
#include "tf2/LinearMath/Quaternion.h"
#include "tf2/utils.h"
#include "tf2_geometry_msgs/tf2_geometry_msgs.hpp"
#include "tf2_ros/buffer.h"
#include "tf2_ros/transform_listener.h"
#include <chrono>
using namespace std::chrono_literals;

class TfListener : public rclcpp::Node
{
public:
  TfListener() : Node("tf_listener")
    {
        buffer_ = std::make_shared<tf2_ros::Buffer>(this->get_clock());
        listener_ = std::make_shared<tf2_ros::TransformListener>(*buffer_);
        timer_ = this->create_wall_timer(5s, std::bind(&TfListener::getTransform, this));
    }

    void getTransform()
        {
            try
            {
                //等待变换可用
                const auto transform= buffer_->lookupTransform("base_link", "target_point", this->get_clock()->now(), rclcpp::Duration::from_seconds(1.0f));
                //转换结果及输出
                const auto &translation = transform.transform.translation;
                const auto &rotation = transform.transform.rotation;
                double roll, pitch, yaw;
                tf2::getEulerYPR(rotation, yaw, pitch, roll);
                RCLCPP_INFO(this->get_logger(), "Translation: x=%.2f, y=%.2f, z=%.2f", translation.x, translation.y, translation.z);
                RCLCPP_INFO(this->get_logger(), "Rotation: roll=%.2f, pitch=%.2f, yaw=%.2f", roll, pitch, yaw);
                //这里不用%f而用%.2f是为了控制输出的精度，避免输出过多的小数位，保持信息简洁明了，但是课本就是一坨
            }
            catch (tf2::TransformException & ex)
            {
                RCLCPP_WARN(this->get_logger(), "Could not transform: %s", ex.what());
            }
        }
    private:
        std::shared_ptr<tf2_ros::Buffer> buffer_;
        std::shared_ptr<tf2_ros::TransformListener> listener_;
        rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<TfListener>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}