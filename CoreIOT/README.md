# Bài 1: CoreIOT (telemetry + attribute qua MQTT)

| File | Nguồn | Nội dung |
|---|---|---|
| `send_telemetry.py` | mới (bản trong báo cáo) | paho-mqtt 2.x, gửi temperature/humidity mỗi 5 s, gửi attribute khi kết nối |
| `first.py` | bản gốc `Exercise/CoreIOT/first.py` (đã che token) | gửi telemetry + nhận RPC `setValue` từ dashboard |

Chạy: `pip install "paho-mqtt>=2.0"` rồi `python send_telemetry.py`
(thay `ACCESS_TOKEN` bằng token của device, KHÔNG đưa token thật lên GitHub).

Lưu ý: `first.py` viết theo API paho-mqtt 1.x (`mqttclient.Client("IOT_DEVICE_2")`).
Với paho-mqtt 2.x phải thêm `mqttclient.CallbackAPIVersion.VERSION1` làm tham số đầu,
hoặc chuyển callback sang VERSION2 như `send_telemetry.py`.

`platformio.ini` và `boards/` trong thư mục này là khung project ESP32 sẵn có, hiện chưa dùng
(Bài 1 chạy bằng Python). Có thể dùng sau nếu muốn gửi telemetry trực tiếp từ YoloUNO.
