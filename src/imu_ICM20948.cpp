#include "imu_ICM20948.h"
#include "i2c/i2c.h"
#include <iostream>
#include <iomanip>

imu_ICM20948::imu_ICM20948(unsigned char addr, const char *bus_name) {
    device.bus = i2c_open(bus_name);
    device.addr = addr;
    i2c_init_device(&device);
    device.flags = 0;

    // Gyro calibration offsets
    for (int i = 0; i < 6; ++i) {
        calibration_offsets[i] = 0.0f; // Initialize gyro calibration offsets to zero
    }

    std::cout << std::hex;
    std::cout << "imu_ICM20948 constructed with address 0x" << (int)addr
              << " on bus " << bus_name << std::endl;
    std::cout << std::dec;
}

imu_ICM20948::~imu_ICM20948() {
    if (device.bus != -1) {
        i2c_close(device.bus);
    }
    std::cout << "imu_ICM20948 destructor called." << std::endl;
}

int imu_ICM20948::bank_select(unsigned char bank){
    int ret;
    
    ret = i2c_ioctl_write(&device, REG_BANK_SEL, &bank, sizeof(bank)); // Write to the bank select register
    if (ret != sizeof(bank)) {
        std::cerr << "Failed to select bank " << (int)bank << " for IMU." << std::endl;
        return -1; // Error in writing to the register
    }
    
    // Success
    return 0;
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


int imu_ICM20948::init_imu(){

    init_imu_i2c(); // Initialize the IMU I2C communication
    calibrate_imu();

    return 0;
}


int imu_ICM20948::init_imu_dmp() {
    
    // TBC
    
    std::cout << "IMU initialized." << std::endl;
    // Prepare the IMU to read quaternion data 
    return 0;
}


int imu_ICM20948::init_imu_i2c(){
    
    unsigned char user_ctrl = 0b00000000; // Default value for USER_CTRL register
    unsigned char pwr_mgmt_1 = 0b00000001; // Default value for PWR_MGMT_1 register
    unsigned char pwr_mgmt_2 = 0b00000000; // Default value for PWR_MGMT_2 register

    std::cout << "Initializing IMU I2C..." << std::endl;

    bank_select(BANK_0); 

    // Setup the IMU registers to enable I2C communication
    if(i2c_ioctl_write(&device, REG_USER_CTRL, &user_ctrl, 1) < 0) {
        std::cerr << "Failed to write to USER_CTRL register." << std::endl;
        return -1; // Error in writing to the register
    }
    if(i2c_ioctl_write(&device, REG_PWR_MGMT_1, &pwr_mgmt_1, 1) < 0) {
        std::cerr << "Failed to write to PWR_MGMT_1 register." << std::endl;
        return -1; // Error in writing to the register
    }

    // Read current PWR_MGMT_2 register value
    if(i2c_ioctl_read(&device, REG_PWR_MGMT_2, &pwr_mgmt_2, 1) < 0 ){
        std::cerr << "Failed to read from PWR_MGMT_2 register." << std::endl;
        return -1; // Error in writing to the register
    } 
    
    std::cout << "Current PWR_MGMT_2 value: " << static_cast<int>(pwr_mgmt_2) << std::endl;

    // Todo : Find a way to set the bits without overwring the reserved bits
    if(i2c_ioctl_write(&device, REG_PWR_MGMT_2, &pwr_mgmt_2, 1)){
        std::cerr << "Failed to write to PWR_MGMT_2 register." << std::endl;
        return -1; // Error in writing to the register
    }

    return 0;

}


imu_readings imu_ICM20948::get_imu_readings() {
    imu_readings readings;
    unsigned char buffer[12]; 
    int bytes;
    bytes = i2c_ioctl_read(&device, 0x2D, buffer, sizeof(buffer)); 

    // std::cout << "Read " << bytes << " bytes from IMU." << std::endl;

    int16_t raw_accelx, raw_accely, raw_accelz;
    int16_t raw_gyrox, raw_gyroy, raw_gyroz;

    raw_accelx = (buffer[0] << 8) | buffer[1];
    raw_accely = (buffer[2] << 8) | buffer[3];
    raw_accelz = (buffer[4] << 8) | buffer[5];
    raw_gyrox = (buffer[6] << 8) | buffer[7];
    raw_gyroy = (buffer[8] << 8) | buffer[9];
    raw_gyroz = (buffer[10] << 8) | buffer[11];

    // Default acceleromter sensitivity +/- 2g = 16384 LSB/g
    readings.accel_x = ((float)raw_accelx / 16384 * 9.81) - calibration_offsets[0];
    readings.accel_y = ((float)raw_accely / 16384 * 9.81) - calibration_offsets[1];
    readings.accel_z = ((float)raw_accelz / 16384 * 9.81) - calibration_offsets[2];

    // Default gyro sensitivity +/- 250dps = 131 LSB/deg/s, 57.27 deg/s = 1 rad/s
    readings.gyro_x = ((float)raw_gyrox / 131 / 57.273) - calibration_offsets[3];
    readings.gyro_y = ((float)raw_gyroy / 131 / 57.273) - calibration_offsets[4];
    readings.gyro_z = ((float)raw_gyroz / 131 / 57.273) - calibration_offsets[5];
             
    return readings;
}


int imu_ICM20948::calibrate_imu() {
    // Placeholder for calibration logic
    std::cout << "Calibrating IMU..." << std::endl;
    

    // Check if IMU is initialized
    if (device.bus == -1) {
        std::cerr << "IMU not initialized. Please initialize the IMU first." << std::endl;
        return -1; // Error: IMU not initialized
    }

    // Read 100 samples from the IMU and calculate the average values
    imu_readings avg_readings = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    for (int i = 0; i < 100; ++i) {
        imu_readings readings = get_imu_readings();
        avg_readings.accel_x += readings.accel_x;
        avg_readings.accel_y += readings.accel_y;
        avg_readings.accel_z += readings.accel_z;
        avg_readings.gyro_x += readings.gyro_x;
        avg_readings.gyro_y += readings.gyro_y;
        avg_readings.gyro_z += readings.gyro_z;
    }

    // Calculate the average readings from the 100 samples
    avg_readings.accel_x /= 100;
    avg_readings.accel_y /= 100;
    avg_readings.accel_z /= 100;
    avg_readings.gyro_x /= 100;
    avg_readings.gyro_y /= 100;
    avg_readings.gyro_z /= 100;

    // Store the difference from the expected values
    calibration_offsets[0] = avg_readings.accel_x; 
    calibration_offsets[1] = avg_readings.accel_y;
    calibration_offsets[2] = avg_readings.accel_z - 9.81; // Adjust for gravity
    calibration_offsets[3] = avg_readings.gyro_x;
    calibration_offsets[4] = avg_readings.gyro_y;
    calibration_offsets[5] = avg_readings.gyro_z;

    return 0; // Return 0 to indicate success
}


int imu_ICM20948::test_func() {
    int ret;

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

