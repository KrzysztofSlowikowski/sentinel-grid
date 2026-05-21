#include "mpu6050.h"
#include <stdio.h>
#include <string.h>

MPU6050_Device mpu6050_init(void) {
    MPU6050_Device device;
    
    // Zerujemy całą strukturę
    memset(&device, 0, sizeof(device));
    
    // Ustawiamy symulowane rejestry
    device.registers[MPU6050_REG_WHO_AM_I] = MPU6050_WHO_AM_I_VALUE;
    
    // Domyślne wartości akcelerometru (spoczynek: X=0, Y=0, Z=+16384 = 1g)
    // 16384 to czułość dla zakresu ±2g (dokumentacja MPU6050)
    device.registers[MPU6050_ACCEL_XOUT_H] = 0x00;  // X = 0
    device.registers[MPU6050_ACCEL_XOUT_L] = 0x00;
    device.registers[MPU6050_ACCEL_YOUT_H] = 0x00;  // Y = 0
    device.registers[MPU6050_ACCEL_YOUT_L] = 0x00;
    device.registers[MPU6050_ACCEL_ZOUT_H] = 0x40;  // Z = 16384 (0x4000)
    device.registers[MPU6050_ACCEL_ZOUT_L] = 0x00;
    
    device.initialized = 1;
    device.connected   = 1;
    device.last_error  = MPU6050_OK;
    
    return device;
}

MPU6050_Status mpu6050_check_who_am_i(MPU6050_Device *device) {
    if (device->initialized == 0) {
        device->last_error = MPU6050_ERR_NOT_INIT;
        return MPU6050_ERR_NOT_INIT;
    }
    
    if (device->connected == 0) {
        device->last_error = MPU6050_ERR_NO_RESPONSE;
        printf("Błąd: czujnik nie odpowiada\n");
        return MPU6050_ERR_NO_RESPONSE;
    }
    
    uint8_t who_am_i = device->registers[MPU6050_REG_WHO_AM_I];
    
    if (who_am_i == MPU6050_WHO_AM_I_VALUE) {
        printf("WHO_AM_I: 0x%02X — poprawny\n", who_am_i);
        device->last_error = MPU6050_OK;
        return MPU6050_OK;
    } else {
        device->last_error = MPU6050_ERR_WRONG_WHO_AM_I;
        printf("WHO_AM_I: 0x%02X — błędny, oczekiwano 0x%02X\n",
               who_am_i, MPU6050_WHO_AM_I_VALUE);
        return MPU6050_ERR_WRONG_WHO_AM_I;
    }
}
void mpu6050_simulate_accel(MPU6050_Device *device, int16_t x, int16_t y, int16_t z) {
    if (device->initialized == 0) return;
    
    // Dla wartości ze znakiem, musimy zapisać je jako dwa bajty (big-endian!)
    // Większy bajt (high) idzie na niższy adres
    
    // X
    device->registers[MPU6050_ACCEL_XOUT_H] = (x >> 8) & 0xFF;   // starszy bajt
    device->registers[MPU6050_ACCEL_XOUT_L] = x & 0xFF;          // młodszy bajt
    
    // Y
    device->registers[MPU6050_ACCEL_YOUT_H] = (y >> 8) & 0xFF;
    device->registers[MPU6050_ACCEL_YOUT_L] = y & 0xFF;
    
    // Z
    device->registers[MPU6050_ACCEL_ZOUT_H] = (z >> 8) & 0xFF;
    device->registers[MPU6050_ACCEL_ZOUT_L] = z & 0xFF;
    
    printf("--- Symulacja: akcelerometr (X=%d, Y=%d, Z=%d) ---\n", x, y, z);
}

MPU6050_Status mpu6050_read_accel(MPU6050_Device *device) {
    if (device->initialized == 0) {
        device->last_error = MPU6050_ERR_NOT_INIT;
        return MPU6050_ERR_NOT_INIT;
    }
    
    if (device->connected == 0) {
        device->last_error = MPU6050_ERR_NO_RESPONSE;
        printf("Błąd: czujnik nie odpowiada\n");
        return MPU6050_ERR_NO_RESPONSE;
    }
    
    // Składamy 16-bitowe wartości z dwóch bajtów (big-endian!)
    int16_t raw_x = ((int16_t)device->registers[MPU6050_ACCEL_XOUT_H] << 8) |
                     device->registers[MPU6050_ACCEL_XOUT_L];
    
    int16_t raw_y = ((int16_t)device->registers[MPU6050_ACCEL_YOUT_H] << 8) |
                     device->registers[MPU6050_ACCEL_YOUT_L];
    
    int16_t raw_z = ((int16_t)device->registers[MPU6050_ACCEL_ZOUT_H] << 8) |
                     device->registers[MPU6050_ACCEL_ZOUT_L];
    
    // Prosta walidacja: dla nieruchomego czujnika, przyspieszenie Z powinno być ~16384
    // Ale nie walidujemy zbyt restrykcyjnie w symulatorze
    
    device->accel_x = raw_x;
    device->accel_y = raw_y;
    device->accel_z = raw_z;
    device->last_error = MPU6050_OK;
    
    return MPU6050_OK;
}

void mpu6050_print_status(MPU6050_Device *device) {
    printf("=== MPU6050 Status ===\n");
    printf("Połączony:    %s\n", device->connected   ? "TAK" : "NIE");
    printf("Gotowy:       %s\n", device->initialized ? "TAK" : "NIE");
    printf("Akcelerometr: X=%d, Y=%d, Z=%d\n", 
           device->accel_x, device->accel_y, device->accel_z);
    printf("Ostatni błąd: %d\n", device->last_error);
    printf("=====================\n");
}

// Funkcja pomocnicza do pobrania wartości (jak w PMS5003)
int16_t mpu6050_get_accel_x(MPU6050_Device *device) { return device->accel_x; }
int16_t mpu6050_get_accel_y(MPU6050_Device *device) { return device->accel_y; }
int16_t mpu6050_get_accel_z(MPU6050_Device *device) { return device->accel_z; }