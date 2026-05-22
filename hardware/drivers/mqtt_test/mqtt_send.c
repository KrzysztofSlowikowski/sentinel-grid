#include <stdio.h>
#include <string.h>
#include <MQTTClient.h>

// Konfiguracja połączenia
#define ADDRESS     "tcp://localhost:1883"
#define CLIENTID    "sentinel_test_client"
#define TOPIC       "sentinel/test"
#define QOS         1
#define TIMEOUT     10000L

int main(int argc, char* argv[]) {
    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;
    int rc;

    // 1. Utwórz klienta
    MQTTClient_create(&client, ADDRESS, CLIENTID,
                      MQTTCLIENT_PERSISTENCE_NONE, NULL);
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;

    // 2. Połącz z brokerem
    rc = MQTTClient_connect(client, &conn_opts);
    if (rc != MQTTCLIENT_SUCCESS) {
        printf("Błąd połączenia, kod: %d\n", rc);
        return 1;
    }
    printf("Połączono z brokerem MQTT\n");

    // 3. Przygotuj wiadomość
    char* payload = "Hello z sentinel-grid C!";
    pubmsg.payload = payload;
    pubmsg.payloadlen = strlen(payload);
    pubmsg.qos = QOS;
    pubmsg.retained = 0;

    // 4. Opublikuj
    MQTTClient_publishMessage(client, TOPIC, &pubmsg, &token);
    printf("Wysłano wiadomość na temat: %s\n", TOPIC);
    rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
    printf("Wiadomość dostarczona (token: %d)\n", token);

    // 5. Rozłącz
    MQTTClient_disconnect(client, TIMEOUT);
    MQTTClient_destroy(&client);

    return 0;
}