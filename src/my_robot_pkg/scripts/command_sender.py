#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from std_msgs.msg import Int32MultiArray
from sensor_msgs.msg import Joy
import serial

SERIAL_PORT = "/dev/ttyS4"  # change from 'COM5' to your system device
BAUD = 9600

class MotorCommander(Node):
    def __init__(self):
        super().__init__('motor_commander')
        self.pub = self.create_publisher(Int32MultiArray, 'motor_cmd', 10)
        self.create_subscription(Joy, 'joy', self.joy_callback, 10)

        self.ser = None
        try:
            self.ser = serial.Serial(SERIAL_PORT, BAUD, timeout=1)
        except Exception as e:
            self.get_logger().error(f"Serial port open failed ({SERIAL_PORT}): {e}")
            self.get_logger().info("Running without serial output; command_sender stays up.")

    def destroy_node(self):
        self.get_logger().info("Shutting down CommandSender")
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
        act1 = msg.axes[6] if len(msg.axes) > 6 else 0.0
        act2 = msg.axes[7] if len(msg.axes) > 7 else 0.0

        cmd = Int32MultiArray()
        if abs(fwd) > abs(turn):
            if fwd > DEADBAND:
                cmd.data = [500, 500, 500, 500]  # forward    
            elif fwd < -DEADBAND:
                cmd.data = [-500, -500, -500, -500]  # backward
            else:
                cmd.data = [0, 0, 0, 0]  # stop
        else:
            if turn > DEADBAND:
                cmd.data = [500, 500, -500, -500]  # left
            elif turn < -DEADBAND:
                cmd.data = [-500, -500, 500, 500]  # right
            else:
                cmd.data = [0, 0, 0, 0]  # stop
        if(act1 > DEADBAND):
            self.ser.write(b'u1')  # Activate mechanism 1
        elif(act1 < -DEADBAND):
            self.ser.write(b'd1')  # Deactivate mechanism 1
        else:
            self.ser.write(b'stop')  # Stop mechanism 1
        if(act2 > DEADBAND):
            self.ser.write(b'u2')  # Activate mechanism 2
        elif(act2 < -DEADBAND):
            self.ser.write(b'd2')  # Deactivate mechanism 2
        else:
            self.ser.write(b'stop')  # Stop mechanism 2
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
