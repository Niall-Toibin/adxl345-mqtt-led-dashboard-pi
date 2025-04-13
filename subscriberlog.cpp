#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "MQTTClient.h"
#include <fstream>
#include <iostream>
using namespace std;

#define ADDRESS    "tcp://192.168.1.121:1883"
#define CLIENTID   "logger"
#define AUTHMETHOD "niall"
#define AUTHTOKEN  "tkno2mmg"
#define TOPIC      "een1071/test"
#define QOS        1
#define TIMEOUT    10000L

volatile MQTTClient_deliveryToken deliveredtoken;

void delivered(void *context, MQTTClient_deliveryToken dt) {
    printf("Message with token value %d delivery confirmed\n", dt);
    deliveredtoken = dt;
}

string getLog(const char* payload, const char* key) {
    string search = string("\"") + key + "\":";
    const char* found = strstr(payload, search.c_str());
    found += search.length();

    if (*found == '"') { // time
        found++;
        string value;
        while (*found && *found != '"') value += *found++;
        return value;
    } else { // not time
        string value;
        while (*found && *found != ',' && *found != '}') value += *found++;
        return value;
    }
}

int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
    char* payloadptr = (char*) message->payload;

    string time  = getLog(payloadptr, "time");
    string pitch = getLog(payloadptr, "pitch");
    string roll  = getLog(payloadptr, "roll");
    string temp  = getLog(payloadptr, "tempC");

    ofstream logFile("sensor_log.txt", ios::app);
    if (logFile.is_open()) {
        logFile << time << ", Pitch: " << pitch << ", Roll: " << roll << ", Temp: " << temp << "°C" << endl;
        logFile.close();
        cout << "Logged: " << time << " Pitch: " << pitch << " Roll: " << roll << " Temp: " << temp << endl;
    }

    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}

void connlost(void *context, char *cause) {
    printf("\nConnection lost\n");
    printf("  cause: %s\n", cause);
}

int main(int argc, char* argv[]) {
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
