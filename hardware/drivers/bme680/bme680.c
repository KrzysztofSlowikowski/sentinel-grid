#include "bme680.h"   // cudzysłów — nasz własny plik (szuka w folderze co ten plik)
#include <stdio.h>    // nawiasy kątowe — biblioteka systemowa (szukaj w 
// /usr/include/ na Linuxie)

BME680_Device bme680_init(uint8_t address) { //zwraca obiekt typppu device
    BME680_Device device; //deklaracja zmiennej na stosie funkcji
//w tym momencie pojawiają się 263 bajty na stosie (tyle ma obiekt typu device)
//ten device jest wogule lokalną nazwą, na zewnątrz funkcji nie istnieje
//funkcja zwraca zawartość, nie ma wpływu na nazwę obiektu
    
    device.address     = address;
    device.chip_id     = 0;
    device.temperature = 0.0f;
    device.initialized = 0;
    device.connected   = 0;
    device.last_error  = BME680_OK;
    
    device.registers[BME680_REG_CHIP_ID]   = BME680_CHIP_ID_VALUE;
    device.registers[BME680_REG_TEMP_MSB]  = 0x1D;
//ustawiamy tablice registers, wartościamy które udają prawdziwy czujnik
    device.registers[BME680_REG_TEMP_LSB]  = 0x40;
    device.registers[BME680_REG_TEMP_XLSB] = 0x00;
    
    device.initialized = 1; //flage ustawiamy na koncu
    device.connected   = 1;
    
    return device; //zwracamy kopie, oryginał na stosie znika
}
//to jest konstruktor czujnika, tworzy i przygotowuje obiekt do pracy
//Tworzy zmienną typu BME680_Device w pamięci. Na razie ma śmieciowe wartości 
//Ustawiamy wszystkie pola na znane wartości

BME680_Status bme680_check_chip_id(BME680_Device *device) {
    if (device->initialized == 0) {
        device->last_error = BME680_ERR_NOT_INIT;
        printf("Błąd: czujnik nie jest zainicjalizowany\n");
        return BME680_ERR_NOT_INIT;
    }

    if (device->connected == 0) {
        device->last_error = BME680_ERR_NO_RESPONSE;
        printf("Błąd: czujnik nie odpowiada\n");
        return BME680_ERR_NO_RESPONSE;
    }

    device->chip_id = device->registers[BME680_REG_CHIP_ID];

    if (device->chip_id == BME680_CHIP_ID_VALUE) {
        printf("Chip ID: 0x%02X — poprawny\n", device->chip_id);
        device->last_error = BME680_OK;
        return BME680_OK;
    } else {
        device->last_error = BME680_ERR_WRONG_CHIP_ID;
        printf("Chip ID: 0x%02X — błędny, oczekiwano 0x%02X\n",
               device->chip_id, BME680_CHIP_ID_VALUE);
        return BME680_ERR_WRONG_CHIP_ID;
    }
}
//czemu strzałka zamiast kropki? w poprzedniej funkcji pisaliśmy device.address
//wynika to z tego że tamta operowała na kopii, przyjmowała koppie obiektu, ta
//funkcja pzryjmuje wskaźnik, czyli adres w pamięci gdzie jest obiekt, żeby 
//dostać się do takiego pola trzeba użyć strzałki jak tu lub (*device).chip_id
//czegoś takiego

BME680_Status bme680_read_temperature(BME680_Device *device) {
    if (device->initialized == 0) {
        device->last_error = BME680_ERR_NOT_INIT;
        return BME680_ERR_NOT_INIT;
    }

    if (device->connected == 0) {
        device->last_error = BME680_ERR_NO_RESPONSE;
        printf("Błąd: czujnik nie odpowiada\n");
        return BME680_ERR_NO_RESPONSE;
    }

    uint32_t temp_raw = 0;

    temp_raw  = (uint32_t)device->registers[BME680_REG_TEMP_MSB]  << 12;
//krok 1 - przesuwamy o 12 bitów w lewo
//Operator |= to skrót — a |= b znaczy a = a | b
//po prostu robi to nam liczbę
    temp_raw |= (uint32_t)device->registers[BME680_REG_TEMP_LSB]  << 4;
//krok 2 - przesuwamy bity lsb o 4 w lewo plus OR
    temp_raw |= (uint32_t)device->registers[BME680_REG_TEMP_XLSB] >> 4;
//krok 3 - przesuwamy bity xlsb o 4 w prawo plus OR

    float temp = (float)temp_raw / 5120.0f;

    if (temp < -40.0f || temp > 85.0f) {
        device->last_error = BME680_ERR_BAD_DATA;
        printf("Błąd: temperatura poza zakresem (%.2f°C)\n", temp);
        return BME680_ERR_BAD_DATA;
    }

    device->temperature = temp;
    device->last_error  = BME680_OK;
    return BME680_OK;
}

//Czujnik ma 20-bitowy pomiar temperatury. 
//Jeden bajt to 8 bitów — za mało. Czujnik rozkłada 20 bitów na trzy rejestry:
//TEMP_MSB  (0xFA) = 0x7E = 0111 1110
//TEMP_LSB  (0xFB) = 0x90 = 1001 0000
//TEMP_XLSB (0xFC) = 0x00 = 0000 0000
//Ale te bajty nie siedzą w rejestrach poukładane
//ładnie jeden za drugim. Każdy bajt zawiera konkretne bity wyniku:

void bme680_print_status(BME680_Device *device) {
    printf("=== BME680 Status ===\n");
    printf("Adres I2C:    0x%02X\n", device->address);
    printf("Chip ID:      0x%02X\n", device->chip_id);
    printf("Temperatura:  %.2f°C\n", device->temperature);
    printf("Połączony:    %s\n", device->connected   ? "TAK" : "NIE");
    printf("Gotowy:       %s\n", device->initialized ? "TAK" : "NIE");
    printf("Ostatni błąd: %d\n",  device->last_error);
    printf("=====================\n");
}

void bme680_simulate_disconnect(BME680_Device *device) {
    device->connected  = 0;
    device->last_error = BME680_ERR_NO_RESPONSE;
    printf("--- Symulacja: czujnik odłączony ---\n");
}

void bme680_simulate_reconnect(BME680_Device *device) {
    device->connected  = 1;
    device->last_error = BME680_OK;
    printf("--- Symulacja: czujnik podłączony ponownie ---\n");
}