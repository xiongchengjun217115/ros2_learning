// Copyright (c) 2009, Willow Garage, Inc.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright
//      notice, this list of conditions and the following disclaimer.
//
//    * Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
//
//    * Neither the name of the Willow Garage nor the names of its
//      contributors may be used to endorse or promote products derived from
//      this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#include "turtlesim/turtle_frame.hpp"

#include <QKeyEvent>
#include <QPointF>

#include <cstdlib>
#include <ctime>
#include <functional>
#include <string>

#include "rcl_interfaces/msg/integer_range.hpp"
#include "rcl_interfaces/msg/parameter_descriptor.hpp"
#include "rcl_interfaces/msg/parameter_event.hpp"
#include "rcl_interfaces/msg/set_parameters_result.hpp"
#include "rclcpp/rclcpp.hpp"
#include "std_srvs/srv/empty.hpp"

#include "turtlesim/srv/kill.hpp"
#include "turtlesim/srv/spawn.hpp"

#define DEFAULT_BG_R 0x45
#define DEFAULT_BG_G 0x56
#define DEFAULT_BG_B 0xff

namespace turtlesim
{

TurtleFrame::TurtleFrame(rclcpp::Node::SharedPtr & node_handle, QWidget * parent, Qt::WindowFlags f)
: QFrame(parent, f)
  , path_image_(500, 500, QImage::Format_ARGB32)
  , path_painter_(&path_image_)
  , frame_count_(0)
  , id_counter_(0)
{
  setFixedSize(500, 500);
  setWindowTitle("TurtleSim  (press T to toggle skin)");
  setFocusPolicy(Qt::StrongFocus);

  srand(time(NULL));

  update_timer_ = new QTimer(this);
  update_timer_->setInterval(16);
  update_timer_->start();

  connect(update_timer_, SIGNAL(timeout()), this, SLOT(onUpdate()));

  nh_ = node_handle;
  rcl_interfaces::msg::IntegerRange range;
  range.from_value = 0;
  range.step = 1;
  range.to_value = 255;
  rcl_interfaces::msg::ParameterDescriptor background_r_descriptor;
  background_r_descriptor.description = "Red channel of the background color";
  background_r_descriptor.integer_range.push_back(range);
  rcl_interfaces::msg::ParameterDescriptor background_g_descriptor;
  background_g_descriptor.description = "Green channel of the background color";
  background_g_descriptor.integer_range.push_back(range);
  rcl_interfaces::msg::ParameterDescriptor background_b_descriptor;
  background_b_descriptor.description = "Blue channel of the background color";
  background_b_descriptor.integer_range.push_back(range);
  nh_->declare_parameter(
    "background_r", rclcpp::ParameterValue(
      DEFAULT_BG_R), background_r_descriptor);
  nh_->declare_parameter(
    "background_g", rclcpp::ParameterValue(
      DEFAULT_BG_G), background_g_descriptor);
  nh_->declare_parameter(
    "background_b", rclcpp::ParameterValue(
      DEFAULT_BG_B), background_b_descriptor);

  rcl_interfaces::msg::ParameterDescriptor holonomic_descriptor;
  holonomic_descriptor.description = "If true, then turtles will be holonomic";
  nh_->declare_parameter("holonomic", rclcpp::ParameterValue(false), holonomic_descriptor);

  rcl_interfaces::msg::ParameterDescriptor use_face_descriptor;
  use_face_descriptor.description =
    "Draw the custom face image instead of the classic turtle sprite; also switchable with the "
    "T key in the turtlesim window and the ~/toggle_skin service";
  nh_->declare_parameter("use_face", rclcpp::ParameterValue(true), use_face_descriptor);

  QVector<QString> turtles;
  turtles.append("ardent.png");
  turtles.append("bouncy.png");
  turtles.append("crystal.png");
  turtles.append("dashing.png");
  turtles.append("eloquent.png");
  turtles.append("foxy.png");
  turtles.append("galactic.png");
  turtles.append("humble.png");
  turtles.append("iron.png");
  turtles.append("jazzy.png");
  turtles.append("rolling.png");

  QString images_path =
    (ament_index_cpp::get_package_share_directory("turtlesim") + "/images/").c_str();
  for (int i = 0; i < turtles.size(); ++i) {
    QImage img;
    img.load(images_path + turtles[i]);
    turtle_images_.append(img);
  }

  alt_image_.load(images_path + "face.png");
  if (alt_image_.isNull()) {
    RCLCPP_WARN(
      nh_->get_logger(), "Could not load %sface.png, keeping classic turtle sprites",
      images_path.toStdString().c_str());
  } else {
    alt_skin_ = nh_->get_parameter("use_face").as_bool();
  }

  meter_ = turtle_images_[0].height();

  clear();

  clear_srv_ =
    nh_->create_service<std_srvs::srv::Empty>(
    "clear",
    std::bind(&TurtleFrame::clearCallback, this, std::placeholders::_1, std::placeholders::_2));
  reset_srv_ =
    nh_->create_service<std_srvs::srv::Empty>(
    "reset",
    std::bind(&TurtleFrame::resetCallback, this, std::placeholders::_1, std::placeholders::_2));
  spawn_srv_ =
    nh_->create_service<turtlesim::srv::Spawn>(
    "spawn",
    std::bind(&TurtleFrame::spawnCallback, this, std::placeholders::_1, std::placeholders::_2));
  kill_srv_ =
    nh_->create_service<turtlesim::srv::Kill>(
    "kill",
    std::bind(&TurtleFrame::killCallback, this, std::placeholders::_1, std::placeholders::_2));
  toggle_skin_srv_ =
    nh_->create_service<std_srvs::srv::Empty>(
    "toggle_skin",
    std::bind(&TurtleFrame::toggleSkinCallback, this, std::placeholders::_1, std::placeholders::_2));

  rclcpp::QoS qos(rclcpp::KeepLast(100), rmw_qos_profile_sensor_data);
  parameter_event_sub_ = nh_->create_subscription<rcl_interfaces::msg::ParameterEvent>(
    "/parameter_events", qos,
    std::bind(&TurtleFrame::parameterEventCallback, this, std::placeholders::_1));
  parameter_callback_handle_ = nh_->add_on_set_parameters_callback(
    [this](const std::vector<rclcpp::Parameter> & parameters) {
      rcl_interfaces::msg::SetParametersResult result;
      result.successful = true;
      for (const auto & parameter : parameters) {
        if (parameter.get_name() == "use_face") {
          applyAltSkin(parameter.as_bool());
        }
      }
      return result;
    });

  RCLCPP_INFO(
    nh_->get_logger(), "Starting turtlesim with node name %s", nh_->get_fully_qualified_name());

  width_in_meters_ = (width() - 1) / meter_;
  height_in_meters_ = (height() - 1) / meter_;
  spawnTurtle("", width_in_meters_ / 2.0, height_in_meters_ / 2.0, 0);

  // spawn all available turtle types
  if (false) {
    for (int index = 0; index < turtles.size(); ++index) {
      QString name = turtles[index];
      name = name.split(".").first();
      name.replace(QString("-"), QString(""));
      spawnTurtle(
        name.toStdString(), 1.0f + 1.5f * (index % 7), 1.0f + 1.5f * (index / 7),
        static_cast<float>(PI) / 2.0f, index);
    }
  }
}

TurtleFrame::~TurtleFrame()
{
  delete update_timer_;
}

bool TurtleFrame::spawnCallback(
  const turtlesim::srv::Spawn::Request::SharedPtr req,
  turtlesim::srv::Spawn::Response::SharedPtr res)
{
  std::string name = spawnTurtle(req->name, req->x, req->y, req->theta);
  if (name.empty()) {
    RCLCPP_ERROR(nh_->get_logger(), "A turtle named [%s] already exists", req->name.c_str());
    return false;
  }

  res->name = name;

  return true;
}

bool TurtleFrame::killCallback(
  const turtlesim::srv::Kill::Request::SharedPtr req,
  turtlesim::srv::Kill::Response::SharedPtr)
{
  M_Turtle::iterator it = turtles_.find(req->name);
  if (it == turtles_.end()) {
    RCLCPP_ERROR(
      nh_->get_logger(), "Tried to kill turtle [%s], which does not exist", req->name.c_str());
    return false;
  }

  turtles_.erase(it);
  update();

  return true;
}

void TurtleFrame::parameterEventCallback(
  const rcl_interfaces::msg::ParameterEvent::ConstSharedPtr event)
{
  // only consider events from this node
  if (event->node == nh_->get_fully_qualified_name()) {
    // since parameter events for this event aren't expected frequently just always call update()
    update();
  }
}

bool TurtleFrame::hasTurtle(const std::string & name)
{
  return turtles_.find(name) != turtles_.end();
}

void TurtleFrame::keyPressEvent(QKeyEvent * event)
{
  if (event->key() == Qt::Key_T) {
    setAltSkin(!alt_skin_);
    return;
  }
  QFrame::keyPressEvent(event);
}

bool TurtleFrame::toggleSkinCallback(
  const std_srvs::srv::Empty::Request::SharedPtr,
  const std_srvs::srv::Empty::Response::SharedPtr)
{
  setAltSkin(!alt_skin_);
  return true;
}

void TurtleFrame::setAltSkin(bool enabled)
{
  applyAltSkin(enabled);
  if (nh_->has_parameter("use_face") && nh_->get_parameter("use_face").as_bool() != enabled) {
    nh_->set_parameter(rclcpp::Parameter("use_face", enabled));
  }
}

void TurtleFrame::applyAltSkin(bool enabled)
{
  if (alt_image_.isNull()) {
    RCLCPP_WARN(nh_->get_logger(), "No alternate turtle image loaded, keeping classic sprites");
    return;
  }
  if (alt_skin_ == enabled) {
    return;
  }
  alt_skin_ = enabled;
  for (auto & entry : turtles_) {
    entry.second->setAltSkin(alt_skin_);
  }
  RCLCPP_INFO(nh_->get_logger(), "Turtle skin: %s", alt_skin_ ? "photo" : "classic");
  update();
}

std::string TurtleFrame::spawnTurtle(const std::string & name, float x, float y, float angle)
{
  return spawnTurtle(name, x, y, angle, rand() % turtle_images_.size());
}

std::string TurtleFrame::spawnTurtle(
  const std::string & name, float x, float y, float angle,
  size_t index)
{
  std::string real_name = name;
  if (real_name.empty()) {
    do{
      std::stringstream ss;
      ss << "turtle" << ++id_counter_;
      real_name = ss.str();
    } while (hasTurtle(real_name));
  } else {
    if (hasTurtle(real_name)) {
      return "";
    }
  }

  TurtlePtr t = std::make_shared<Turtle>(
    nh_, real_name, turtle_images_[static_cast<int>(index)], alt_image_, QPointF(
      x,
      height_in_meters_ - y), angle);
  t->setAltSkin(alt_skin_);
  turtles_[real_name] = t;
  update();

  RCLCPP_INFO(
    nh_->get_logger(), "Spawning turtle [%s] at x=[%f], y=[%f], theta=[%f]",
    real_name.c_str(), x, y, angle);

  return real_name;
}

void TurtleFrame::clear()
{
  // make all pixels fully transparent
  path_image_.fill(qRgba(255, 255, 255, 0));
  update();
}

void TurtleFrame::onUpdate()
{
  if (!rclcpp::ok()) {
    close();
    return;
  }

  rclcpp::spin_some(nh_);

  updateTurtles();
}

void TurtleFrame::paintEvent(QPaintEvent * event)
{
  (void)event;  // NO LINT
  QPainter painter(this);

  int r = DEFAULT_BG_R;
  int g = DEFAULT_BG_G;
  int b = DEFAULT_BG_B;
  nh_->get_parameter("background_r", r);
  nh_->get_parameter("background_g", g);
  nh_->get_parameter("background_b", b);
  QRgb background_color = qRgb(r, g, b);
  painter.fillRect(0, 0, width(), height(), background_color);

  painter.drawImage(QPoint(0, 0), path_image_);

  M_Turtle::iterator it = turtles_.begin();
  M_Turtle::iterator end = turtles_.end();
  for (; it != end; ++it) {
    it->second->paint(painter);
  }
}

void TurtleFrame::updateTurtles()
{
  if (last_turtle_update_.nanoseconds() == 0) {
    last_turtle_update_ = nh_->now();
    return;
  }

  bool modified = false;
  M_Turtle::iterator it = turtles_.begin();
  M_Turtle::iterator end = turtles_.end();
  for (; it != end; ++it) {
    modified |= it->second->update(
      0.001 * update_timer_->interval(), path_painter_, path_image_, width_in_meters_,
      height_in_meters_);
  }
  if (modified) {
    update();
  }

  ++frame_count_;
}


bool TurtleFrame::clearCallback(
  const std_srvs::srv::Empty::Request::SharedPtr,
  std_srvs::srv::Empty::Response::SharedPtr)
{
  RCLCPP_INFO(nh_->get_logger(), "Clearing turtlesim.");
  clear();
  return true;
}

bool TurtleFrame::resetCallback(
  const std_srvs::srv::Empty::Request::SharedPtr,
  std_srvs::srv::Empty::Response::SharedPtr)
{
  RCLCPP_INFO(nh_->get_logger(), "Resetting turtlesim.");
  turtles_.clear();
  id_counter_ = 0;
  spawnTurtle("", width_in_meters_ / 2.0, height_in_meters_ / 2.0, 0);
  clear();
  return true;
}

}  // namespace turtlesim
