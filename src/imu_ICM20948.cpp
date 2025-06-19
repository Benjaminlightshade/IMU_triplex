#include "imu_ICM20948.h"
#include "i2c/i2c.h"
#include <iostream>
#include <iomanip>
#include <unistd.h>

imu_ICM20948::imu_ICM20948(unsigned char addr, const char *bus_name) {
    device.bus = i2c_open(bus_name);
    device.addr = addr;
    i2c_init_device(&device);
    device.flags = 0;

    device_mag.bus = device.bus;
    device_mag.addr = 0x0C;
    i2c_init_device(&device_mag);
    device_mag.flags = 0;

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

    // Initialize the IMU I2C communication and the neccessary registers
    // IMU registers
    unsigned char user_ctrl =   0b00000000; // Default value for USER_CTRL register
    unsigned char pwr_mgmt_1 =  0b00000001; // Default value for PWR_MGMT_1 register
    unsigned char pwr_mgmt_2 =  0b00000000; // Default value for PWR_MGMT_2 register
    unsigned char int_pin_cfg = 0b00000010; // Default value for INT_PIN_CFG register
    
    // Mag registers
    unsigned char mag_reset = 0b00000001; // Bit to reset the magnetometer
    unsigned char whoami = 0;   // Who am I register used for identification and debugging
    unsigned char mag_ctrl2 = 0b00001000; // CNTRL2, mode 4, 100Hz refresh rate. 
    unsigned char mag_self_test = 0; // Self test mode for the magnetometer, not used in this case


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

    // Enable the accel and gyro sensors
    // The write will return an error because the first 2 bits are reserved. 
    // The rest of the bits will still be written correctly 
    i2c_ioctl_write(&device, REG_PWR_MGMT_2, &pwr_mgmt_2, 1);

    // Setup the magnetometer

    // Enable bypass mode to the magnetometer on the imu.
    if(i2c_ioctl_write(&device, REG_INT_PIN_CFG, &int_pin_cfg, 1) < 0) {
        std::cerr << "Failed to write to I2C_MST_CTRL register." << std::endl;
        return -1; // Error in writing to the register
    }

    // Short delay to allow the serial by pass to take effect
    usleep(100); // Sleep for 100 microseconds

    //////////////////////////////////////////////////
    /* Communicate with the magnetometer registers */
    /////////////////////////////////////////////////

    // Start the setup with a soft reset of the magnetometer
    if(i2c_ioctl_write(&device_mag, REG_MAG_CNTRL_3, &mag_reset, 1) < 0) {
        std::cerr << "Failed to write to Magnetometer CNTRL3 register." << std::endl;
        return -1; // Error in writing to the register
    }


    // For debugging purposes. 
    // Set the magnetometer in self test mode instead
    // Self test magnetometer, according to the datasheet.
    // Power down mode
    mag_self_test = 0; 
    i2c_ioctl_write(&device_mag, REG_MAG_CNTRL_2, &mag_self_test, 1); 

    // Self test mode
    mag_self_test = 0b00000001; 
    i2c_ioctl_write(&device_mag, REG_MAG_CNTRL_2, &mag_self_test, 1); 

    // Check if data is ready
    unsigned char status1 = 0;
    unsigned char bytes_read[6];

    while( (status1 & 0b00000001) != 0x01){
        // Wait until the magnetometer data is ready
        std::cout << "Waiting for magnetometer data to be ready..." << std::endl;
        usleep(10);
        i2c_ioctl_read(&device_mag, REG_MAG_STATUS_1, &status1, 1); 
    }

    // Display the magnetometer data. To check if the values are in the expected working range. 
    std::cout << "Magnetometer data is ready." << std::endl;
    i2c_ioctl_read(&device_mag, REG_MAG_HXL, bytes_read, 6); // Read 6 bytes of magnetometer data
    std::cout << "Magnetometer self test data: ";
    for(int i = 0; i < 6; i++) {
        std::cout << std::hex << static_cast<int>(bytes_read[i]) << " ";
    }
    std::cout << std::dec << std::endl;
    
    // Start the setup with a soft reset of the magnetometer
    if(i2c_ioctl_write(&device_mag, REG_MAG_CNTRL_3, &mag_reset, 1) < 0) {
        std::cerr << "Failed to write to Magnetometer CNTRL3 register." << std::endl;
        return -1; // Error in writing to the register
    }

    usleep(100); // Sleep for 100 microseconds

    // Check the device ID of the magnetometer. Used for identification and debugging.
    if(i2c_ioctl_read(&device_mag, REG_MAG_DEVICE_ID, &whoami, 1) < 0){
        std::cerr << "Failed to read from Magnetometer WHO_AM_I register." << std::endl;
        return -1; // Error in reading from the register
    }
    std::cout << "Magnetometer WHO_AM_I: " << static_cast<int>(whoami) << std::endl;

    // Set up for the magnetometer, using cntrl2 register
    if(i2c_ioctl_write(&device_mag, REG_MAG_CNTRL_2, &mag_ctrl2, 1) < 0) {
        std::cerr << "Failed to write to Magnetometer CNTRL2 register." << std::endl;
        return -1; // Error in writing to the register
    }

    return 0;

}


imu_readings imu_ICM20948::get_imu_readings() {
    imu_readings readings;
    unsigned char buffer[12]; 
    unsigned char mag_buffer[9];
    int16_t raw_accelx, raw_accely, raw_accelz;
    int16_t raw_gyrox, raw_gyroy, raw_gyroz;
    int16_t raw_magx, raw_magy, raw_magz;

    i2c_ioctl_read(&device, 0x2D, buffer, sizeof(buffer)); 


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


    // Read status1 and the 6 bytes of magnetometer data from the magnetometer registers
    // Reads up to the status2 register, which is required. 
    if(i2c_ioctl_read(&device_mag, REG_MAG_STATUS_1, mag_buffer, sizeof(mag_buffer)) < 0) {
        std::cerr << "Failed to read from Magnetometer." << std::endl;
        return readings; // Return empty readings on error
    }

    // Check if the magnetometer has a measurement ready by checking the DRDY register.
    // Retry up to 3 times if the data is not ready.
    int retry_counter = 0;
    while((mag_buffer[0] & 0b00000001) != 0x01){
        // Wait until the magnetometer data is ready
        retry_counter++;
        if (retry_counter > 3) {
            std::cerr << "Magnetometer read data not available." << std::endl;
            return readings; // Return empty readings if data is not ready
        }
        usleep(10); // Sleep for 10 microseconds
        i2c_ioctl_read(&device_mag, REG_MAG_STATUS_1, mag_buffer, sizeof(mag_buffer));
    }

    raw_magx = (mag_buffer[2] << 8) | mag_buffer[1];
    raw_magy = (mag_buffer[4] << 8) | mag_buffer[3];
    raw_magz = (mag_buffer[6] << 8) | mag_buffer[5];

    // Magnetometer typical resolution 0.15uT/LSB
    readings.mag_x = ((float)raw_magx) * 0.15 / 1000000; // Default magnetometer sensitivity 16 LSB/uT
    readings.mag_y = ((float)raw_magy) * 0.15 / 1000000; // Default magnetometer sensitivity 16 LSB/uT
    readings.mag_z = ((float)raw_magz) * 0.15 / 1000000; // Default magnetometer sensitivity 16 LSB/uT

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


