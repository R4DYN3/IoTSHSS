#include <PubSubClient.h> // For MQTT connections
#include <WiFi.h> // For connecting to Wifi
#include <SoftwareSerial.h>

SoftwareSerial softSerial(2, 3);  
HardwareSerial& btSerial = Serial1;

#pragma pack(1)
struct SensorData {
    uint32_t gas;
    uint32_t light;
    uint32_t water;
    uint32_t soil;
    uint32_t PIR_motion;
    uint16_t warnings;
};
#pragma pop

void receiveData(SensorData* data) {
    static bool receiving = false;
    static uint8_t buffer[sizeof(SensorData)];
    static uint8_t index = 0;

    while (softSerial.available()) {
        uint8_t byte = softSerial.read();

        if (!receiving) {
            if (byte == 0xAA) {  
                Serial.println("Start byte detected.");
                receiving = true;
                index = 0;
            }
            continue;
        }

        // Store bytes in buffer
        if (index < sizeof(SensorData)) {
            buffer[index++] = byte;
        } 
        else if (index == sizeof(SensorData)) {
            if (byte == 0x55) {  // End marker
                Serial.println("End marker detected.");
                // Copy struct from buffer
                memcpy(data, buffer, sizeof(SensorData));
                // Print values [debugging]:
                Serial.print("Gas: "); Serial.println(data->gas);
                Serial.print("Light: "); Serial.println(data->light);
                Serial.print("Water: "); Serial.println(data->water);
                Serial.print("Soil: "); Serial.println(data->soil);
                Serial.print("PIR Motion: "); Serial.println(data->PIR_motion);
                Serial.print("Warnings: "); Serial.println(data->warnings, BIN); 

                if (data->warnings & (1 << 13)) Serial.println("Warning: Gas Danger");
                if (data->warnings & (1 << 12)) Serial.println("Warning: Low Light");
                if (data->warnings & (1 << 11)) Serial.println("Warning: High Water Level");
                if (data->warnings & (1 << 10)) Serial.println("Warning: Dry Soil (Hydropenia)");
                
                Serial.println("----------------------");
            } else {
                Serial.println("Error: End marker missing. Data corrupted.");
            }

            // Reset for next message
            receiving = false;
            index = 0;
            continue;
        } else {
            // Buffer overflow prevention
            Serial.println("Error: Buffer overflow. Resetting.");
            receiving = false;
            index = 0;
            continue;
        }
    }
}


const char* ssid = "GS21U";          // Wifi Network Name
const char* password = "konnect123";      // Wifi Password
const char* mqtt_server = "192.168.1.8"; // Raspberry Pi's IPv4 Address

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(9600);
  softSerial.begin(9600); 
  btSerial.begin(9600);

  // Assign static IP address
  IPAddress local_ip(192, 168, 1, 115);  // Example IP address to assign to Arduino, note needs same first three sects, eg 192.168.1.xxx
  IPAddress gateway(192, 168, 1, 1);     // Gateway - Find by ipconfig in terminal
  IPAddress subnet(255, 255, 255, 0);    // Subnet mask
  IPAddress dns(8, 8, 8, 8);  // Google DNS

  WiFi.config(local_ip, dns, gateway, subnet);
  
  // Connect to Wi-Fi
  Serial.print("Connecting to Wi-Fi...");
  WiFi.begin(ssid, password);
  Serial.println("Attempting to connect to Wi-Fi...");

  // Stop connection requests after 20 tries
  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 20) {
    delay(500);
    Serial.print(".");
    retries++;
  }

  // Print Static IP Address after successful connection, otherwise print status error
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected to Wi-Fi!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nFailed to connect to Wi-Fi.");
    Serial.println("Status: " + String(WiFi.status()));
  }

  // Connect to Raspberry Pi via MQTT
  Serial.println("Attempting to connect to MQTT broker...");
  client.setServer(mqtt_server, 1883);

  client.setServer(mqtt_server, 1883);
  while (!client.connected()) {
    if (client.connect("Arduino_Client")) {
      Serial.println("Connected to MQTT broker");
    } else {
      delay(500);
      Serial.print(".");
    }
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32_Client")) {
      Serial.println("Connected!");
      client.subscribe("home/security");  // Subscribe to topic for debugging
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" Retrying in 5 seconds...");
      delay(5000);
    }
  }
}

void mqtt_publish(SensorData* data) {
    client.publish("home/security", (uint8_t*)data, sizeof(SensorData));
    Serial.println("Sensor data published to MQTT");
    delay(2000);
}


void loop() {
  while (btSerial.available())
  {
    char c = btSerial.read();
    softSerial.write(c);
    Serial.print("BT → Uno: "); Serial.println(c);
  }
  if (!client.connected()) {
     reconnect();
  }
  client.loop();

  // Initialise struct to hold sensor data in: 
  SensorData data;
  // call receive data func 
  receiveData(&data);
  mqtt_publish(&data);
  // Firebase Upload code here
  delay(500);
}

