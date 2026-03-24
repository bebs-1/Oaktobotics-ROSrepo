#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from std_msgs.msg import String
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
        self.buffer = b""
        self.running = True

        self.ser = serial.Serial(SERIAL_PORT, BAUD, timeout=1)
        time.sleep(2)

        # Start socket thread
        self.thread = threading.Thread(target=self.socket_loop, daemon=True)
        self.thread.start()

    def socket_loop(self):
        try:
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.connect((HOST, PORT))
                s.sendall(b"Hello, world\n")
                while self.running:
                    data = s.recv(1024)
                    if not data:
                        time.sleep(0.05)
                        continue
                    self.buffer += data
                    while b"\n" in self.buffer:
                        line, self.buffer = self.buffer.split(b"\n", 1)
                        # write to serial
                        try:
                            self.ser.write(line + b"\n")
                        except Exception as e:
                            self.get_logger().error(f"Serial write failed: {e}")

                        # publish the line as a ROS message for other nodes
                        msg = String()
                        msg.data = line.decode('utf-8', errors='replace')
                        self.pub.publish(msg)

                        if line == b"close":
                            self.get_logger().info("Received 'close' command, shutting down socket loop")
                            self.running = False
                            break
        except Exception as e:
            self.get_logger().error(f"Socket error: {e}")

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
