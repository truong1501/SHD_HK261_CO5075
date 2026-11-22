# Webserver — YOLO UNO / PlatformIO

Dự án tham khảo cấu trúc `../YoloUNO_PlatformIO`, sử dụng Arduino trên ESP32-S3.

- `boards/yolo_uno.json`: định nghĩa board từ dự án mẫu.
- `src/main.cpp`: khởi tạo task DHT20 và web server.
- `src/temp_humi_monitor.cpp`: đọc DHT20 mỗi 5 giây, cập nhật biến dùng chung.
- `src/mainserver.cpp`: route HTTP, cấu hình Wi-Fi và điều khiển LED.
- `include/web_pages.h`: HTML/CSS tĩnh nằm trong flash, gửi bằng `send_P` (không cấp phát heap).
- `include/logo_png.h`: logo dạng mảng byte, phục vụ ở `/logo.png`; CSS ở `/style.css`.
  Hai file này được trình duyệt cache 1 ngày; khi sửa CSS hãy tăng `?v=` trong HTML.
- `include/wifi_credentials.h`, `src/wifi_credentials.cpp`: lưu và đọc cấu hình router bằng Preferences/NVS.
- `include/global.h`, `src/global.cpp`: biến dùng chung và cấu hình AP.
- `platformio.ini`: cài thư viện `robtillaart/DHT20` bằng `lib_deps`.

## Build và nạp chương trình

Mở thư mục này trong VS Code với PlatformIO IDE, hoặc chạy trong PlatformIO terminal:

```sh
pio run
pio run --target upload
pio device monitor
```

Khi chưa có cấu hình router, kết nối Wi-Fi AP có tên và mật khẩu trong `src/global.cpp`.
Mở địa chỉ AP được in trên Serial Monitor (thường là `http://192.168.4.1`).
Đổi tên và mật khẩu AP trong `src/global.cpp` nếu cần.

## Tự kết nối router sau mất điện

1. Vào trang Cài đặt, chọn Quét Wi-Fi rồi chọn mạng tìm thấy, hoặc nhập SSID trực tiếp
   (ví dụ mạng ẩn). Nhập mật khẩu router, chọn Kết nối.
2. Khi kết nối thành công, Serial in `Router Wi-Fi saved to flash`. Cấu hình được lưu
   bằng Preferences (có sẵn trong Arduino-ESP32), không cần thêm `lib_deps`.
3. Sau reset hoặc mất điện, ESP32 đọc cấu hình đã lưu và thử kết nối router.
   Dùng IP STA in trên Serial để mở dashboard từ thiết bị cùng mạng.
4. Nếu chưa lưu cấu hình, không đọc được flash hoặc kết nối quá 10 giây, ESP32 mở AP.
5. Nhấn BOOT sau khi chương trình khởi động (kể cả lúc đang thử kết nối) để vào AP và đổi router.
   Thao tác này không xóa thông tin đã lưu. Nhập sai thông tin mới không ghi đè cấu hình cũ.

Thông tin router được lưu cùng nhau trong namespace `router-wifi`, key `credentials`.
Chỉ ghi flash sau lần cấu hình mới thành công, không ghi lại ở mỗi lần khởi động.

Kiểm thử trên board: cấu hình router → thấy log lưu flash → ngắt/cấp nguồn → thấy IP STA;
thử đổi router thành công rồi cấp nguồn lại; thử mật khẩu sai để kiểm tra AP sau 10 giây
và cấu hình cũ vẫn được dùng sau reset; thử router tắt và nút BOOT để kiểm tra đường về AP.

## Quét Wi-Fi và nhập mật khẩu

- Quét bất đồng bộ để server tiếp tục phục vụ request. Khi đang ở AP, ESP32 tạm dùng
  AP_STA để quét và giữ trang cài đặt truy cập được; khi xong trở về AP.
- Trang Cài đặt tự quét khi mở; nút "Quét lại" để quét tiếp. Mỗi mạng hiển thị cột sóng
  (4 mức: Rất mạnh ≥ -55, Mạnh ≥ -67, Trung bình ≥ -75 dBm, còn lại Yếu), RSSI (dBm),
  phần trăm chất lượng `2 × (RSSI + 100)` và biểu tượng 🔒 nếu có mật khẩu. ESP32 chỉ trả
  về 6 mạng mạnh nhất (`MAX_SCAN_RESULTS`), mỗi SSID một lần, sắp xếp từ mạnh đến yếu.
- Thời hạn quét là 15 giây (`SCAN_TIMEOUT_MS`). Không dùng thời hạn 6 giây mặc định của
  `scanComplete()`, vì khi đang phát AP việc quét chậm hơn và dễ bị báo lỗi nhầm.
- Chọn mạng mở sẽ khóa ô mật khẩu; nút "Hiện" cho phép xem lại mật khẩu đã gõ.
- Dashboard hiển thị chế độ (AP/STA), IP và RSSI tới router khi ở STA.
- Chờ quét xong trước khi kết nối. Không tìm thấy mạng vẫn có thể nhập SSID thủ công.
- Ô mật khẩu dùng `type="password"`, không điền lại mật khẩu đã lưu. Thông tin router
  gửi bằng POST `/connect` trong body, không nằm trong URL. Mật khẩu không in ra Serial
  hoặc trả về trong kết quả quét. POST không mã hóa: HTTP vẫn có thể được xem bằng
  công cụ phát triển của trình duyệt hoặc khi lưu lượng bị đọc; che ký tự chỉ che trên màn hình.

Kiểm thử trên board: quét ở AP/STA, quét lại, chọn mạng, nhập mật khẩu có ký tự đặc biệt,
kiểm tra POST `/connect` trong Network tab, kiểm tra kết nối và lưu cấu hình sau reset.

DHT20 dùng SDA GPIO11, SCL GPIO12 theo dự án mẫu;
hai đầu ra trên web: LED đơn D13 (GPIO48) và LED RGB NeoPixel (WS2812) GPIO45, điều khiển bằng thư viện Adafruit NeoPixel. Nhấn BOOT để về chế độ AP
khi đang ở chế độ STA. Task nháy LED48 của mẫu được thay bằng task server
để LED48 nhận lệnh từ web.

Luồng dữ liệu: DHT20 → task cảm biến → biến trong `global.cpp` → `/sensors`
→ JavaScript cập nhật nhiệt độ/độ ẩm trên trang mỗi 3 giây. Khi giá trị là `NAN`
(ví dụ trước lần đọc đầu tiên), trang hiển thị `--`.

Build chỉ kiểm tra biên dịch; hoạt động của cảm biến, LED và Wi-Fi cần kiểm tra trên board.
