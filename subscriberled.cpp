#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "MQTTClient.h"
#include <iostream>
#include <gpiod.h>
using namespace std;

#define ADDRESS    "tcp://192.168.1.121:1883"
#define CLIENTID   "led"
#define AUTHMETHOD "niall"
#define AUTHTOKEN  "tkno2mmg"
#define TOPIC      "een1071/test"
#define QOS        1
#define TIMEOUT    10000L

volatile MQTTClient_deliveryToken deliveredtoken;

// Prints token number of delivered message
void delivered(void *context, MQTTClient_deliveryToken dt) {
    printf("Message with token value %d delivery confirmed\n", dt);
    deliveredtoken = dt;
}

float getPitch(const char* payload) {
    // searches for 'pitch'
    const char* found = strstr(payload, "\"pitch\":");
    if(!found) {
      return 0.0;
    }
    return atof(found + 8);
}

struct gpiod_chip *chip = nullptr;
struct gpiod_line *line = nullptr;

// Prints topic & payload, extracts pitch value & controls GPIO pin
int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
    int i;
    char* payloadptr;
    printf("Message arrived\n");
    printf("  topic: %s\n", topicName);
    printf("  message: ");
    payloadptr = (char*) message->payload;
    for(i=0; i<message->payloadlen; i++) {
        putchar(*payloadptr++);
    }
    putchar('\n');

    payloadptr = (char*) message->payload;
    float pitchVal = getPitch(payloadptr);

    // Turn LED on if pitch > 20.
    if(pitchVal > 20) {
        gpiod_line_set_value(line, 1);
        cout << "Pitch > 20. LED switched on." << endl;
    } else {
        gpiod_line_set_value(line, 0);
        cout << "Pitch < 20. LED switched off." << endl;
    }

    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}

// Prints reason for lost connection
void connlost(void *context, char *cause) {
    printf("\nConnection lost\n");
    printf("  cause: %s\n", cause);
}

int main(int argc, char* argv[]) {

    // Open chip and line
    chip = gpiod_chip_open("/dev/gpiochip0");
    line = gpiod_chip_get_line(chip, 17);

    gpiod_line_request_output(line, "led_actuator", 0);


    MQTTClient client;
    MQTTClient_connectOptions opts = MQTTClient_connectOptions_initializer;
    int rc;
    int ch;

    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    opts.keepAliveInterval = 20;
    opts.cleansession = 0;
    opts.username = AUTHMETHOD;
    opts.password = AUTHTOKEN;

    MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered);

    if ((rc = MQTTClient_connect(client, &opts)) != MQTTCLIENT_SUCCESS) {
        printf("Failed to connect, return code %d\n", rc);
        exit(-1);
    }
    printf("Subscribing to topic %s\nfor client %s using QoS%d\n\n"
           "Press Q<Enter> to quit\n\n", TOPIC, CLIENTID, QOS);
    MQTTClient_subscribe(client, TOPIC, QOS);

    do {
        ch = getchar();
    } while(ch!='Q' && ch != 'q');

    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
    return rc;
}
