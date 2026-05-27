#include <stdio.h>
#include <unistd.h>

#include "mqtt_client/mqtt_client.h"
#include "bme680/bme680.h"
#include "pms5003/pms5003.h"
#include "mpu6050/mpu6050.h"

// Konfiguracja
#define MQTT_BROKER     "tcp://localhost:1883"
#define MQTT_CLIENT_ID  "sentinel_gateway"

#define TOPIC_TEMP   "sentinel/node/02/temperature"
#define TOPIC_PM25   "sentinel/node/02/pm25"
#define TOPIC_ACCEL  "sentinel/node/02/accel"
#define TOPIC_ALERT  "sentinel/alerts"

int main(void) {
    printf("=== Sentinel Grid — Gateway z MQTT (modularny) ===\n\n");
    
    // --- MQTT ---
    if (mqtt_init(MQTT_BROKER, MQTT_CLIENT_ID) != 0) {
        return 1;
    }
    if (mqtt_connect() != 0) {
        return 1;
    }
    
    // --- CZUJNIKI ---
    printf("\n--- Inicjalizacja czujników ---\n");
    
    BME680_Device bme680 = bme680_init(BME680_I2C_ADDRESS);
    PMS5003_Device pms5003 = pms5003_init();
    MPU6050_Device mpu6050 = mpu6050_init();
    
    if (bme680_check_chip_id(&bme680) != BME680_OK ||
        mpu6050_check_who_am_i(&mpu6050) != MPU6050_OK) {
        printf("BŁĄD: inicjalizacja czujników\n");
        mqtt_cleanup();
        return 1;
    }
    
    printf("Wszystkie czujniki gotowe\n\n");
    
    // --- GŁÓWNA PĘTLA (5 odczytów) ---
    printf("--- Rozpoczęcie odczytów ---\n");
    
    uint16_t test_pm25[] = {35, 65, 42, 88, 23};
    
    for (int i = 0; i < 5; i++) {
        printf("\n--- Odczyt %d ---\n", i + 1);
        
        // BME680
        if (bme680_read_temperature(&bme680) == BME680_OK) {
            char buf[16];
            snprintf(buf, sizeof(buf), "%.2f", bme680.temperature);
            mqtt_publish(TOPIC_TEMP, buf, 1);
            
            if (bme680.temperature > 35.0f) {
                snprintf(buf, sizeof(buf), "{\"temp\":%.2f}", bme680.temperature);
                mqtt_publish(TOPIC_ALERT, buf, 2);
            }
        }
        
        // PMS5003
        pms5003_simulate_data(&pms5003, test_pm25[i]);
        if (pms5003_read_frame(&pms5003) == PMS5003_OK) {
            char buf[16];
            snprintf(buf, sizeof(buf), "%d", pms5003_get_pm25(&pms5003));
            mqtt_publish(TOPIC_PM25, buf, 1);
            
            if (pms5003_get_pm25(&pms5003) > 50) {
                snprintf(buf, sizeof(buf), "{\"pm25\":%d}", pms5003_get_pm25(&pms5003));
                mqtt_publish(TOPIC_ALERT, buf, 2);
            }
        }
        
        // MPU6050
        if (mpu6050_read_accel(&mpu6050) == MPU6050_OK) {
            char buf[64];
            snprintf(buf, sizeof(buf), "X=%d,Y=%d,Z=%d",
                     mpu6050_get_accel_x(&mpu6050),
                     mpu6050_get_accel_y(&mpu6050),
                     mpu6050_get_accel_z(&mpu6050));
            mqtt_publish(TOPIC_ACCEL, buf, 1);
            
            int16_t z = mpu6050_get_accel_z(&mpu6050);
            if (z < 14000 || z > 18000) {
                snprintf(buf, sizeof(buf), "{\"vibration_z\":%d}", z);
                mqtt_publish(TOPIC_ALERT, buf, 2);
            }
        }
        
        sleep(2);
    }
    
    // --- SPRZĄTANIE ---
    printf("\n--- Koniec ---\n");
    mqtt_cleanup();
    
    return 0;
}