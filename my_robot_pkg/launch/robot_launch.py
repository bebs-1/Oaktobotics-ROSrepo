import launch
from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    command_sender_node = Node(
        package='my_robot_pkg',
        executable='command_sender.py',
        name='motor_commander'
    )

    motor_driver_node = Node(
        package='my_robot_pkg',
        executable='motor_driver',
        name='motor_driver'
    )

    return LaunchDescription([
        command_sender_node,
        motor_driver_node
    ])