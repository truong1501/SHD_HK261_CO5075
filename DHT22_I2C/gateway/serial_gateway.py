"""Serial-to-MQTT gateway: forwards DHT20 readings from the board to CoreIOT.

    YoloUNO --(USB serial, JSON lines)--> this script --(MQTT)--> CoreIOT

The board (env "gateway") prints {"temperature": .., "humidity": ..} every 5 s.
A simulated "light" value is added before publishing. Credentials are read from
gateway/.env, falling back to CoreIOT/.env.

Usage:
    python serial_gateway.py                       # auto-detect the board's port
    python serial_gateway.py /dev/cu.usbmodem1101  # explicit port
"""
import json
import os
import random
import sys
import time
from pathlib import Path

import paho.mqtt.client as mqtt
import serial
from dotenv import load_dotenv
from serial.tools import list_ports

HERE = Path(__file__).resolve().parent
load_dotenv(HERE / ".env")
load_dotenv(HERE.parent.parent / "CoreIOT" / ".env")

BROKER_ADDRESS = os.environ.get("COREIOT_HOST", "app.coreiot.io")
PORT = int(os.environ.get("COREIOT_PORT", "1883"))
CLIENT_ID = os.environ.get("COREIOT_CLIENT_ID")
USERNAME = os.environ.get("COREIOT_USERNAME")
PASSWORD = os.environ.get("COREIOT_PASSWORD")
if not (CLIENT_ID and USERNAME and PASSWORD):
    sys.exit("Missing COREIOT_CLIENT_ID / COREIOT_USERNAME / COREIOT_PASSWORD "
             "(gateway/.env or CoreIOT/.env)")

TOPIC_TELEMETRY = "v1/devices/me/telemetry"
BAUD = 115200


def find_port():
    """Return the board's USB CDC port (/dev/cu.usbmodem* on macOS, /dev/ttyACM* on Linux)."""
    for p in list_ports.comports():
        if "usbmodem" in p.device or "ttyACM" in p.device:
            return p.device
    raise serial.SerialException("board not found (no /dev/cu.usbmodem* port)")


# Simulated light sensor: bounded random walk in the indoor range 300-500 lux.
# If the board ever reports "light" itself, that value is kept as is.
_lux = 400.0


def simulated_light():
    global _lux
    _lux = min(500.0, max(300.0, _lux + random.uniform(-15, 15)))
    return round(_lux)


def on_connect(client, userdata, flags, reason_code, properties):
    if reason_code.is_failure:
        print("MQTT connection failed:", reason_code)
    else:
        print("Connected to CoreIOT")


def read_loop(ser, client):
    while True:
        line = ser.readline().decode("utf-8", errors="replace").strip()
        if not line:
            continue
        if not line.startswith("{"):
            print("[board]", line)
            continue
        try:
            data = json.loads(line)
        except json.JSONDecodeError:
            print("Skipped malformed JSON:", line)
            continue
        data.setdefault("light", simulated_light())
        client.publish(TOPIC_TELEMETRY, json.dumps(data), qos=1)
        print("Published:", data)


def main():
    port = sys.argv[1] if len(sys.argv) > 1 else None

    client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2, client_id=CLIENT_ID)
    client.username_pw_set(USERNAME, PASSWORD)
    client.on_connect = on_connect
    client.connect(BROKER_ADDRESS, PORT, keepalive=60)
    client.loop_start()

    try:
        while True:
            # The USB port disappears on board reset; reopen it so the boot log
            # (including the I2C scan) is captured within the board's 3 s wait.
            try:
                p = port or find_port()
                with serial.Serial(p, BAUD, timeout=10) as ser:
                    print(f"Reading {p} @ {BAUD} baud (press RST to replay the I2C scan, Ctrl+C to stop)")
                    read_loop(ser, client)
            except (serial.SerialException, OSError) as e:
                print("Serial port unavailable, retrying in 1 s (is the Serial Monitor open?):", e)
                time.sleep(1)
    except KeyboardInterrupt:
        print("\nStopping...")
    finally:
        client.loop_stop()
        client.disconnect()


if __name__ == "__main__":
    main()
