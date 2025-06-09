#!/usr/bin/env python3
"""
Launch file for the Extended Kalman Filter (EKF) node in Gazebo simulation.
"""
 
import os
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare
 
 
def generate_launch_description():

    # Create the launch description and populate
    ld = LaunchDescription()

    
    ### EKF node launch configuration.
    
    """
    # Constants for paths to different files and folders
    package_name = 'imu_triplex'
 
    # Config file paths
    ekf_config_file_path = 'config/ekf.yaml'
 
    # Set the path to different packages
    pkg_share = FindPackageShare(package=package_name).find(package_name)
 
    # Set the path to config files
    default_ekf_config_path = os.path.join(pkg_share, ekf_config_file_path)
 
    # Launch configuration variables
    ekf_config_file = LaunchConfiguration('ekf_config_file')
    use_sim_time = LaunchConfiguration('use_sim_time')
 
    # Declare the launch arguments
    declare_ekf_config_file_cmd = DeclareLaunchArgument(
        name='ekf_config_file',
        default_value=default_ekf_config_path,
        description='Full path to the EKF configuration YAML file'
    )
 
    declare_use_sim_time_cmd = DeclareLaunchArgument(
        name='use_sim_time',
        default_value='true',
        description='Use simulation (Gazebo) clock if true'
    )
 
    # Specify the actions
    start_ekf_node_cmd = Node(
        package='robot_localization',
        executable='ekf_node',
        name='ekf_filter_node',
        output='screen',
        parameters=[default_ekf_config_path,
            {'use_sim_time': use_sim_time}
        ]
    )
 
    # Add the declarations
    ld.add_action(declare_ekf_config_file_cmd)
    ld.add_action(declare_use_sim_time_cmd)
 
    # Add the actions
    ld.add_action(start_ekf_node_cmd)

"""


    
    # Launch main node
    main_node_cmd = Node(
        package='imu_triplex',
        executable='i2c_main',
        name='i2c_main_node',
        output='screen',
        parameters=[{'use_sim_time': use_sim_time}]
    )
    ld.add_action(main_node_cmd)



    # Launch madgwick filter node
    madgwick_node_cmd = Node(
        package='imu_filter_madgwick',
        executable='imu_filter_madgwick_node',
        name='madgwick_filter_node',
        output='screen',
        parameters=[{'use_mag': false}]
    )

    return ld
