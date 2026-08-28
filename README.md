# IoT based Multi-sensor monitoring module.
This project represents designing and implementing a multi-sensor module which includes NodeMCU(ESP8266) and various air quality sensors to communicate with a central data server for data storage and analysis. The NodeMCU (ESP8266) microcontroller integrates with various sensors to monitor environmental parameters like carbon dioxide, nitrogen dioxide, Volatile Organic Matter, Particulate Matter, Temperature, and Humidity.
The Communication is established via the MQTT protocol, ensuring efficient and reliable data transmission to a central Raspberry Pi server using the Internet. The collected data is stored in a MariaDB database and values are visualized through Grafana for real-time monitoring. 
The system’s design emphasizes scalability, cost-effectiveness, and ease of deployment, making it suitable for diverse applications such as smart homes, industrial automation and environmental monitoring. A prototype PCB is designed using Altium ECAD software for robust and durable deployment.

# Hardware
- Sensirion SCD41 CO2 sensor
- Sensirion SPS30 Particulate Matter sensor
- Sensirion SVM41 sensor
- Adafruit LIS3DH Triple Axis Accelerometer
- Grove seeed Loudness sensor
- NodeMCU(ESP8266)
- Raspberry Pi 4 Model B

# Software
- Arduino IDE (for NodeMCU(ESP8266) programming)
- Raspberry Pi OS
- MariaDB
- phpMyAdmin
- Apache2
- Grafana Dashboard
- Altium Designer

