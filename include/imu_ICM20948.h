#ifndef imu_ICM20948_H
#define imu_ICM20948_H

#include "i2c/i2c.h"
#include <iostream>

// IMU register common address
#define REG_BANK_SEL    0x7F

// IMU reigister addresses - bank 1

#define REG_WHO_AM_I        0x00
#define REG_USER_CTRL       0x03
#define REG_PWR_MGMT_1      0x06
#define REG_PWR_MGMT_2      0x07
#define REG_INT_PIN_CFG     0x0F
#define REG_ACCEL_XOUT_H    0x2D
#define REG_ACCEL_XOUT_L    0x2E
#define REG_ACCEL_YOUT_H    0x2F
#define REG_ACCEL_YOUT_L    0x30
#define REG_ACCEL_ZOUT_H    0x31
#define REG_ACCEL_ZOUT_L    0x32
#define REG_GYRO_XOUT_H     0x33
#define REG_GYRO_XOUT_L     0x34
#define REG_GYRO_YOUT_H     0x35
#define REG_GYRO_YOUT_L     0x36
#define REG_GYRO_ZOUT_H     0x37
#define REG_GYRO_ZOUT_L     0x38
#define REG_TEMP_OUT_H      0x39
#define REG_TEMP_OUT_L      0x3A

#define REG_FIFO_EN_1       0x66
#define REG_FIFO_EN_2       0x67

// IMU reigister addresses - bank 2

#define REG_GYRO_SMPLRT_DIV 0x00
#define REG_GYRO_CONFIG_1   0x01
#define REG_GYRO_CONFIG_2   0x02

#define REG_ACCEL_SMPLRT_DIV_1 0x10
#define REG_ACCEL_SMPLRT_DIV_2 0x11
#define REG_ACCEL_CONFIG_1     0x14
#define REG_ACCEL_CONFIG_2     0x15


// Useful register values

#define BANK_0 0x00
#define BANK_1 0x01
#define BANK_2 0x02
#define BANK_3 0x03

// Magnetometer register addresses

#define REG_MAG_COMPANY_ID  0x00
#define REG_MAG_DEVICE_ID   0x01
#define REG_MAG_STATUS_1    0x10
#define REG_MAG_HXL         0x11
#define REG_MAG_HXH         0x12
#define REG_MAG_HYL         0x13
#define REG_MAG_HYH         0x14
#define REG_MAG_HZL         0x15
#define REG_MAG_HZH         0x16
#define REG_MAG_STATUS_2    0x18
#define REG_MAG_CNTRL_1     0x30
#define REG_MAG_CNTRL_2     0x31
#define REG_MAG_CNTRL_3     0x32


struct  imu_readings{
    float accel_x;
    float accel_y;
    float accel_z;
    float gyro_x;
    float gyro_y;
    float gyro_z;
    float mag_x;
    float mag_y;
    float mag_z;
};

class imu_ICM20948{

    public: 
        imu_ICM20948(unsigned char addr, const char *bus_name);
        ~imu_ICM20948();
        int bank_select(unsigned char bank);
        int identify();
        int init_imu();
        int init_imu_dmp();
        int init_imu_i2c();
        imu_readings get_imu_readings();
        int calibrate_imu();

    private:
        I2CDevice device;
        I2CDevice device_mag;
        float calibration_offsets[6];
        
        
    
};




#endif

/*
Requirements for accessing the I2C bus:

Address of the device 


*/ 