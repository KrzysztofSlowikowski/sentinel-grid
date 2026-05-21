#include <stdio.h>
#include "pms5003.h"

int main(void) {
    printf("=== Sentinel Grid — test czujnika PMS5003 ===\n\n");
    
    // Inicjalizacja czujnika
    PMS5003_Device czujnik = pms5003_init();
    
    // --- SCENARIUSZ 1: normalna praca ---
    printf("--- Scenariusz 1: normalny odczyt ---\n");
    
    // Symulujemy nadejście danych z czujnika (PM2.5 = 35 μg/m³)
    pms5003_simulate_data(&czujnik, 35);
    
    // Odczytujemy ramkę
    PMS5003_Status status = pms5003_read_frame(&czujnik);
    if (status == PMS5003_OK) {
        printf("Odczyt PM2.5: %d μg/m³\n", pms5003_get_pm25(&czujnik));
    } else {
        printf("Błąd odczytu: %d\n", status);
    }
    
    pms5003_print_status(&czujnik);
    
    // --- SCENARIUSZ 2: próba odczytu bez nowych danych ---
    printf("\n--- Scenariusz 2: odczyt bez nowych danych (timeout) ---\n");
    
    // Nie wywołujemy simulate_data — data_ready = 0
    status = pms5003_read_frame(&czujnik);
    if (status != PMS5003_OK) {
        printf("Oczekiwany błąd: brak danych (kod: %d)\n", status);
    }
    
    pms5003_print_status(&czujnik);
    
    // --- SCENARIUSZ 3: symulacja odłączenia czujnika ---
    printf("\n--- Scenariusz 3: czujnik odłączony ---\n");
    czujnik.connected = 0;
    
    pms5003_simulate_data(&czujnik, 50);  // próbujemy symulować dane
    status = pms5003_read_frame(&czujnik);
    if (status != PMS5003_OK) {
        printf("Oczekiwany błąd: czujnik nie odpowiada (kod: %d)\n", status);
    }
    
    pms5003_print_status(&czujnik);
    
    // --- SCENARIUSZ 4: powrót czujnika i nowe dane ---
    printf("\n--- Scenariusz 4: powrót połączenia ---\n");
    czujnik.connected = 1;
    
    pms5003_simulate_data(&czujnik, 120);
    status = pms5003_read_frame(&czujnik);
    if (status == PMS5003_OK) {
        printf("Odczyt PM2.5 po powrocie: %d μg/m³\n", pms5003_get_pm25(&czujnik));
    }
    
    pms5003_print_status(&czujnik);
    
    // --- SCENARIUSZ 5: uszkodzona ramka (złe start bajty) ---
    printf("\n--- Scenariusz 5: uszkodzona ramka ---\n");
    
    // Ręcznie psujemy początek ramki
    czujnik.frame_buffer[0] = 0xFF;
    czujnik.frame_buffer[1] = 0xFF;
    czujnik.data_ready = 1;
    
    status = pms5003_read_frame(&czujnik);
    if (status != PMS5003_OK) {
        printf("Wykryto uszkodzoną ramkę (kod: %d)\n", status);
    }
    
    pms5003_print_status(&czujnik);
     // --- SCENARIUSZ 6: uszkodzone dane (zła suma kontrolna) ---
    printf("\n--- Scenariusz 6: uszkodzone dane (zła suma kontrolna) ---\n");
    
    // Symulujemy dobre dane
    pms5003_simulate_data(&czujnik, 80);
    
    // Ręcznie psujemy sumę kontrolną
    czujnik.frame_buffer[30] = 0xFF;  // zły checksum
    
    status = pms5003_read_frame(&czujnik);
    if (status != PMS5003_OK) {
        printf("Wykryto uszkodzone dane (kod: %d)\n", status);
    }
    
    pms5003_print_status(&czujnik);
    
    return 0;
}