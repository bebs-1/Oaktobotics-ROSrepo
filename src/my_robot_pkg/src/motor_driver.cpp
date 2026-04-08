#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "std_msgs/msg/int32_multi_array.hpp"
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
        publisher_ = this->create_publisher<std_msgs::msg::Int32MultiArray>(
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
    rclcpp::Publisher<std_msgs::msg::Int32MultiArray>::SharedPtr publisher_;
    rclcpp::TimerBase::SharedPtr timer_;

    void publish_motor_data()
    {
        auto message = std_msgs::msg::Int32MultiArray();
        
        // Read motor data and store as integers
        int vel1 = static_cast<int>(motor1.GetVelocity());
        int vel2 = static_cast<int>(motor2.GetVelocity());
        int vel3 = static_cast<int>(motor3.GetVelocity());
        int vel4 = static_cast<int>(motor4.GetVelocity());
        
        int curr1 = static_cast<int>(motor1.GetCurrent() * 1000);  // Convert to mA
        int curr2 = static_cast<int>(motor2.GetCurrent() * 1000);
        int curr3 = static_cast<int>(motor3.GetCurrent() * 1000);
        int curr4 = static_cast<int>(motor4.GetCurrent() * 1000);

        // Populate data array: [vel1, curr1, vel2, curr2, vel3, curr3, vel4, curr4]
        message.data = {vel1, curr1, vel2, curr2, vel3, curr3, vel4, curr4};

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

