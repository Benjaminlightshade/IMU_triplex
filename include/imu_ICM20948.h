#ifndef imu_ICM20948_H
#define imu_ICM20948_H

#include "i2c/i2c.h"
#include <iostream>
#include <cstdint>
#

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
        int identify();
        int init_imu_dmp();
        imu_readings get_imu_readings();


    private:
        I2CDevice device;
    
};




#endif

/*
Requirements for accessing the I2C bus:

Address of the device 


*/ 