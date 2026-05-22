#include <stdio.h>
#include "bme680/bme680.h"
#include "pms5003/pms5003.h"
#include "mpu6050/mpu6050.h"

int main(void) {
    printf("=== Sentinel Grid — test wszystkich czujników ===\n\n");
    
    // --- INICJALIZACJA WSZYSTKICH CZUJNIKÓW ---
    printf("--- Inicjalizacja czujników ---\n");
    
    BME680_Device bme680 = bme680_init(BME680_I2C_ADDRESS);
    PMS5003_Device pms5003 = pms5003_init();
    MPU6050_Device mpu6050 = mpu6050_init();
    
    // Sprawdzenie BME680
    if (bme680_check_chip_id(&bme680) != BME680_OK) {
        printf("BŁĄD: BME680 nie odpowiada\n");
        return 1;
    }
    
    // Sprawdzenie MPU6050
    if (mpu6050_check_who_am_i(&mpu6050) != MPU6050_OK) {
        printf("BŁĄD: MPU6050 nie odpowiada\n");
        return 1;
    }
    
    printf("Wszystkie czujniki zainicjalizowane prawidłowo\n\n");
    
    // --- SYMULACJA DANYCH (bo nie mamy prawdziwego sprzętu) ---
    printf("--- Symulacja odczytów ---\n");
    
    // BME680: symulujemy temperaturę (można dodać później)
    bme680_read_temperature(&bme680);
    
    // PMS5003: symulujemy PM2.5 = 65 (przekracza 50!)
    pms5003_simulate_data(&pms5003, 65);
    
    // MPU6050: symulujemy drgania na osi Z (13000 = poza zakresem 14000-18000)
    mpu6050_simulate_accel(&mpu6050, 0, 0, 13000);
    
    // --- ODCZYT DANYCH ---
    printf("\n--- Odczyt danych z czujników ---\n");
    
    // BME680 - temperatura
    if (bme680_read_temperature(&bme680) == BME680_OK) {
        printf("BME680: Temperatura = %.2f°C\n", bme680.temperature);
    } else {
        printf("BME680: Błąd odczytu temperatury\n");
    }
    
    // PMS5003 - PM2.5
    if (pms5003_read_frame(&pms5003) == PMS5003_OK) {
        uint16_t pm25 = pms5003_get_pm25(&pms5003);
        printf("PMS5003: PM2.5 = %d µg/m³\n", pm25);
        
        // ALERT: PM2.5 > 50
        if (pm25 > 50) {
            printf("*** ALERT: Zanieczyszczenie powietrza! PM2.5 = %d µg/m³ (norma: 50) ***\n", pm25);
        }
    } else {
        printf("PMS5003: Błąd odczytu PM2.5\n");
    }
    
    // MPU6050 - akcelerometr
    if (mpu6050_read_accel(&mpu6050) == MPU6050_OK) {
        int16_t accel_z = mpu6050_get_accel_z(&mpu6050);
        printf("MPU6050: Drgania Z = %d (spoczynek: 16384)\n", accel_z);
        
        // ALERT: drgania poza zakresem 14000-18000
        if (accel_z < 14000 || accel_z > 18000) {
            printf("*** ALERT: Wykryto drgania! Przyspieszenie Z = %d (norma: 14000-18000) ***\n", accel_z);
        }
    } else {
        printf("MPU6050: Błąd odczytu akcelerometru\n");
    }
    
    // --- PODSUMOWANIE ---
    printf("\n--- Status końcowy czujników ---\n");
    bme680_print_status(&bme680);
    pms5003_print_status(&pms5003);
    mpu6050_print_status(&mpu6050);
    
    return 0;
}