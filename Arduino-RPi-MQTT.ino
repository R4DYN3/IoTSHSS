#include <PubSubClient.h>  // For MQTT connections
#include <WiFi.h>          // For connecting to WiFi

#define PIR_SENSOR_PIN 2
#define MAGNETIC_SENSOR_PIN 3

const char* ssid = "GS21U";           // Wifi Network Name
const char* password = "konnect123";  // Wifi Password
const char* mqtt_server = "192.168.1.8";  // Raspberry Pi's IPv4 Address

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  pinMode(PIR_SENSOR_PIN, INPUT);
  pinMode(MAGNETIC_SENSOR_PIN, INPUT_PULLUP);

  // Assign static IP address
  IPAddress local_ip(192, 168, 0, 100);  // Example IP address to assign to Arduino
  IPAddress gateway(192, 168, 1, 1);     // Gateway - Find by ipconfig in terminal
  IPAddress subnet(255, 255, 255, 0);    // Subnet mask
  IPAddress dns(8, 8, 8, 8);             // Google DNS

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
  client.setServer(mqtt_server, 1883); // Only set the server once!

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

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Sensor read from Keyestudio Board would go here

  // Example sensor read code vvv
  bool motionDetected = digitalRead(PIR_SENSOR_PIN);
  bool doorOpened = digitalRead(MAGNETIC_SENSOR_PIN) == LOW;

  if (motionDetected) {
    Serial.println("MOTION_DETECTED");
    client.publish("home/security", "MOTION_DETECTED");
  }

  if (doorOpened) {
    Serial.println("DOOR_OPENED");
    client.publish("home/security", "DOOR_OPENED");
  }

  // Firebase Upload code here

  delay(500);
}
