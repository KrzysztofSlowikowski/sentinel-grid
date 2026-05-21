#ifndef MPU6050_H
#define MPU6050_H

#include <stdint.h>

typedef enum {
    MPU6050_OK                =  0,
    MPU6050_ERR_NOT_INIT      = -1,
    MPU6050_ERR_NO_RESPONSE   = -2,
    MPU6050_ERR_WRONG_WHO_AM_I = -3,
    MPU6050_ERR_BAD_DATA      = -4
} MPU6050_Status;

// Adres I2C
#define MPU6050_I2C_ADDRESS    0x68

// Rejestry
#define MPU6050_REG_WHO_AM_I   0x75
#define MPU6050_REG_PWR_MGMT_1 0x6B

// Akcelerometr (big-endian: HIGH pierwszy, LOW drugi)
#define MPU6050_ACCEL_XOUT_H   0x3B
#define MPU6050_ACCEL_XOUT_L   0x3C
#define MPU6050_ACCEL_YOUT_H   0x3D
#define MPU6050_ACCEL_YOUT_L   0x3E
#define MPU6050_ACCEL_ZOUT_H   0x3F
#define MPU6050_ACCEL_ZOUT_L   0x40

// Oczekiwana wartość WHO_AM_I
#define MPU6050_WHO_AM_I_VALUE 0x68

typedef struct {
    uint8_t initialized;
    uint8_t connected;
    MPU6050_Status last_error;
    
    // Symulowane rejestry (jak w BME680)
    uint8_t registers[256];
    
    // Odczytane wartości (ze znakiem!)
    int16_t accel_x;
    int16_t accel_y;
    int16_t accel_z;
} MPU6050_Device;

MPU6050_Device mpu6050_init(void);
MPU6050_Status mpu6050_check_who_am_i(MPU6050_Device *device);
MPU6050_Status mpu6050_read_accel(MPU6050_Device *device);
void           mpu6050_print_status(MPU6050_Device *device);
void           mpu6050_simulate_accel(MPU6050_Device *device, int16_t x, int16_t y, int16_t z);
int16_t mpu6050_get_accel_x(MPU6050_Device *device);
int16_t mpu6050_get_accel_y(MPU6050_Device *device);
int16_t mpu6050_get_accel_z(MPU6050_Device *device);

#endif