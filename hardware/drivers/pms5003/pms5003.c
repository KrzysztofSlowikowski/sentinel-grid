#include "pms5003.h"
#include <stdio.h>
#include <string.h>

PMS5003_Device pms5003_init(void) {
    PMS5003_Device device;
    
    // Zerujemy całą strukturę (ustawia wszystkie bajty na 0)
    //W BME680 ręcznie ustawiałeś każde pole. To działa, ale 
    //przy większych strukturach męczące. memset wypełnia cały blok pamięci zerami:
    memset(&device, 0, sizeof(device));
    //To jednym ruchem ustawia initialized=0, connected=0, last_error=0, 
    //cały frame_buffer na zera, wszystkie pm na 0, data_ready=0.

    //Potem ręcznie ustawiamy tylko te które mają być 1.
    
    device.initialized = 1;
    device.connected   = 1;
    device.last_error  = PMS5003_OK;
    device.data_ready  = 0;
    
    return device;
}

void pms5003_simulate_data(PMS5003_Device *device, uint16_t pm25_value) {
    if (device->initialized == 0) return;
    
    // Tworzymy ramkę dla PM2.5 = zadana wartość
    uint8_t fake_frame[32] = {0};
    
    // Start bajty
    fake_frame[0] = 0x42;
    fake_frame[1] = 0x4D;
    
    // Długość ramki (2 bajty, little-endian)
    fake_frame[2] = 0x00;  // high byte = 0
    fake_frame[3] = 0x1C;  // low byte = 28
    
    // PM2.5 (dwa bajty, little-endian: młodszy pierwszy)
    fake_frame[6] = pm25_value & 0xFF;        // młodszy bajt
    fake_frame[7] = (pm25_value >> 8) & 0xFF; // starszy bajt

    uint16_t check_sum = 0;
    for (int i = 0; i < 30; i++) {
        check_sum += fake_frame[i];
    }
    fake_frame[30] = (uint8_t)(check_sum & 0xFF);
    fake_frame[31] = 0x00;
    
    // Uproszczenie: pomijamy inne wartości (PM1.0, PM10)
    // i sumę kontrolną (na razie)
    
    // Kopiujemy do bufora urządzenia
    memcpy(device->frame_buffer, fake_frame, 32);
    device->data_ready = 1;
    
    printf("--- Symulacja: nowe dane z czujnika (PM2.5 = %d μg/m³) ---\n", pm25_value);
}
//To nowy koncept. W BME680 nie mieliśmy czegoś takiego, bo tam po prostu ustawiałeś rejestry. 
//Tutaj potrzebujemy funkcji która zasymuluje nadejście ramki z czujnika.

uint8_t pms5003_calculate_checksum(PMS5003_Device *device) {
    uint16_t sum = 0;
    
    // Dodajemy wszystkie bajty od 0 do 29
    for (int i = 0; i < 30; i++) {
        sum += device->frame_buffer[i];
    }
    
    // Zwracamy tylko młodszy bajt (mod 256)
    return (uint8_t)(sum & 0xFF);
}

PMS5003_Status pms5003_read_frame(PMS5003_Device *device) {
    if (device->initialized == 0) {
        device->last_error = PMS5003_ERR_NOT_INIT;
        return PMS5003_ERR_NOT_INIT;
    }
    
    if (device->connected == 0) {
        device->last_error = PMS5003_ERR_NO_FRAME;
        printf("Błąd: czujnik nie odpowiada\n");
        return PMS5003_ERR_NO_FRAME;
    }
    
    if (device->data_ready == 0) {
        device->last_error = PMS5003_ERR_TIMEOUT;
        printf("Błąd: brak danych (timeout)\n");
        return PMS5003_ERR_TIMEOUT;
    }
    
    // Sprawdź czy ramka zaczyna się od 0x42 0x4D
    if (device->frame_buffer[0] != 0x42 || device->frame_buffer[1] != 0x4D) {
        device->last_error = PMS5003_ERR_NO_FRAME;
        printf("Błąd: nie znaleziono początku ramki (0x%02X 0x%02X)\n",
               device->frame_buffer[0], device->frame_buffer[1]);
        return PMS5003_ERR_NO_FRAME;
    }
    
    // Wyciągnij PM2.5 (dwa bajty, little-endian)
    uint16_t pm25_raw = device->frame_buffer[PMS5003_PM25_CF1_LOW] |
                        (device->frame_buffer[PMS5003_PM25_CF1_HIGH] << 8);

    uint8_t received_checksum = device->frame_buffer[PMS5003_CHECKSUM_IDX];
    uint8_t calculated_checksum = pms5003_calculate_checksum(device);
    
    if (received_checksum != calculated_checksum) {
        device->last_error = PMS5003_ERR_BAD_CHECKSUM;
        printf("Błąd: suma kontrolna nie zgadza się (otrzymano 0x%02X, obliczono 0x%02X)\n",
               received_checksum, calculated_checksum);
        device->data_ready = 0;  // odrzucamy uszkodzone dane
        return PMS5003_ERR_BAD_CHECKSUM;
    }
    
    // Zapisz w strukturze
    device->pm2_5_cf1 = pm25_raw;
    
    // Po odczytaniu, dane już nie są świeże
    device->data_ready = 0;
    device->last_error = PMS5003_OK;
    
    return PMS5003_OK;
}

uint16_t pms5003_get_pm25(PMS5003_Device *device) {
    return device->pm2_5_cf1;
}

void pms5003_print_status(PMS5003_Device *device) {
    printf("=== PMS5003 Status ===\n");
    printf("Połączony:    %s\n", device->connected   ? "TAK" : "NIE");
    printf("Gotowy:       %s\n", device->initialized ? "TAK" : "NIE");
    printf("Dane świeże:  %s\n", device->data_ready  ? "TAK" : "NIE");
    printf("PM2.5:        %d μg/m³\n", device->pm2_5_cf1);
    printf("Ostatni błąd: %d\n", device->last_error);
    printf("=====================\n");
}