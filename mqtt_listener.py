import paho.mqtt.client as mqtt

LOG_FILE = "security_log.txt"

def on_connect(client, userdata, flags, rc):
    print("Connected to MQTT broker")
    client.subscribe("home/security")

def on_message(client, userdata, msg):
    event = msg.payload.decode()
    print(f"Received: {event}")

    # Save event to log file
    with open(LOG_FILE, "a") as log:
        log.write(f"{event}\n")

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect("localhost", 1883, 60)  # Connect to local MQTT broker
client.loop_forever()