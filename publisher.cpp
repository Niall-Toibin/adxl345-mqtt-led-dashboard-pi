#include <iostream>
#include <fstream>
#include <ctime>
#include <string>
#include <cstring>
#include "MQTTClient.h"
#include "ADXL345.h"
#include "I2CDevice.h"

#define CPU_TEMP "/sys/class/thermal/thermal_zone0/temp"
using namespace std;

#define ADDRESS "tcp://192.168.1.121:1883"
#define CLIENTID "rpi1"
#define AUTHMETHOD "niall"
#define AUTHTOKEN "tkno2mmg"
#define TOPIC "een1071/test"
#define QOS 1
#define TIMEOUT 10000L


float getCPUTemperature() { // get the CPU temperature
    int cpuTemp;            // store as an int
    fstream fs;
    fs.open(CPU_TEMP, fstream::in); // read from the file
    fs >> cpuTemp;
    fs.close();
    return (((float)cpuTemp) / 1000);
}

string getLocalTime() {
    time_t now = time(NULL);
    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
    return string(buf);
}

int main(int argc, char* argv[]) {
    char str_payload[256];
    MQTTClient client;
    MQTTClient_connectOptions opts = MQTTClient_connectOptions_initializer;
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;
    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    opts.keepAliveInterval = 20;
    opts.cleansession = 1;
    opts.username = AUTHMETHOD;
    opts.password = AUTHTOKEN;
    int rc;
    if ((rc = MQTTClient_connect(client, &opts)) != MQTTCLIENT_SUCCESS) {
        cout << "Failed to connect, return code " << rc << endl;
        return -1;
    }
    
    ADXL345 sensor(1, 0x53);
    sensor.setRange(ADXL345::PLUSMINUS_4_G);
    sensor.setResolution(ADXL345::NORMAL);
    sensor.readSensorState();
    float pitch = sensor.getPitch();
    float roll  = sensor.getRoll();
    float temp = getCPUTemperature();
    string time = getLocalTime();
    
    snprintf(str_payload, sizeof(str_payload),
             "{\"d\":{\"pitch\":%.2f,\"roll\":%.2f,\"tempC\":%.2f,\"time\":\"%s\"}}",
             pitch, roll, temp, time.c_str());
    
    pubmsg.payload = str_payload;
    pubmsg.payloadlen = (int)strlen(str_payload);
    pubmsg.qos = QOS;
    pubmsg.retained = 0;
    
    MQTTClient_publishMessage(client, TOPIC, &pubmsg, &token);
    cout << "Waiting for up to " << (int)(TIMEOUT/1000)
         << " seconds for publication of " << str_payload
         << " on topic " << TOPIC << " for ClientID: " << CLIENTID << endl;
    rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
    cout << "Message with token " << (int)token << " delivered." << endl;
    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
    return rc;
}
