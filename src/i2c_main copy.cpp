#include <stdio.h>
#include <cstdlib>
#include <iostream>

#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <unistd.h>

using namespace std;

int main(int argc, char const *argv[])
{

    char device[] = "/dev/i2c-1";
    int slave_addr = 0x68; // I2C address of ICM20948V2
    int adapter_nr = 1; /* probably dynamically determined */

    int file;
    char filename[20];
    
    /* Open the i2c device file */
    snprintf(filename, 19, "/dev/i2c-%d", adapter_nr);
    file = open(filename, O_RDWR);
    if (file < 0) {
      printf("Failed to open the i2c bus\n");
      exit(0);
    }    

    cout << "I2C device opened successfully" << endl; 

    /* Set the I2C slave address */
    if (ioctl(file, I2C_SLAVE, slave_addr) < 0) {
      printf("Failed to acquire bus access and/or talk to slave.\n");
      exit(1);
    }

    // Checks for the functionality of the I2C adapter

    unsigned long funcs; 
    if (ioctl(file, I2C_FUNCS, &funcs) < 0) {
      printf("Failed to get the adapter functionality.\n");
      exit(1);
    } 

    printf("I2C adapter functionality: 0x%lx\n", funcs);

    /* Now you can read from or write to the device */
    
    // Command for reading a byte

    struct i2c_msg msgs[2];
    struct i2c_rdwr_ioctl_data rdwr_data;

    __u8 reg_addr = 0x00;
    __u8 buf;

    // First message: Write the register address
    msgs[0].addr = slave_addr;
    msgs[0].flags = 0; // Write
    msgs[0].len = 1;
    msgs[0].buf = &reg_addr;

    // Second message: Read the data
    msgs[1].addr = slave_addr;
    msgs[1].flags = I2C_M_RD; // Read
    msgs[1].len = 1;
    msgs[1].buf = &buf;

    rdwr_data.msgs = msgs;
    rdwr_data.nmsgs = 2;

    while (1) {
        if (ioctl(file, I2C_RDWR, &rdwr_data) < 0) {
            printf("Failed to read from the device.\n");
            exit(1);
        }
        printf("Read byte: 0x%02x\n", buf);
        sleep(1);
    }

    return 0;

}







