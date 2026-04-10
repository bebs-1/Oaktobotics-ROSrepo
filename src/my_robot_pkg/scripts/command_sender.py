#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from std_msgs.msg import String
from sensor_msgs.msg import Joy
import time
import socket
import serial
import threading

HOST = "192.168.1.101"
PORT = 65432
SERIAL_PORT = "/dev/ttyUSB0"  # change from 'COM5' to your system device
BAUD = 9600

class MotorCommander(Node):
    def __init__(self):
        super().__init__('motor_commander')
        self.pub = self.create_publisher(String, 'motor_cmd', 10)
        self.create_subscription(Joy, 'joy', self.joy_callback, 10)
        self.buffer = b""
        self.running = True

        self.ser = None
        try:
            self.ser = serial.Serial(SERIAL_PORT, BAUD, timeout=1)
            time.sleep(2)
        except Exception as e:
            self.get_logger().error(f"Serial port open failed ({SERIAL_PORT}): {e}")
            self.get_logger().info("Running without serial output; command_sender stays up.")

        
        
    def destroy_node(self):
        self.get_logger().info("Shutting down CommandSender")
        self.running = False
        if self.thread.is_alive():
            self.thread.join(timeout=1.0)
        try:
            self.ser.close()
        except Exception:
            pass
        super().destroy_node()

    def joy_callback(self, msg: Joy):
        # axes[1]: left stick Y (forward/backward), axes[0]: left stick X (turn)
        DEADBAND = 0.2
        fwd = msg.axes[1] if len(msg.axes) > 1 else 0.0
        turn = msg.axes[0] if len(msg.axes) > 0 else 0.0

        cmd = String()
        if abs(fwd) > abs(turn):
            if fwd > DEADBAND:
                cmd.data = 'forward'
            elif fwd < -DEADBAND:
                cmd.data = 'backward'
            else:
                cmd.data = 'stop'
        else:
            if turn > DEADBAND:
                cmd.data = 'left'
            elif turn < -DEADBAND:
                cmd.data = 'right'
            else:
                cmd.data = 'stop'

        self.pub.publish(cmd)


def main(args=None):
    rclpy.init(args=args)
    node = MotorCommander()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == '__main__':
    main()
