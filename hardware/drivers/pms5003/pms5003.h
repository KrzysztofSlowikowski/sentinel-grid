#ifndef PMS5003_H
#define PMS5003_H

#include <stdint.h>

typedef enum {
    PMS5003_OK                =  0,
    PMS5003_ERR_NOT_INIT      = -1,
    PMS5003_ERR_NO_FRAME      = -2,   // nie znaleziono początku ramki
    PMS5003_ERR_BAD_CHECKSUM  = -3,   // suma kontrolna się nie zgadza
    PMS5003_ERR_TIMEOUT       = -4    // czujnik nie wysłał danych
} PMS5003_Status;

// Start bajty ramki
#define PMS5003_START_BYTE_1  0x42
#define PMS5003_START_BYTE_2  0x4D

#define PMS5003_FRAME_LENGTH  32      // cała ramka to 32 bajty
#define PMS5003_DATA_START    4       // dane zaczynają się od bajtu 4

#define PMS5003_PM25_CF1_LOW  6       // bajt 6 = młodszy bajt PM2.5
#define PMS5003_PM25_CF1_HIGH 7       // bajt 7 = starszy bajt PM2.5

#define PMS5003_CHECKSUM_IDX  30 


//W BME680 składałeś 20 bitów z trzech bajtów używając przesunięć. 
//Tutaj PM2.5 to 16-bitowa liczba (0-65535) zapisana w dwóch bajtach.

typedef struct {
    uint8_t      initialized;
    uint8_t      connected;
    PMS5003_Status last_error;
    
    // Bufor na surowe dane z UART
    uint8_t      frame_buffer[32];
    
    // Ostatnie odczytane wartości
    uint16_t     pm1_0_cf1;    // PM1.0 w μg/m³
    uint16_t     pm2_5_cf1;    // PM2.5 w μg/m³
    uint16_t     pm10_cf1;     // PM10 w μg/m³
    
    // Flaga czy mamy świeże dane
    uint8_t      data_ready;
} PMS5003_Device;

PMS5003_Device pms5003_init(void);
PMS5003_Status pms5003_read_frame(PMS5003_Device *device);
uint16_t       pms5003_get_pm25(PMS5003_Device *device);
void           pms5003_print_status(PMS5003_Device *device);
void           pms5003_simulate_data(PMS5003_Device *device, uint16_t pm25_value);
uint8_t pms5003_calculate_checksum(PMS5003_Device *device);

#endif