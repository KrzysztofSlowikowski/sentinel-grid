#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <MQTTClient.h>

#include "bme680/bme680.h"
#include "pms5003/pms5003.h"
#include "mpu6050/mpu6050.h"

// Konfiguracja MQTT
#define MQTT_BROKER     "tcp://localhost:1883"
#define MQTT_CLIENT_ID  "sentinel_gateway"
#define MQTT_QOS        1
#define MQTT_TIMEOUT    10000L

// Topiki (zgodne z sentinel-grid)
#define TOPIC_BME680_TEMP   "sentinel/node/01/temperature"
#define TOPIC_PMS5003_PM25  "sentinel/node/01/pm25"
#define TOPIC_MPU6050_ACCEL "sentinel/node/01/accel"
#define TOPIC_ALERT         "sentinel/alerts"

// Funkcja pomocnicza do wysyłania MQTT
void mqtt_send(MQTTClient client, const char* topic, const char* payload) {
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;
    
    pubmsg.payload = (char*)payload;
    pubmsg.payloadlen = strlen(payload);
    pubmsg.qos = MQTT_QOS;
    pubmsg.retained = 0;
    
    int rc = MQTTClient_publishMessage(client, topic, &pubmsg, &token);
    if (rc != MQTTCLIENT_SUCCESS) {
        printf("MQTT błąd wysyłania na temat %s, kod: %d\n", topic, rc);
    } else {
        MQTTClient_waitForCompletion(client, token, MQTT_TIMEOUT);
        printf("MQTT wysłano: %s = %s\n", topic, payload);
    }
}

int main(void) {
    printf("=== Sentinel Grid — Gateway z MQTT ===\n\n");
    
    // --- INICJALIZACJA MQTT ---
    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    
    MQTTClient_create(&client, MQTT_BROKER, MQTT_CLIENT_ID,
                      MQTTCLIENT_PERSISTENCE_NONE, NULL);
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    
    int rc = MQTTClient_connect(client, &conn_opts);
    if (rc != MQTTCLIENT_SUCCESS) {
        printf("Błąd połączenia z brokerem MQTT (kod: %d)\n", rc);
        printf("Upewnij się że Mosquitto działa: sudo systemctl start mosquitto\n");
        return 1;
    }
    printf("Połączono z brokerem MQTT: %s\n\n", MQTT_BROKER);
    
    // --- INICJALIZACJA CZUJNIKÓW ---
    printf("--- Inicjalizacja czujników ---\n");
    
    BME680_Device bme680 = bme680_init(BME680_I2C_ADDRESS);
    PMS5003_Device pms5003 = pms5003_init();
    MPU6050_Device mpu6050 = mpu6050_init();
    
    if (bme680_check_chip_id(&bme680) != BME680_OK) {
        printf("BŁĄD: BME680 nie odpowiada\n");
        return 1;
    }
    
    if (mpu6050_check_who_am_i(&mpu6050) != MPU6050_OK) {
        printf("BŁĄD: MPU6050 nie odpowiada\n");
        return 1;
    }
    
    printf("Wszystkie czujniki zainicjalizowane prawidłowo\n\n");
    
    // --- GŁÓWNA PĘTLA ---
    printf("--- Rozpoczęcie odczytów (Ctrl+C aby zakończyć) ---\n");
    
    for (int i = 0; i < 5; i++) {  // 5 odczytów, potem koniec
        printf("\n--- Odczyt %d ---\n", i + 1);
        
        // 1. BME680 - temperatura
        if (bme680_read_temperature(&bme680) == BME680_OK) {
            char payload[32];
            snprintf(payload, sizeof(payload), "%.2f", bme680.temperature);
            mqtt_send(client, TOPIC_BME680_TEMP, payload);
            
            // Alert: temperatura > 35°C
            if (bme680.temperature > 35.0f) {
                char alert[64];
                snprintf(alert, sizeof(alert), "{\"node\":\"01\",\"type\":\"high_temp\",\"value\":%.2f}", bme680.temperature);
                mqtt_send(client, TOPIC_ALERT, alert);
            }
        } else {
            printf("BME680: błąd odczytu\n");
        }
        
        // 2. PMS5003 - PM2.5 (symulujemy różne wartości dla przykładu)
        // W prawdziwym systemie czytałbyś z czujnika
        uint16_t test_values[] = {35, 65, 42, 88, 23};
        pms5003_simulate_data(&pms5003, test_values[i]);
        
        if (pms5003_read_frame(&pms5003) == PMS5003_OK) {
            uint16_t pm25 = pms5003_get_pm25(&pms5003);
            char payload[16];
            snprintf(payload, sizeof(payload), "%d", pm25);
            mqtt_send(client, TOPIC_PMS5003_PM25, payload);
            
            // Alert: PM2.5 > 50 µg/m³
            if (pm25 > 50) {
                char alert[64];
                snprintf(alert, sizeof(alert), "{\"node\":\"01\",\"type\":\"high_pm25\",\"value\":%d}", pm25);
                mqtt_send(client, TOPIC_ALERT, alert);
            }
        } else {
            printf("PMS5003: błąd odczytu\n");
        }
        
        // 3. MPU6050 - akcelerometr
        if (mpu6050_read_accel(&mpu6050) == MPU6050_OK) {
            char payload[64];
            snprintf(payload, sizeof(payload), "X=%d,Y=%d,Z=%d", 
                     mpu6050_get_accel_x(&mpu6050),
                     mpu6050_get_accel_y(&mpu6050),
                     mpu6050_get_accel_z(&mpu6050));
            mqtt_send(client, TOPIC_MPU6050_ACCEL, payload);
            
            // Alert: drgania na osi Z (spoczynek to ~16384, zakres 14000-18000)
            int16_t accel_z = mpu6050_get_accel_z(&mpu6050);
            if (accel_z < 14000 || accel_z > 18000) {
                char alert[64];
                snprintf(alert, sizeof(alert), "{\"node\":\"01\",\"type\":\"vibration\",\"z\":%d}", accel_z);
                mqtt_send(client, TOPIC_ALERT, alert);
            }
        } else {
            printf("MPU6050: błąd odczytu\n");
        }
        
        sleep(2);  // czekaj 2 sekundy przed następnym odczytem
    }
    
    // --- SPRZĄTANIE ---
    printf("\n--- Koniec testu ---\n");
    MQTTClient_disconnect(client, MQTT_TIMEOUT);
    MQTTClient_destroy(&client);
    
    return 0;
}