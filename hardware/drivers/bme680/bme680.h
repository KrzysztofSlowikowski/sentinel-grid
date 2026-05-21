#ifndef BME680_H //jeśli nie zdefiniowany plik
#define BME680_H //to zdefiniuj, jakby dwa pliki zawiera ten sam plik to bez
//tego by się nie skompilowało

#include <stdint.h> //biblioteka standardowa która daje typy o konkretnych
//wielkosciach
typedef enum {
    BME680_OK                =  0,
    BME680_ERR_NOT_INIT      = -1,
    BME680_ERR_NO_RESPONSE   = -2,
    BME680_ERR_WRONG_CHIP_ID = -3,
    BME680_ERR_BAD_DATA      = -4
} BME680_Status;

#define BME680_I2C_ADDRESS    0x76
#define BME680_REG_CHIP_ID    0xD0
#define BME680_REG_TEMP_MSB   0xFA
#define BME680_REG_TEMP_LSB   0xFB
#define BME680_REG_TEMP_XLSB  0xFC
#define BME680_CHIP_ID_VALUE  0x61
//0x76 to adres I2C czujnika BME680, każdy czujnik na magistrali ma unikalny
//adres - gdy Pi chce porozmawiać z BME680, woła po ty adresie, inne czujniki
//słyszą wołanie ale ignorują je bo adres nie pasuje
//0x61 to rejestr identyfikacyjny, potrzebny do weryfikacji
// 0xFA,0xFB,0xFC to temperatura zapisana w 3 rejestrach 
//(bo jeden bajt to za mało by zapisać)

typedef struct {
    uint8_t address;  //adres czujnika (7 bitów) wystarczy bajt
    uint8_t chip_id;  //identyfikator czujnika
    uint8_t registers[256]; //serce symulatora, tworzymy tablice 256 bajtów
//która uudaje pamięć czujnika (każdy indeks tablicy to jeden rejestr)
    float   temperature; //obliczona temperatura w celsjuszach
    uint8_t initialized; //flaga 0 - niezainicjalizowany, 1 - gotowy do pracy
//zamiast wartości true/false mamy zero/jeden bo C nie ma wbudowanego boolean
    uint8_t      connected;
    BME680_Status last_error;
} BME680_Device; //taka kontrukcja z typedef pozwala na defioniowanie 
// zmiennych w sposób BME680_Device device1;

//Struct to sposób żeby zdefioniować kilka zmiennych w jeden obiekt, struct 
//to taki formularz w kodzie

BME680_Device bme680_init(uint8_t address);
BME680_Status bme680_check_chip_id(BME680_Device *device);
BME680_Status bme680_read_temperature(BME680_Device *device);
void          bme680_print_status(BME680_Device *device);
void          bme680_simulate_disconnect(BME680_Device *device);
void          bme680_simulate_reconnect(BME680_Device *device);

#endif // BME680_H
//to są deklaracje funkcji:
//Funkcja bme680_init przyjmuje adres I2C (uint8_t address) i zwraca gotowy
//obiekt BME680_Device
//uint8_t bme680_check_chip_id(BME680_Device *device);
//Przyjmuje wskaźnik na urządzenie (*device) i zwraca 1 jeśli chip id 
//się zgadza, 0 jeśli nie.
//float bme680_read_temperature(BME680_Device *device);
//Przyjmuje wskaźnik na urządzenie, zwraca temperaturę jako float
//wskaźniki w C:
//bez nich funkcja tworzyła by kopię obiektu co przy dużych strukturach
//zajmuje więcej miejsca co znacznik czyli adres na dany obiekt który sprawia
//ze funkcja nie kopiuje tylko modyfikuje to konkretne miejsce w pamieci