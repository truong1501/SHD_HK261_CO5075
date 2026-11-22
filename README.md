# SHD_HK261_CO5075 – Bài thực hành Internet of Things (YoloUNO / ESP32-S3)

Mỗi thư mục là code của một bài trong báo cáo thực hành. Các bài 2–5 là **project PlatformIO
độc lập**: mở đúng thư mục đó trong VS Code (File → Open Folder) để PlatformIO nhận `platformio.ini`.

| Bài | Thư mục | Nội dung | Các env |
|---|---|---|---|
| 1 | [`CoreIOT/`](CoreIOT) | Python gửi telemetry/attribute lên CoreIOT qua MQTT | (Python) |
| 2 | [`LedBlinky/`](LedBlinky) | LED GPIO48 nhấp nháy 1 s, in trạng thái ra Serial | `led` |
| 3 | [`DHT22_I2C/`](DHT22_I2C) | Quét bus I2C, đọc cảm biến **DHT20** (0x38) | `scan`, `basic`, `queue` |
| 4 | [`RTOS_Semaphore/`](RTOS_Semaphore) | FreeRTOS binary / counting semaphore và mutex | `binary`, `counting`, `mutex` |
| 5 | [`Webserver/`](Webserver) | Webserver AP/STA: hiển thị DHT20, bật/tắt 2 LED | `yolo_uno` |

## Nguồn gốc từng file

| Bài | File | Nguồn | Ghi chú |
|---|---|---|---|
| 1 | `send_telemetry.py` | viết mới cho báo cáo | paho-mqtt 2.x |
| 1 | `first.py` | code gốc | access token đã thay bằng `ACCESS_TOKEN` |
| 2 | `src/main.cpp` | viết mới cho báo cáo | task FreeRTOS + `vTaskDelayUntil` |
| 2 | `reference/main_template_led_neo.cpp` | project mẫu YoloUNO_PlatformIO | chỉ tham khảo, không build |
| 3 | `src/main_i2c_scan.cpp` | viết mới cho báo cáo | quét 0x08–0x77, kiểm tra mã lỗi `read()` |
| 3 | `src/main_dht20_basic.cpp` | code gốc | 2 task: LED + in DHT20 mỗi 5 s |
| 3 | `src/main_dht20_queue.cpp` | code gốc | task đọc → queue → task in |
| 4 | `src/main_semaphore_binary.cpp` | code gốc | nút BOOT → LED nháy 3 lần |
| 4 | `src/main_semaphore_count.cpp` | code gốc | xếp hàng tối đa 5 lần nhấn |
| 4 | `src/main_semaphore_mutex.cpp` | viết mới cho báo cáo | binary + mutex bảo vệ Serial, log TAKE/GIVE |
| 5 | `src/`, `include/`, `README.md` | code gốc | `temp_humi_monitor.cpp` đã sửa: lỗi đọc → `NAN`, ghi biến trong `sensor_mux` |

## Một project, nhiều chương trình: chọn bằng `env`

Bài 3 và 4 có nhiều file `main_*.cpp`, mỗi file có `setup()`/`loop()` riêng, nên không thể build
cùng lúc (lỗi *multiple definition of setup()*). `platformio.ini` khai báo nhiều env, mỗi env
chỉ build một file:

```ini
[env]                     ; cấu hình chung, mọi env đều kế thừa
board = yolo_uno
...
[env:binary]
build_src_filter = -<*> +<main_semaphore_binary.cpp>
```

```sh
pio run -e binary -t upload   # build + nạp env "binary"
pio device monitor            # xem Serial (115200)
```

Trong VS Code: chọn env ở thanh trạng thái phía dưới rồi bấm Upload.

## Phần cứng

- Board YoloUNO (ESP32-S3, 8 MB flash), định nghĩa trong `boards/yolo_uno.json` của từng project.
- LED: GPIO48 (Bài 5 thêm GPIO41). NeoPixel: GPIO45. Nút BOOT: GPIO0.
- I2C: SDA = GPIO11, SCL = GPIO12; DHT20 ở địa chỉ 0x38 (thư viện `robtillaart/DHT20`).

## Bảo mật

Không commit access token CoreIOT hay mật khẩu Wi-Fi router. Script Python đọc token từ biến
`ACCESS_TOKEN` trong file; hãy sửa cục bộ và không commit thay đổi đó.
