# Bài 2 – LED Blinky trên YoloUNO

Một task FreeRTOS bật/tắt LED đơn trên chân **D13 (GPIO48)** với chu kỳ 2 s (sáng 1 s, tắt 1 s)
và in trạng thái kèm thời điểm ra Serial.

- Dùng `vTaskDelayUntil` để chu kỳ cố định, không bị trôi theo thời gian thực thi.
- `loop()` chỉ nhường CPU; toàn bộ công việc nằm trong task.

## Chạy

```bash
pio run -e led -t upload
pio device monitor
```

```
[    1000 ms] LED ON
[    2000 ms] LED OFF
```

`reference/` chứa project mẫu ban đầu (LED + NeoPixel + DHT20), chỉ để tham khảo, không được build.
