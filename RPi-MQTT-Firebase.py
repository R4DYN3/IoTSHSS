import paho.mqtt.client as mqtt
import struct
import firebase_admin
from firebase_admin import credentials, firestore
import datetime

# Define MQTT broker details
MQTT_BROKER = "192.168.1.8"  # Update this to your broker's IP
MQTT_PORT = 1883
MQTT_TOPIC = "home/security"

# Define the struct format (matches ESP32 struct: 5 uint32 + 1 uint16)
STRUCT_FORMAT = "<5I H"  # 5x uint32_t + 1x uint16_t

cred = credentials.Certificate('/home/alex/Documents/Smart Home Security System/iotshss-firebase-adminsdk-fbsvc-feb96a7009.json')
firebase_admin.initialize_app(cred)
db = firestore.client()

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connected to MQTT broker!")
        client.subscribe(MQTT_TOPIC)
    else:
        print(f" Failed to connect, return code {rc}")

def on_message(client, userdata, msg):
    # Ensure the received message has the correct size
    if len(msg.payload) == struct.calcsize(STRUCT_FORMAT):
        # Unpack the binary message into values
        gas, light, water, soil, pir_motion, warnings = struct.unpack(STRUCT_FORMAT, msg.payload)

        # Print the raw unpacked data
        print(f"Raw Unpacked Data:")
        print(f"   Gas: {gas}, Light: {light}, Water: {water}, Soil: {soil}, PIR: {pir_motion}, Warnings: {warnings}")

        # Print the received sensor data
        print(f"Received Sensor Data:")
        print(f"Gas Level: {gas}")
        print(f"Light Level: {light}")
        print(f"Water Level: {water}")
        print(f"Soil Moisture: {soil}")
        print(f"PIR Motion: {'Detected' if pir_motion else 'None'}")
        print(f"Warnings: {bin(warnings)}")

        # Decode warnings
        warning_messages = []
        if warnings & (1 << 13): warning_messages.append("DANGEROUS GAS LEVELS")
        if warnings & (1 << 12): warning_messages.append("LOW LIGHT LEVELS")
        if warnings & (1 << 11): warning_messages.append("HIGH WATER LEVEL")
        if warnings & (1 << 10): warning_messages.append("DRY SOIL (Hydropenia)")

        # Get the current timestamp for data organization
        timestamp = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")

        # Organize data into Firestore
        sensor_data = {
            "gas_level": gas,
            "light_level": light,
            "water_level": water,
            "soil_moisture": soil,
            "pir_motion": pir_motion,
            "warnings": warning_messages,
            "timestamp": timestamp
        }

        # Create a document ID based on the timestamp to organize data
        doc_ref = db.collection("sensor_data").document(timestamp)
        doc_ref.set(sensor_data)

        print(f"Data sent to Firebase at {timestamp}")

    else:
        print("Received incorrect data size!")

# Create MQTT client
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

# Connect to the MQTT broker and start the client loop
client.connect(MQTT_BROKER, MQTT_PORT, 60)

# Start listening for incoming messages
client.loop_forever()
