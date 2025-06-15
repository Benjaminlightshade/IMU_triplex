#include <iostream>
#include <thread>
#include <iomanip> 
#include <chrono>

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/imu.hpp" 

// #include "test_lib.h"
#include "i2c/i2c.h"
#include "imu_ICM20948.h"

class ImuPublisherNode : public rclcpp::Node
{
public:
    ImuPublisherNode() : Node("imu_publisher_node")
    {
        // Initialize the IMU sensor

        imu_ = std::make_unique<imu_ICM20948>(0x68, "/dev/i2c-1");
        imu_->identify();
        imu_->init_imu();
        RCLCPP_INFO(this->get_logger(), "IMU initialized successfully.");
        
        // Create the publisher for IMU data. 
        // Topic gets the data and publishes every 10ms. 
        // This is aligned with the IMU's output rate, specifically the magnetometer. 
        publisher_ = this->create_publisher<sensor_msgs::msg::Imu>("imu/data_raw", 10);
        
        // Create a timer to publish IMU readings periodically (e.g., every 100ms for 10Hz)
        // Adjust the duration based on your desired publishing rate.
        timer_ = this->create_wall_timer(
            std::chrono::milliseconds(10), // Publish every 10ms 
            std::bind(&ImuPublisherNode::publish_imu_data, this));

        RCLCPP_INFO(this->get_logger(), "IMU publisher node started. Publishing to /imu/data_raw topic.");
    }

    ~ImuPublisherNode()
    {
        RCLCPP_INFO(this->get_logger(), "Shutting down IMU publisher node.");
    }

private:
    void publish_imu_data()
    {
        // Get the latest IMU readings
        imu_readings readings = imu_->get_imu_readings();
        
        // Create a new IMU message
        sensor_msgs::msg::Imu imu_msg;

        // Set the header
        imu_msg.header.stamp = this->now(); // Current ROS 2 time
        imu_msg.header.frame_id = "imu_link"; // A descriptive frame ID for your IMU

        // Populate linear acceleration (assuming readings are in m/s^2)
        imu_msg.linear_acceleration.x = readings.accel_x;
        imu_msg.linear_acceleration.y = readings.accel_y;
        imu_msg.linear_acceleration.z = readings.accel_z;

        // Populate angular velocity (assuming readings are in rad/s)
        imu_msg.angular_velocity.x = readings.gyro_x;
        imu_msg.angular_velocity.y = readings.gyro_y;
        imu_msg.angular_velocity.z = readings.gyro_z;

        // Orientation: If your IMU doesn't provide quaternions, you might leave these
        // as zeros or use a separate sensor fusion library to estimate orientation.
        // If orientation is not being reported, set the first element of the covariance to -1.
        imu_msg.orientation.x = 0.0; 
        imu_msg.orientation.y = 0.0;
        imu_msg.orientation.z = 0.0;
        imu_msg.orientation.w = 1.0; // Identity quaternion
        imu_msg.orientation_covariance[0] = -1.0; // Indicates orientation is not reported

        // Publish the IMU message
        publisher_->publish(imu_msg);

        // Optional: Log the readings for debugging
        // RCLCPP_INFO(this->get_logger(), 
        //             "Published Accel: (%.2f, %.2f, %.2f), Gyro: (%.2f, %.2f, %.2f)",
        //             readings.accel_x, readings.accel_y, readings.accel_z,
        //             readings.gyro_x, readings.gyro_y, readings.gyro_z);
    }

    rclcpp::TimerBase::SharedPtr timer_;
    rclcpp::Publisher<sensor_msgs::msg::Imu>::SharedPtr publisher_;
    std::unique_ptr<imu_ICM20948> imu_; // IMU object as a member variable
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<ImuPublisherNode>());
  rclcpp::shutdown();
  return 0;
}