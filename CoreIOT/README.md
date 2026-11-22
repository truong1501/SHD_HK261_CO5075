# Bài 1 – CoreIOT: thiết bị giả lập qua MQTT

Script Python đóng vai một thiết bị IoT trên nền tảng CoreIOT (ThingsBoard):

- Gửi telemetry `temperature`, `humidity`, `light` mỗi 5 giây lên `v1/devices/me/telemetry`.
- Nhận lệnh RPC `setValue` từ widget switch trên dashboard, ghi vào client attribute `value`
  và phản hồi qua `v1/devices/me/rpc/response/{id}`.
- Xác thực MQTT Basic (client ID, username, password), thư viện paho-mqtt 2.x.

## Cài đặt và chạy

```bash
python3 -m venv .venv && source .venv/bin/activate
pip install -r requirements.txt
cp .env.example .env        # điền thông tin MQTT Basic của device
python coreiot_device.py
```

Thông tin đăng nhập lấy tại *Device → Manage credentials → MQTT Basic* và chỉ nằm trong `.env`
(đã được `.gitignore`).

## Kết quả

```
Connected successfully!!
Subscribed... ['Granted QoS 1']
Published: {'temperature': 30, 'humidity': 50, 'light': 100}
```

Kiểm tra trên CoreIOT: tab **Latest telemetry** của device và **Attributes → Client attributes**
(key `value` sau khi bật/tắt switch trên dashboard).

| File | Nội dung |
|---|---|
| `coreiot_device.py` | Telemetry + xử lý RPC |
| `.env.example` | Mẫu cấu hình đăng nhập |
| `requirements.txt` | `paho-mqtt>=2.0`, `python-dotenv` |
