# Bài 5 – Webserver trên YoloUNO

Webserver chạy trực tiếp trên ESP32-S3: giám sát DHT20, điều khiển LED, cấu hình Wi-Fi qua trình duyệt
và gửi dữ liệu lên CoreIOT khi có Internet.

## Tính năng

- **AP / STA**: lần đầu phát Wi-Fi `GoogleBing` (`http://192.168.4.1`). Sau khi cấu hình router,
  thông tin được lưu vào flash (Preferences/NVS) và tự kết nối lại sau khi mất điện.
  Kết nối thất bại quá 10 s hoặc nhấn **BOOT** sẽ quay về AP.
- **Quét Wi-Fi** bất đồng bộ: 6 mạng mạnh nhất, mức sóng, RSSI (dBm), mạng có mật khẩu.
- **Dashboard**: nhiệt độ/độ ẩm cập nhật mỗi 3 s (`fetch('/sensors')`), biểu đồ mini, trạng thái kết nối.
- **Điều khiển**: LED đơn **D13 (GPIO48)** và LED RGB **WS2812 (GPIO45)** với bảng chọn màu.
- **CoreIOT**: ở chế độ STA, task riêng publish `temperature`, `humidity`, `light` (giả lập) mỗi 5 s qua MQTT.

## Kiến trúc

| Task | Ưu tiên | Vai trò |
|---|---|---|
| DHT20 | 2 | Đọc cảm biến mỗi 5 s, ghi biến dùng chung (bảo vệ bằng `portMUX`) |
| Web Server | 2 | Quản lý Wi-Fi AP/STA, xử lý HTTP mỗi 20 ms |
| CoreIOT | 1 | Chờ `xBinarySemaphoreInternet`, publish telemetry qua MQTT |

| Route | Mô tả |
|---|---|
| `GET /` | Dashboard |
| `GET /sensors` | JSON: nhiệt độ, độ ẩm, trạng thái LED, màu, chế độ, IP, RSSI, CoreIOT |
| `GET /toggle?led=1\|2` | Bật/tắt LED |
| `GET /color?hex=RRGGBB` | Đổi màu LED RGB |
| `GET /settings` | Trang cấu hình Wi-Fi |
| `POST /scan`, `GET /scan` | Bắt đầu quét / lấy kết quả |
| `POST /connect` | Kết nối và lưu router |

## Chạy

```bash
cp include/secrets.h.example include/secrets.h   # điền thông tin MQTT Basic của CoreIOT
pio run -t upload
pio device monitor
```

1. Kết nối Wi-Fi `GoogleBing` (mật khẩu trong `src/global.cpp`), mở `http://192.168.4.1`.
2. **Cấu hình Wi-Fi** → chọn router → Kết nối. Serial in `STA IP address` và `CoreIOT: connecting... connected`.
3. Mở dashboard bằng IP STA từ thiết bị cùng mạng.

Giới hạn: HTTP không mã hoá, mật khẩu router gửi bằng POST trong mạng cục bộ.
