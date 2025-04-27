# 📡 IoT Sensor Data Visualization with MQTT & Qt (Raspberry Pi + ADXL345)

## Overview
This project provides a complete IoT data acquisition and visualization pipeline using:

- **MQTT Protocol** for lightweight messaging
- **Raspberry Pi 4** as the sensor node (publisher)
- **ADXL345 accelerometer** and internal temperature readings
- **Qt 5 GUI application** for real-time graph plotting and topic-based data filtering
- **Subject-based filtering** support using MQTT wildcards and GUI-based topic selection

Sensor data is published in **JSON** format and accessed by multiple clients, including a logger, a hardware-driven LED indicator, and a responsive Qt visualization application.

## Features
-  **Real time QT Visualization** – Real-time plotting with QCustomPlot
-  **Sensor Integration** – Pitch, roll, and CPU temperature data via ADXL345 and RPi
-  **JSON Payload Parsing** – Qt GUI extracts and displays specific fields
-  **Subject-Based Filtering** – Supports MQTT wildcards and GUI topic selection
-  **Dynamic Axis Scaling** – Auto-resizes based on data field selected
-  **GPIO Control** – Subscriber toggles LED when pitch exceeds 20°
-  **Data Logging** – Logger (subscriber application) logs pitch/roll/temp to file
-  **MQTT Last Will** – Last Will and Testament message implemented for broker abnormal disconnects

## Architecture
```
Raspberry Pi (Publisher)
├── Reads ADXL345 + CPU Temp
├── Publishes to: 
│   ├── een1071/pitch
│   ├── een1071/test
│   └── een1071/LastWill
└── Uses Paho MQTT C API

MQTT Broker (Debian VM @ 192.168.1.124)
└── Handles all topic routing

Qt GUI Client (Debian VM)
├── Connects to broker
├── Displays pitch/roll/temp vs. time
└── Topic + Field dropdown menus

Additional Subscribers:
├── Logger: Appends sensor data to text file
└── LED: Activates LED if pitch > 20
```

## Hardware & Wiring
### ADXL345 to Raspberry Pi GPIO
| ADXL345 Pin | Function     | Pi GPIO Pin         |
|-------------|--------------|---------------------|
| VCC         | Power        | Pin 1 (3.3 V)        |
| GND         | Ground       | Pin 6 (GND)          |
| SDA         | I²C Data     | Pin 3 (GPIO 2)        |
| SCL         | I²C Clock    | Pin 5 (GPIO 3)        |
| CS          | Chip Select  | Not connected (I²C mode) |
| SDO         | I²C Address | Optional pull-down    |

All devices share the same broker. **GPIO17** is used on the subscriber for LED control via **libgpiod**.


## Build & Run
### Raspberry Pi (Publisher)
```bash
sudo apt update
sudo apt install libgpiod-dev build-essential

g++ publisher.cpp ADXL345.cpp I2CDevice.cpp -o publisher -lpaho-mqtt3c
./publisher
```

### Logger / LED Subscriber
```bash
g++ subscriberLED.cpp -o subscriberled -lpaho-mqtt3c -lgpiod
g++ subscriberLog.cpp -o subscriberled -lpaho-mqtt3c

./subscriberled & ./subscriberlog &
```

### Qt GUI
Open `qt.pro` in **Qt Creator**.  
Make sure the kit is set to **Qt 5.15** and build the project.

You’ll need to install dependencies:
```bash
sudo apt install qtbase5-dev qt5-qmake libqt5charts5-dev
```

## Live Demo
The GUI allows the user to:
- Select topic (e.g. `een1071/test`, `een1071/pitch`, `een1071/#`)
- Select field (`pitch`, `roll`, `tempC`)
- Watch data plotted dynamically with autoscaling y-axis
- Monitor raw JSON payload in real time

📷 See `demo.mp4` or the `video/` folder for a full walkthrough.

## Sample Output
### JSON
```json
{"d":{"pitch":14.73,"roll":-0.04,"tempC":52.10,"time":"2025-04-14 17:02:53"}}
```

### Console:
```java
Parsed pitch = 14.73
Parsed tempC = 52.10
```

## Results & Observations
- MQTT messaging operates seamlessly between Pi and VM broker.
- Subject-based filtering is effective using wildcards
- GUI plotting is responsive, accurate, and supports dynamic scaling.
- The LED subscriber consistently responds to pitch thresholds in real time.
- Logging subscriber captures all expected data fields for review.

## Future Improvements
- Add persistent topic subscriptions between sessions
- Implement per-topic graph coloring
- Export plotted data to CSV from Qt
- Add sliding window X axis

## License
2025 Niall Tóibín 

Built using Qt 5.15, Paho MQTT, and standard C++17 libraries.



