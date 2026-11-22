# send_telemetry.py - Gửi telemetry (và attribute) lên CoreIOT qua MQTT
# Cài đặt: pip install "paho-mqtt>=2.0"
import json, random, time
import paho.mqtt.client as mqtt

BROKER = "app.coreiot.io"          # MQTT broker của CoreIOT
PORT = 1883                        # MQTT không mã hoá (8883 nếu dùng TLS)
ACCESS_TOKEN = "ACCESS_TOKEN"      # access token của device (đã che)
TOPIC_TELEMETRY = "v1/devices/me/telemetry"
TOPIC_ATTRIBUTES = "v1/devices/me/attributes"

def on_connect(client, userdata, flags, reason_code, properties):
    print("Connected, reason code:", reason_code)
    # Attribute (client-side): thông tin tĩnh, chỉ cần gửi khi kết nối
    client.publish(TOPIC_ATTRIBUTES,
                   json.dumps({"firmware": "1.0.0", "location": "Lab IoT - HCMUT"}), qos=1)

# paho-mqtt 2.x bắt buộc khai báo phiên bản API của callback
client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2, client_id="yolo-uno-sim")
client.username_pw_set(ACCESS_TOKEN)   # CoreIOT xác thực device bằng token ở trường username
client.on_connect = on_connect
client.connect(BROKER, PORT, keepalive=60)
client.loop_start()                    # luồng nền xử lý mạng, ping, gửi lại QoS1

try:
    while True:
        # Telemetry: dữ liệu chuỗi thời gian, mỗi lần gửi là một điểm dữ liệu mới
        payload = {"temperature": round(random.uniform(28, 33), 1),
                   "humidity": round(random.uniform(55, 75), 1)}
        info = client.publish(TOPIC_TELEMETRY, json.dumps(payload), qos=1)
        info.wait_for_publish()        # chờ broker xác nhận (PUBACK)
        print("Published:", payload)
        time.sleep(5)
except KeyboardInterrupt:
    client.loop_stop()
    client.disconnect()

# Cách khác (HTTP API) để thử nhanh một bản tin:
# curl -X POST -H "Content-Type: application/json" \
#      -d '{"temperature": 30.5, "humidity": 65}' \
#      https://app.coreiot.io/api/v1/ACCESS_TOKEN/telemetry
