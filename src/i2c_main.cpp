#include <iostream>
#include <thread>

#include "rclcpp/rclcpp.hpp"

#include "test_lib.h"
#include "i2c/i2c.h"
#include "imu_ICM20948.h"


int main()
{
    std::cout << "Starting IMU ICM20948 Example..." << std::endl;

    imu_ICM20948 imu(0x68, "/dev/i2c-1");
    imu.identify();
    imu.test_func();

    while(true){

        imu_readings readings = imu.get_imu_readings();
        
        // Process the readings as needed
        std::cout << "Accel: (" << readings.accel_x << ", " << readings.accel_y << ", " << readings.accel_z << "), "
                  << "Gyro: (" << readings.gyro_x << ", " << readings.gyro_y << ", " << readings.gyro_z << ")" 
                  << std::endl;

        // Delay for a second
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));        
    }
 
    std::cout << "exiting" << std::endl;

    return 0;
}

/*
Work
1. Collect the i2c data
2. Visualize it in RVIZ
3. Do sensor fusion with multiple sensors 

*/