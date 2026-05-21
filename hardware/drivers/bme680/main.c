#include <stdio.h>
#include "bme680.h"

int main(void) {
    printf("=== Sentinel Grid — test czujnika BME680 ===\n\n");

    // --- SCENARIUSZ 1: normalna praca ---
    printf("--- Scenariusz 1: normalna praca ---\n");
    BME680_Device czujnik = bme680_init(BME680_I2C_ADDRESS);

    BME680_Status status = bme680_check_chip_id(&czujnik);
    if (status != BME680_OK) {
        printf("Inicjalizacja nieudana, kod błędu: %d\n", status);
        return 1;
    }

    status = bme680_read_temperature(&czujnik);
    if (status == BME680_OK) {
        printf("Temperatura: %.2f°C\n", czujnik.temperature);
    }

    bme680_print_status(&czujnik);

    // --- SCENARIUSZ 2: czujnik się odłącza ---
    printf("--- Scenariusz 2: awaria połączenia ---\n");
    bme680_simulate_disconnect(&czujnik);

    status = bme680_read_temperature(&czujnik);
    if (status != BME680_OK) {
        printf("Nie udało się odczytać temperatury, kod: %d\n", status);
    }

    bme680_print_status(&czujnik);

    // --- SCENARIUSZ 3: czujnik wraca ---
    printf("--- Scenariusz 3: powrót połączenia ---\n");
    bme680_simulate_reconnect(&czujnik);

    status = bme680_read_temperature(&czujnik);
    if (status == BME680_OK) {
        printf("Temperatura po powrocie: %.2f°C\n", czujnik.temperature);
    }

    // --- SCENARIUSZ 4: przekłamane dane ---
    printf("--- Scenariusz 4: przekłamane dane w rejestrach ---\n");
    czujnik.registers[BME680_REG_TEMP_MSB]  = 0xFF;
    czujnik.registers[BME680_REG_TEMP_LSB]  = 0xFF;
    czujnik.registers[BME680_REG_TEMP_XLSB] = 0xFF;

    status = bme680_read_temperature(&czujnik);
    if (status != BME680_OK) {
        printf("Wykryto przekłamane dane, kod: %d\n", status);
    }

    bme680_print_status(&czujnik);

    // --- SCENARIUSZ 5 ---
    czujnik.registers[BME680_REG_CHIP_ID] = 0x00;

    status = bme680_check_chip_id(&czujnik);
    if (status != BME680_OK) {
        printf("Zły adres chipu: kod błędu %d\n", status);
    }
    bme680_print_status(&czujnik);

    return 0;
}
//Żeby przekazać wskaźnik do istniejącej zmiennej, używasz 
//operatora & który znaczy "daj mi adres tej zmiennej w pamięci":



//Każdy program w C musi mieć funkcję main — to punkt wejścia, 
//pierwsze miejsce gdzie program zaczyna działać.
