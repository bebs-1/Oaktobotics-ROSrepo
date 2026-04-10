#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "sensor_msgs/msg/joint_state.hpp"
#include "my_robot_pkg/SparkMax.hpp"
#include <chrono>
#include <iomanip>
#include <memory>
#include <iostream>
#include <string>


class MotorDriver : public rclcpp::Node
{
public:
    MotorDriver() : Node("motor_driver"), motor1("can0", 1), motor2("can0", 2), 
                    motor3("can0", 3), motor4("can0", 4)
    {
        // Configure motors
        motor1.SetMotorType(MotorType::kBrushless);
        motor2.SetMotorType(MotorType::kBrushless);
        motor3.SetMotorType(MotorType::kBrushless);
        motor4.SetMotorType(MotorType::kBrushless);

        
        
        motor1.SetIdleMode(IdleMode::kBrake);
        motor2.SetIdleMode(IdleMode::kBrake);
        motor3.SetIdleMode(IdleMode::kBrake);
        motor4.SetIdleMode(IdleMode::kBrake);
        
        motor1.SetSensorType(SensorType::kHallSensor);
        motor2.SetSensorType(SensorType::kHallSensor);
        motor3.SetSensorType(SensorType::kHallSensor);
        motor4.SetSensorType(SensorType::kHallSensor);

        motor1.BurnFlash();
        motor2.BurnFlash();
        motor3.BurnFlash();
        motor4.BurnFlash();

        // Create subscriber for motor commands
        sub_ = this->create_subscription<std_msgs::msg::String>(
            "motor_cmd", 10,
            std::bind(&MotorDriver::cmd_callback, this, std::placeholders::_1));

        // Create publisher for motor data
        publisher_ = this->create_publisher<sensor_msgs::msg::JointState>(
            "motor_data", 10);

        // Create timer to publish motor data at 10 Hz
        timer_ = this->create_wall_timer(
            std::chrono::milliseconds(100),
            std::bind(&MotorDriver::publish_motor_data, this));
    }

private:
    SparkMax motor1;
    SparkMax motor2;
    SparkMax motor3;
    SparkMax motor4;

    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr sub_;
    rclcpp::Publisher<sensor_msgs::msg::JointState>::SharedPtr publisher_;
    rclcpp::TimerBase::SharedPtr timer_;

    void publish_motor_data()
    {
        auto message = sensor_msgs::msg::JointState();
        message.header.stamp = this->now();

        message.name     = {"motor1", "motor2", "motor3", "motor4"};
        message.velocity = {
            static_cast<double>(motor1.GetVelocity()),
            static_cast<double>(motor2.GetVelocity()),
            static_cast<double>(motor3.GetVelocity()),
            static_cast<double>(motor4.GetVelocity())
        };
        message.effort = {
            static_cast<double>(motor1.GetCurrent()),
            static_cast<double>(motor2.GetCurrent()),
            static_cast<double>(motor3.GetCurrent()),
            static_cast<double>(motor4.GetCurrent())
        };

        publisher_->publish(message);
        RCLCPP_DEBUG(this->get_logger(), "Published motor data");
    }

    void cmd_callback(const std_msgs::msg::String::SharedPtr msg)
    {
        std::string cmd = msg->data;
        RCLCPP_INFO(this->get_logger(), "Received: %s", cmd.c_str());
        SparkBase::Heartbeat();
        if(cmd == "forward") {
            motor1.SetDutyCycle(0.5);
            motor2.SetDutyCycle(0.5);
            motor3.SetDutyCycle(0.5);
            motor4.SetDutyCycle(0.5);
        }
        else if(cmd == "left") {
            motor1.SetDutyCycle(-0.5);
            motor2.SetDutyCycle(-0.5);
            motor3.SetDutyCycle(0.5);
            motor4.SetDutyCycle(0.5);
        }
        else if(cmd == "backward") {
            motor1.SetDutyCycle(-0.5);
            motor2.SetDutyCycle(-0.5);
            motor3.SetDutyCycle(-0.5);
            motor4.SetDutyCycle(-0.5);
        }
        else if(cmd == "right") {
            motor1.SetDutyCycle(0.5);
            motor2.SetDutyCycle(0.5);
            motor3.SetDutyCycle(-0.5);
            motor4.SetDutyCycle(-0.5);
        }
        else if(cmd == "stop") {
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

