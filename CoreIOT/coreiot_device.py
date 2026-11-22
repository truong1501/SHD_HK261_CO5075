"""Simulated CoreIOT device: publishes telemetry and handles dashboard RPC.

Authentication: MQTT Basic, credentials loaded from .env (see .env.example).
"""
import json
import os
import sys
import time
from pathlib import Path

import paho.mqtt.client as mqtt
from dotenv import load_dotenv

load_dotenv(Path(__file__).with_name(".env"))

BROKER_ADDRESS = "app.coreiot.io"
PORT = 1883

CLIENT_ID = os.environ.get("COREIOT_CLIENT_ID")
USERNAME = os.environ.get("COREIOT_USERNAME")
PASSWORD = os.environ.get("COREIOT_PASSWORD")
if not (CLIENT_ID and USERNAME and PASSWORD):
    sys.exit("Missing COREIOT_CLIENT_ID / COREIOT_USERNAME / COREIOT_PASSWORD in .env")

TOPIC_TELEMETRY = "v1/devices/me/telemetry"
TOPIC_ATTRIBUTES = "v1/devices/me/attributes"
TOPIC_RPC_REQUEST = "v1/devices/me/rpc/request/+"


def connected(client, userdata, flags, reason_code, properties):
    if reason_code.is_failure:
        print("Connection failed:", reason_code)
        return
    print("Connected successfully!!")
    # Subscribing here restores the subscription after every reconnect
    client.subscribe(TOPIC_RPC_REQUEST, qos=1)


def subscribed(client, userdata, mid, reason_codes, properties):
    print("Subscribed...", [str(rc) for rc in reason_codes])


def recv_message(client, userdata, message):
    payload = message.payload.decode("utf-8")
    print("Received:", message.topic, payload)

    try:
        request = json.loads(payload)
    except json.JSONDecodeError:
        print("Ignored: payload is not JSON")
        return

    # Dashboard switch widget sends {"method": "setValue", "params": true|false}
    if request.get("method") == "setValue":
        value = request.get("params")
        client.publish(TOPIC_ATTRIBUTES, json.dumps({"value": value}), qos=1)

        # Two-way RPC: the widget times out unless a response is published
        request_id = message.topic.rsplit("/", 1)[-1]
        client.publish(f"v1/devices/me/rpc/response/{request_id}",
                       json.dumps({"value": value}), qos=1)
        print("setValue ->", value)


client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2, client_id=CLIENT_ID)
client.username_pw_set(USERNAME, PASSWORD)
client.on_connect = connected
client.on_subscribe = subscribed
client.on_message = recv_message

client.connect(BROKER_ADDRESS, PORT, keepalive=60)
client.loop_start()

# Publish telemetry every 5 s
temp = 30
humi = 50
light_intensity = 100
try:
    while True:
        collect_data = {"temperature": temp, "humidity": humi, "light": light_intensity}
        client.publish(TOPIC_TELEMETRY, json.dumps(collect_data), qos=1)
        print("Published:", collect_data)
        temp += 1
        humi += 1
        light_intensity += 1
        time.sleep(5)
except KeyboardInterrupt:
    print("Stopping...")
finally:
    client.loop_stop()
    client.disconnect()
