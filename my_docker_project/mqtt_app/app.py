import paho.mqtt.client as mqtt
from datetime import datetime
import urllib3
import json
import sqlite3
from pymongo import MongoClient

http = urllib3.PoolManager()

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, reason_code, properties):
    print(f"Connected with result code {reason_code}")
    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe("ict720/supachai/esp32/#")

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    print(msg.topic+" "+str(msg.payload))
    # heartbeat message
    if msg.topic.split('/')[-1] == "beat":
        data = json.loads(msg.payload.decode())
        print(f"Device: {data['mac']}")
        return
    if msg.topic.split('/')[-1] == "data":
        data = json.loads(msg.payload.decode())
        print(f"Device: {data['name']}")
        # insert to SQLite
        station = msg.topic.split('/')[2]
        device = data['name']
        rssi = data['rssi']
        c.execute("INSERT INTO taist (station, device, rssi) VALUES (?, ?, ?)", 
                  (station, device, rssi))
        print("Inserted to SQLite")
        conn.commit()
        # insert to MongoDB
        db = mongo_client.taist_db
        db_col = db.ble_logs
        db_col.insert_one({"timestamp": datetime.now(), 
                           "station": station, 
                           "device": device, 
                           "rssi": rssi})
        print("Inserted to MongoDB")

    # insert to Firebase
    # payload = {"timestamp": datetime.now().isoformat(), "message": msg.payload.decode()}
    # encoded_payload = json.dumps(payload).encode('utf-8')
    # print(encoded_payload)
    # http.request('POST', 'https://taist-supachai-2025-default-rtdb.firebaseio.com/taist.json', 
    #              body=encoded_payload, headers={'Content-Type': 'application/json'})

# init SQLite
conn = sqlite3.connect('taist.db')
c = conn.cursor()
c.execute('''CREATE TABLE IF NOT EXISTS taist (
          _id INTEGER PRIMARY KEY AUTOINCREMENT,
          timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
          station TEXT,
          device TEXT,
          rssi INTEGER
          )''')
conn.commit()

# init MongoDB
mongo_client = MongoClient('my_docker_project-mongo-1', 27017,
                           username='root', password='example')

# init MQTT
mqttc = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
mqttc.on_connect = on_connect
mqttc.on_message = on_message

mqttc.connect("mosquitto", 1883, 60)


# Blocking call that processes network traffic, dispatches callbacks and
# handles reconnecting.
# Other loop*() functions are available that give a threaded interface and a
# manual interface.
mqttc.loop_forever()