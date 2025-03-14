#include <chrono>
#include <functional>
#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/laser_scan.hpp"
#include "std_msgs/msg/string.hpp"
#include "geometry_msgs/msg/twist.hpp"

class Lidar_stop_cmd2 : public rclcpp::Node {

public:
  Lidar_stop_cmd2():Node("Lidar_stop_cmd2") {
    laser_sub = this->create_subscription<sensor_msgs::msg::LaserScan>(
        "scan", rclcpp::SensorDataQoS(), std::bind(&Lidar_stop_cmd2::scan_callback, this, std::placeholders::_1));

    cmd_vel_= this -> create_publisher<geometry_msgs::msg::Twist>("cmd_vel",10);
    
    cmd_vel_sub_ = this -> create_subscription<geometry_msgs::msg::Twist>(
        "cmd_vel_smooth",10, std::bind(&Lidar_stop_cmd2::cmd_vel_smoothCb,this,std::placeholders::_1)
        );

    timer_ = this -> create_wall_timer(
        std::chrono::milliseconds(2000),
        std::bind(&Lidar_stop_cmd2::check_stop_condition, this)
        );

  }
public:
  void scan_callback(const sensor_msgs::msg::LaserScan::SharedPtr msg){
    lidar_data_ = *msg;
    process_data();
  }

  void cmd_vel_smoothCb(const geometry_msgs::msg::Twist::SharedPtr msg){
    cmd_vel_smooth_ = *msg;
    process_data();
  }

public:
  void process_data(){
    
    int count = lidar_data_.scan_time/ lidar_data_.time_increment;
    auto cmd_msg = geometry_msgs::msg::Twist();
    for(int i=0;i<count;i++){
      if(lidar_data_.ranges[i] > lidar_data_.range_min && lidar_data_.ranges[i] <0.3 && have_published == false && cmd_vel_smooth_.linear.x!=0.0 &&cmd_vel_smooth_.angular.z!=0.0){
        cmd_msg.linear.x = 0.0;
        cmd_msg.angular.z = 0.0;
        cmd_vel_ -> publish(cmd_msg);
        
        RCLCPP_INFO(this -> get_logger(), "STOP cmd_vel published (x=0, z=0)!");
        have_published = true;
      }
      else{
        cmd_msg.linear.x = cmd_vel_smooth_.linear.x;
        cmd_msg.angular.z = cmd_vel_smooth_.angular.z;
      }
    }
  
  }

  void check_stop_condition(){
    have_published = false;
  }
  geometry_msgs::msg::Twist cmd_vel_smooth_;
  sensor_msgs::msg::LaserScan lidar_data_;
  bool have_published = false;
  rclcpp::Subscription<sensor_msgs::msg::LaserScan>::SharedPtr laser_sub;
  rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr cmd_vel_;
  rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr cmd_vel_sub_;
  rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char **argv){
  rclcpp::init(argc,argv);
  std::shared_ptr<Lidar_stop_cmd2> node = std::make_shared<Lidar_stop_cmd2>();

  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}






