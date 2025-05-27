#include "imu_ICM20948.h"
#include "i2c/i2c.h"
#include <iostream>
#include <iomanip>

imu_ICM20948::imu_ICM20948(unsigned char addr, const char *bus_name) {
    device.bus = i2c_open(bus_name);
    device.addr = addr;
    i2c_init_device(&device);
    device.flags = 0;

    std::cout << "imu_ICM20948 constructed with address 0x" << std::hex << (int)addr
              << " on bus " << bus_name << std::endl;
}

imu_ICM20948::~imu_ICM20948() {
    if (device.bus != -1) {
        i2c_close(device.bus);
    }
    std::cout << "imu_ICM20948 destructor called." << std::endl;
}

int imu_ICM20948::bank_select(unsigned char bank){
    i2c_ioctl_write(&device, REG_BANK_SEL, &bank, sizeof(bank)); // Write to the bank select register
    return 1;
}

int imu_ICM20948::identify(){
    unsigned char buffer[1];
    int ret = i2c_ioctl_read(&device, 0x00, buffer, sizeof(buffer)); // Read WHO_AM_I register

    if (ret < 0) {
        std::cerr << "Failed to read from IMU." << std::endl;
        return -1;
    }

    if (buffer[0] == 0xEA) { // Check if the device ID matches ICM20948
        std::cout << "IMU ICM20948 identified successfully." << std::endl;
        return 0; // Success
    } else {
        std::cerr << "IMU identification failed." <<std::endl;
        return -1; // Failure
    }
}

int imu_ICM20948::init_imu_dmp() {
    
    // TBC
    
    std::cout << "IMU initialized." << std::endl;
    // Prepare the IMU to read quaternion data 
    return 0;
}

int imu_ICM20948::init_imu_i2c(){
    std::cout << "Initializing IMU I2C..." << std::endl;

    bank_select(BANK_0); 

    // TBC

}

imu_readings imu_ICM20948::get_imu_readings() {
    imu_readings readings;
    unsigned char buffer[12]; 
    int bytes;
    bytes = i2c_ioctl_read(&device, 0x2D, buffer, sizeof(buffer)); 

    std::cout << "Read " << bytes << " bytes from IMU." << std::endl;

    readings.accel_x = (int16_t)((buffer[0] << 8) | buffer[1]);
    readings.accel_y = (int16_t)((buffer[2] << 8) | buffer[3]);
    readings.accel_z = (int16_t)((buffer[4] << 8) | buffer[5]);
    readings.gyro_x = (int16_t)((buffer[6] << 8) | buffer[7]);
    readings.gyro_y = (int16_t)((buffer[8] << 8) | buffer[9]);
    readings.gyro_z = (int16_t)((buffer[10] << 8) | buffer[11]);
                       
    return readings;
}

int imu_ICM20948::test_func() {

    int8_t ret;

    std::cout << "Checking accel X registers:" << std::endl;
    i2c_ioctl_read(&device, 0x2D, &ret, sizeof(ret)); 
    std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(ret) << std::endl;
    i2c_ioctl_read(&device, 0x2E, &ret, sizeof(ret)); 
    std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(ret) << std::endl;

    std::cout << "Checking user control registers" << std::endl;
    i2c_ioctl_read(&device, 0x03, &ret, sizeof(ret)); 
    std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(ret) << std::endl;

    std::cout << "End of test func" << std::endl;


    return 0; // Placeholder for actual test functionality
}

