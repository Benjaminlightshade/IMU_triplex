#ifndef imu_ICM20948_H
#define imu_ICM20948_H

#include "i2c/i2c.h"
#include <iostream>
#include <cstdint>

// IMU reigister addresses

#define REG_WHO_AM_I 0x00
#define REG_BANK_SEL 0x7F

#define BANK_0 0x00
#define BANK_1 0x01
#define BANK_2 0x02
#define BANK_3 0x03

struct  imu_readings{
    int16_t accel_x;
    int16_t accel_y;
    int16_t accel_z;
    int16_t gyro_x;
    int16_t gyro_y;
    int16_t gyro_z;
    int16_t mag_x;
    int16_t mag_y;
    int16_t mag_z;
};

class imu_ICM20948{

    public: 
        imu_ICM20948(unsigned char addr, const char *bus_name);
        ~imu_ICM20948();
        int bank_select(unsigned char bank);
        int identify();
        int init_imu_dmp();
        int init_imu_i2c();
        imu_readings get_imu_readings();

        int test_func();
    private:
        I2CDevice device;
    
};




#endif

/*
Requirements for accessing the I2C bus:

Address of the device 


*/ 