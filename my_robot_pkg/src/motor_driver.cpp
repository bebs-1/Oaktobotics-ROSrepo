#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>
#include <memory>
#include <string>

// Simple motor control stub (replace with actual SparkMax when library is available)
class SimpleMotor
{
public:
    SimpleMotor(const std::string &interface, uint8_t id) 
        : interface_(interface), id_(id) {}
    
    void SetDutyCycle(double cycle) {
        RCLCPP_INFO(rclcpp::get_logger("motor"), 
            "Motor %u on %s: DutyCycle=%.2f", id_, interface_.c_str(), cycle);
    }
    
private:
    std::string interface_;
    uint8_t id_;
};

class MotorDriver : public rclcpp::Node
{
public:
    MotorDriver()
    : Node("motor_driver"),
      motor1("can0", 1),
      motor2("can0", 2),
      motor3("can0", 3),
      motor4("can0", 4)
    {
        sub_ = this->create_subscription<std_msgs::msg::String>(
            "motor_cmd", 10,
            std::bind(&MotorDriver::cmd_callback, this, std::placeholders::_1));

        RCLCPP_INFO(this->get_logger(), "MotorDriver initialized");
    }

private:
    SimpleMotor motor1;
    SimpleMotor motor2;
    SimpleMotor motor3;
    SimpleMotor motor4;

    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr sub_;

    void cmd_callback(const std_msgs::msg::String::SharedPtr msg)
    {
        std::string cmd = msg->data;
        RCLCPP_INFO(this->get_logger(), "Received: %s", cmd.c_str());

        if (cmd == "forward") {
            motor1.SetDutyCycle(0.5);
            motor2.SetDutyCycle(0.5);
            motor3.SetDutyCycle(0.5);
            motor4.SetDutyCycle(0.5);
        } else if (cmd == "left") {
            motor1.SetDutyCycle(-0.5);
            motor2.SetDutyCycle(-0.5);
            motor3.SetDutyCycle(0.5);
            motor4.SetDutyCycle(0.5);
        } else if (cmd == "backward") {
            motor1.SetDutyCycle(-0.5);
            motor2.SetDutyCycle(-0.5);
            motor3.SetDutyCycle(-0.5);
            motor4.SetDutyCycle(-0.5);
        } else if (cmd == "right") {
            motor1.SetDutyCycle(0.5);
            motor2.SetDutyCycle(0.5);
            motor3.SetDutyCycle(-0.5);
            motor4.SetDutyCycle(-0.5);
        } else if (cmd == "stop") {
            motor1.SetDutyCycle(0.0);
            motor2.SetDutyCycle(0.0);
            motor3.SetDutyCycle(0.0);
            motor4.SetDutyCycle(0.0);
        }
    }
};


int main(int argc, char **argv) {
  rclcpp::init(argc, argv);
  auto node = std::make_shared<MotorDriver>();
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}
