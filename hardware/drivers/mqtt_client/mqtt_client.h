#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include <MQTTClient.h>

// Typ dla callbacku (funkcja którą wywołamy gdy przyjdzie wiadomość)
typedef void (*mqtt_callback_t)(const char* topic, const char* payload);

// Inicjalizacja i połączenie
int mqtt_init(const char* broker, const char* client_id);
int mqtt_connect(void);
int mqtt_is_connected(void);

// Wysyłanie
int mqtt_publish(const char* topic, const char* payload, int qos);

// Subskrypcja
int mqtt_subscribe(const char* topic, int qos, mqtt_callback_t callback);

// Sprzątanie
void mqtt_disconnect(void);
void mqtt_cleanup(void);

// Getter dla wewnętrznego klienta (jeśli potrzebny)
MQTTClient mqtt_get_client(void);

#endif