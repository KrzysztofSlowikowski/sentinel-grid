#include "mqtt_client.h"
#include <stdio.h>
#include <string.h>

// Zmienne statyczne — widoczne tylko w tym pliku
static MQTTClient client;
static int connected = 0;
static MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;

// Callback dla przychodzących wiadomości (na razie pusty, potem rozszerzymy)
static int message_callback(void* context, char* topicName, int topicLen, MQTTClient_message* message) {
    (void)context;      // informuje kompilator: "wiem że nie używam, to celowe"
    (void)topicLen;     // to samo
    
    printf("Odebrano MQTT: %s -> %.*s\n", topicName, message->payloadlen, (char*)message->payload);
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;           // funkcja musi zwracać int (1 = sukces)
}

int mqtt_init(const char* broker, const char* client_id) {
    int rc = MQTTClient_create(&client, broker, client_id,
                                MQTTCLIENT_PERSISTENCE_NONE, NULL);
    if (rc != MQTTCLIENT_SUCCESS) {
        printf("MQTT: błąd tworzenia klienta (kod: %d)\n", rc);
        return -1;
    }
    
    // Ustaw callback dla przychodzących wiadomości
    MQTTClient_setCallbacks(client, NULL, NULL, message_callback, NULL);
    
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    
    printf("MQTT: klient utworzony (broker: %s, id: %s)\n", broker, client_id);
    return 0;
}

int mqtt_connect(void) {
    int rc = MQTTClient_connect(client, &conn_opts);
    if (rc != MQTTCLIENT_SUCCESS) {
        printf("MQTT: błąd połączenia (kod: %d)\n", rc);
        connected = 0;
        return -1;
    }
    connected = 1;
    printf("MQTT: połączono\n");
    return 0;
}

int mqtt_is_connected(void) {
    return connected;
}

int mqtt_publish(const char* topic, const char* payload, int qos) {
    if (!connected) {
        printf("MQTT: nie połączono, nie można wysłać\n");
        return -1;
    }
    
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;
    
    pubmsg.payload = (char*)payload;
    pubmsg.payloadlen = strlen(payload);
    pubmsg.qos = qos;
    pubmsg.retained = 0;
    
    int rc = MQTTClient_publishMessage(client, topic, &pubmsg, &token);
    if (rc != MQTTCLIENT_SUCCESS) {
        printf("MQTT: błąd wysyłania na %s (kod: %d)\n", topic, rc);
        return -1;
    }
    
    MQTTClient_waitForCompletion(client, token, 10000L);
    return 0;
}

int mqtt_subscribe(const char* topic, int qos, mqtt_callback_t callback) {
    (void)callback;  // na razie nie używamy, ale interfejs pozostawiamy dla przyszłości
    
    if (!connected) {
        printf("MQTT: nie połączono, nie można subskrybować\n");
        return -1;
    }
    
    int rc = MQTTClient_subscribe(client, topic, qos);
    if (rc != MQTTCLIENT_SUCCESS) {
        printf("MQTT: błąd subskrypcji %s (kod: %d)\n", topic, rc);
        return -1;
    }
    
    printf("MQTT: subskrybuję %s\n", topic);
    return 0;
}

void mqtt_disconnect(void) {
    if (connected) {
        MQTTClient_disconnect(client, 10000L);
        connected = 0;
        printf("MQTT: rozłączono\n");
    }
}

void mqtt_cleanup(void) {
    mqtt_disconnect();
    MQTTClient_destroy(&client);
    printf("MQTT: posprzątano\n");
}

MQTTClient mqtt_get_client(void) {
    return client;
}