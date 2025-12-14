import mysql.connector
from mysql.connector import Error
import paho.mqtt.client as mqtt
import json

# Database configuration
try: 
    conn = mysql.connector.connect(
        host = "localhost",
        port = 3306,
        user = "root",
        password = "master2024",
        database = "Master"
    )
    if conn.is_connected():
        print("Connected to database")
        
except Error as e:
    print(f"Error connecting to database: {e}")

# MQTT configuration
mqtt_broker = "192.168.0.105"
mqtt_port = 1883
mqtt_topic = "home/+"

# Callback when the client receives a connection response from the broker
def on_connect(client, userdata, flags, rc):
    print(f"Connected with result code {rc}")
    client.subscribe(mqtt_topic)

# Callback when a PUBLISH message is received from the broker
def on_message(client, userdata, msg):
    print(f"Message received on topic {msg.topic}: {msg.payload.decode()}")

    try:

        received_message = json.loads(msg.payload.decode())
        temperature = received_message.get("temp")
        humidity = received_message.get("hum")
        voc = received_message.get("voc")
        nox = received_message.get("nox")
        co2 = received_message.get("co2")
        sps30 = received_message.get("sps30")
        s = received_message.get("sound")
        vibration = received_message.get("vib")
        
        
        # Function to save data to the database
        cursor = conn.cursor()
        query = """INSERT INTO sensors (Topic, Temperature, Humidity, VocIndex, NoxIndex, CO2, SPS30, Sound, Vibration)
                   VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s)"""
        cursor.execute(query, (msg.topic, temperature, humidity, voc, nox, co2, sps30, s, vibration))
        conn.commit()
        cursor.close()
        print("Data saved to database.")
        
    except Error as e:
        if conn:
            conn.rollback()
        print(f"Error processing message or saving to database: {e}")

# Set up MQTT client
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

# Connect to the MQTT broker
client.connect(mqtt_broker, mqtt_port, 60)

# Start the MQTT client loop
client.loop_forever()

# Close the connection when done (not reachable as it loops forever)
# conn.close()

