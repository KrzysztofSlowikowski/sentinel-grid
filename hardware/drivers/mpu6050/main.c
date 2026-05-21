#include <stdio.h>
#include "mpu6050.h"

int main(void) {
    printf("=== Sentinel Grid — test czujnika MPU6050 ===\n\n");
    
    MPU6050_Device czujnik = mpu6050_init();
    
    // --- SCENARIUSZ 1: normalny odczyt (spoczynek) ---
    printf("--- Scenariusz 1: normalny odczyt (spoczynek) ---\n");
    
    MPU6050_Status status = mpu6050_check_who_am_i(&czujnik);
    if (status != MPU6050_OK) {
        printf("Błąd inicjalizacji: %d\n", status);
        return 1;
    }
    
    status = mpu6050_read_accel(&czujnik);
    if (status == MPU6050_OK) {
        printf("Przyspieszenie: X=%d, Y=%d, Z=%d\n",
               mpu6050_get_accel_x(&czujnik),
               mpu6050_get_accel_y(&czujnik),
               mpu6050_get_accel_z(&czujnik));
    }
    
    mpu6050_print_status(&czujnik);
    
    // --- SCENARIUSZ 2: symulacja ruchu (przyspieszenie w lewo) ---
    printf("\n--- Scenariusz 2: ruch w lewo (X = -10000) ---\n");
    mpu6050_simulate_accel(&czujnik, -10000, 0, 16384);
    
    status = mpu6050_read_accel(&czujnik);
    if (status == MPU6050_OK) {
        printf("Przyspieszenie po ruchu: X=%d, Y=%d, Z=%d\n",
               mpu6050_get_accel_x(&czujnik),
               mpu6050_get_accel_y(&czujnik),
               mpu6050_get_accel_z(&czujnik));
    }
    
    mpu6050_print_status(&czujnik);
    
    // --- SCENARIUSZ 3: odłączenie czujnika ---
    printf("\n--- Scenariusz 3: czujnik odłączony ---\n");
    czujnik.connected = 0;
    
    status = mpu6050_read_accel(&czujnik);
    if (status != MPU6050_OK) {
        printf("Oczekiwany błąd: %d\n", status);
    }
    
    mpu6050_print_status(&czujnik);
    
    // --- SCENARIUSZ 4: powrót czujnika ---
    printf("\n--- Scenariusz 4: powrót połączenia ---\n");
    czujnik.connected = 1;
    
    status = mpu6050_read_accel(&czujnik);
    if (status == MPU6050_OK) {
        printf("Przyspieszenie po powrocie: X=%d, Y=%d, Z=%d\n",
               mpu6050_get_accel_x(&czujnik),
               mpu6050_get_accel_y(&czujnik),
               mpu6050_get_accel_z(&czujnik));
    }
    
    mpu6050_print_status(&czujnik);
    
    return 0;
}