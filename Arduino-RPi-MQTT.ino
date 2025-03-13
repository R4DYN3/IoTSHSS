#include <PubSubClient.h>
#include <WiFi.h>

#define PIR_SENSOR_PIN 2
#define MAGNETIC_SENSOR_PIN 3

const char* ssid = "xxxxxxxxx";
const char* password = "xxxxxxxxx";
const char* mqtt_server = "xxx.xxx.x.x";

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  pinMode(PIR_SENSOR_PIN, INPUT);
  pinMode(MAGNETIC_SENSOR_PIN, INPUT_PULLUP);

  // Set static IP address (replace with an unused IP in your network range)
  IPAddress local_ip(192, 168, 0, 100);  // Example IP address
  IPAddress gateway(xxx, xxx, x, x);     // Gateway (usually your router IP)
  IPAddress subnet(255, 255, 255, 0);    // Subnet mask
  IPAddress dns(8, 8, 8, 8);  // Google DNS

  WiFi.config(local_ip, dns, gateway, subnet);
  
  //Connect to Wi-Fi
  Serial.print("Connecting to Wi-Fi...");
  WiFi.begin(ssid, password);
  Serial.println("Attempting to connect to Wi-Fi...");

  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 20) {
    delay(500);
    Serial.print(".");
    retries++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected to Wi-Fi!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nFailed to connect to Wi-Fi.");
    Serial.println("Status: " + String(WiFi.status()));
  }

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
      client.subscribe("test/topic");  // Subscribe to a topic for debugging
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

  delay(500);
}
