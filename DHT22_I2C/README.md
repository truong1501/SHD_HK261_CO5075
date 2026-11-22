# Bài 3 – Kết nối I2C với cảm biến DHT20

DHT20 nối bus I2C: **SDA = GPIO11, SCL = GPIO12**, địa chỉ **0x38**, tần số 100 kHz.
Thư viện `robtillaart/DHT20` được cài qua `lib_deps`.

## Các chương trình (chọn bằng env)

| Env | File | Mô tả |
|---|---|---|
| `scan` *(mặc định)* | `src/main_i2c_scan.cpp` | Quét bus I2C 0x08–0x77, đọc DHT20 mỗi 5 s, kiểm tra mã lỗi |
| `basic` | `src/main_dht20_basic.cpp` | Task LED + task đọc DHT20 |
| `queue` | `src/main_dht20_queue.cpp` | Producer/consumer qua FreeRTOS queue |
| `gateway` | `src/main_dht20_gateway.cpp` | In mỗi lần đo thành một dòng JSON để gửi lên CoreIOT |

```bash
pio run -e scan -t upload
pio device monitor          # bấm RST khi Monitor đang mở để xem lại kết quả quét
```

```
Scanning I2C bus...
  Found device at 0x38  <- DHT20
Scan done: 1 device(s)
Temperature: 29.85 C | Humidity: 61.20 %
```

Mã lỗi của `dht20.read()`: `-10` checksum, `-11` mất kết nối, `-14` timeout, `-15` đọc quá sớm (< 1 s).

## Gửi dữ liệu lên CoreIOT qua Serial gateway

Board không cần Wi-Fi: máy tính làm gateway, đọc JSON qua cáp USB và publish lên CoreIOT.

```
DHT20 --I2C--> YoloUNO --USB Serial (JSON)--> serial_gateway.py --MQTT--> CoreIOT
```

```bash
pio run -e gateway -t upload
pip install -r gateway/requirements.txt
python gateway/serial_gateway.py            # tự tìm cổng /dev/cu.usbmodem*
```

- Thông tin đăng nhập dùng lại `CoreIOT/.env` của Bài 1.
- `light` là giá trị **giả lập** (300–500 lux) vì phần cứng không có cảm biến ánh sáng.
- Đóng Serial Monitor trước khi chạy gateway (mỗi cổng chỉ một chương trình mở được) và không chạy
  đồng thời với `coreiot_device.py` (cùng client ID).
