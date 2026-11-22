# Bài 1: CoreIOT – thiết bị giả lập qua MQTT

`coreiot_device.py` giả lập một thiết bị IoT kết nối tới CoreIOT (nền ThingsBoard):

- Gửi **telemetry** `temperature`, `humidity`, `light` mỗi 5 giây lên `v1/devices/me/telemetry`.
- Subscribe `v1/devices/me/rpc/request/+` để nhận lệnh từ dashboard. Khi nhận `setValue`, ghi giá trị
  lên **attribute** `value` và trả lời RPC qua `v1/devices/me/rpc/response/{id}`.
- Xác thực **MQTT Basic** (client ID + username + password) và dùng paho-mqtt 2.x.

## Chạy trên máy

```bash
python3 -m venv .venv
source .venv/bin/activate
pip install "paho-mqtt>=2.0"

# Thông tin MQTT Basic của device trên CoreIOT (Device → Manage credentials → MQTT Basic)
export COREIOT_CLIENT_ID="..."
export COREIOT_USERNAME="..."
export COREIOT_PASSWORD="..."

python coreiot_device.py
```

Script đọc thông tin đăng nhập từ biến môi trường, nên file `.py` không chứa mật khẩu và có thể commit an toàn.
Đừng ghi giá trị thật vào README hay commit chúng lên GitHub.

Kết quả mong đợi:

```
Connected successfully!!
Subscribed... ['Granted QoS 1']
Published: {'temperature': 30, 'humidity': 50, 'light': 100}
```

Kiểm tra trên CoreIOT: tab **Latest telemetry** của device, và tab **Attributes → Client attributes**
(key `value` sau khi bấm switch trên dashboard).

`platformio.ini` và `boards/` trong thư mục này là khung project ESP32 có sẵn, hiện chưa dùng.
