# Internet of Things – Bài thực hành trên YoloUNO (ESP32-S3)

Mã nguồn các bài thực hành môn **Internet of Things** (chương trình Thạc sĩ, HCMUT),
học viên **Nguyễn Xuân Trường** – MSHV 2570526.

- Landing page: <https://truong1501.github.io/SHD_HK261_CO5075/>
- Báo cáo (PDF): [`docs/2570526_NguyenXuanTruong_BaoCaoThucHanh.pdf`](docs/2570526_NguyenXuanTruong_BaoCaoThucHanh.pdf)

| Bài | Thư mục | Nội dung |
|---|---|---|
| 1 | [`CoreIOT`](CoreIOT) | Thiết bị giả lập gửi telemetry và nhận RPC qua MQTT |
| 2 | [`LedBlinky`](LedBlinky) | Task FreeRTOS nháy LED D13 với chu kỳ cố định |
| 3 | [`DHT22_I2C`](DHT22_I2C) | Quét bus I2C, đọc DHT20, Serial gateway lên CoreIOT |
| 4 | [`RTOS_Semaphore`](RTOS_Semaphore) | Binary / counting semaphore và mutex |
| 5 | [`Webserver`](Webserver) | Dashboard web AP/STA, cấu hình Wi-Fi, điều khiển LED, CoreIOT |

## Phần cứng

| Thành phần | Chân |
|---|---|
| Board | YoloUNO – ESP32-S3, 8 MB flash |
| LED đơn | D13 (GPIO48) |
| LED RGB WS2812 | GPIO45 |
| Nút BOOT | GPIO0 |
| DHT20 (I2C, 0x38) | SDA GPIO11, SCL GPIO12 |

## Môi trường

- [PlatformIO](https://platformio.org) (VS Code), framework Arduino trên `espressif32`.
- Mỗi bài là một project độc lập; mở đúng thư mục bài trong VS Code.
- Bài có nhiều chương trình chọn bằng env: `pio run -e <env> -t upload`.
- Bài 1 và gateway Bài 3 dùng Python 3 (`paho-mqtt` 2.x).

Thông tin đăng nhập (`.env`, `secrets.h`) không được đưa lên repo; mỗi bài có file mẫu `*.example`.
