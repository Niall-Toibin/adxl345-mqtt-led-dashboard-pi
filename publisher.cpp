#include <iostream>
#include <fstream>
#include <ctime>
#include <string>
#include <cstring>
#include <unistd.h>
#include "MQTTClient.h"
#include "ADXL345.h"
#include "I2CDevice.h"

#define CPU_TEMP "/sys/class/thermal/thermal_zone0/temp"
using namespace std;
using namespace exploringRPi;


#define ADDRESS "tcp://192.168.1.124:1883"
#define CLIENTID "rpi1"
#define AUTHMETHOD "niall"
#define AUTHTOKEN "tkno2mmg"
#define TOPIC "een1071/test"
#define PITCH_TOPIC "een1071/pitch"
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
    MQTTClient client;
    MQTTClient_connectOptions opts = MQTTClient_connectOptions_initializer;
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;
    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    opts.keepAliveInterval = 20;
    opts.cleansession = 1;
    opts.username = AUTHMETHOD;
    opts.password = AUTHTOKEN;

    MQTTClient_willOptions will_opts = MQTTClient_willOptions_initializer;
    will_opts.struct_version = 0; 
    will_opts.topicName = "een1071/LastWill";
    will_opts.message   = "Publisher disconnected unexpectedly";
    will_opts.qos       = QOS;
    will_opts.retained  = 0;
    opts.will = &will_opts;

    
    int rc;
    if ((rc = MQTTClient_connect(client, &opts)) != MQTTCLIENT_SUCCESS) {
        cout << "Failed to connect, return code " << rc << endl;
        return -1;
    }
    
    ADXL345 sensor(1, 0x53);
    sensor.setRange(ADXL345::PLUSMINUS_4_G);
    sensor.setResolution(ADXL345::NORMAL);
    while (true) {
        sensor.readSensorState();
        float pitch = sensor.getPitch();
        float roll  = sensor.getRoll();
        float temp  = getCPUTemperature();
        string timeStr = getLocalTime();

        char str_payload[256];
        snprintf(str_payload, sizeof(str_payload),
            "{\"d\":{\"pitch\":%.2f,\"roll\":%.2f,\"tempC\":%.2f,\"time\":\"%s\"}}",
            pitch, roll, temp, timeStr.c_str());

        pubmsg.payload = str_payload;
        pubmsg.payloadlen = (int)strlen(str_payload);
        pubmsg.qos = QOS;
        pubmsg.retained = 0;

        MQTTClient_publishMessage(client, TOPIC, &pubmsg, &token);
        cout << "Published: " << str_payload 
             << " to topic " << TOPIC << endl;
        rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);

        // Also publish pitch to een1071/pitch
        char pitch_payload[256];
        snprintf(pitch_payload, sizeof(pitch_payload),
                "{\"d\":{\"pitch\":%.2f}}", pitch);

        pubmsg.payload = pitch_payload;
        pubmsg.payloadlen = (int)strlen(pitch_payload);

        MQTTClient_publishMessage(client, PITCH_TOPIC, &pubmsg, &token);
        cout << "Published pitch-only to " << PITCH_TOPIC << endl;
        rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);

        sleep(1);
    }

    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
    return rc;
}