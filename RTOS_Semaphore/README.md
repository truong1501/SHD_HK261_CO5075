# Bài 4 – Semaphore trên YoloUNO (FreeRTOS)

Nút **BOOT (GPIO0)** là nguồn sự kiện, LED **D13 (GPIO48)** là đầu ra.

| Env | File | Cơ chế |
|---|---|---|
| `mutex` *(mặc định)* | `src/main_semaphore_mutex.cpp` | Binary semaphore báo hiệu nút → LED, mutex bảo vệ Serial dùng chung |
| `binary` | `src/main_semaphore_binary.cpp` | Binary semaphore: mỗi lần nhấn LED nháy 3 lần |
| `counting` | `src/main_semaphore_count.cpp` | Counting semaphore: các lần nhấn được xếp hàng (tối đa 5) |

## Kiến trúc (env `mutex`)

| Task | Ưu tiên | Vai trò |
|---|---|---|
| MonitorButton | 10 | Quét nút mỗi 10 ms, chống dội 50 ms, `xSemaphoreGive` |
| BlinkLED | 5 | Block trên `xSemaphoreTake`, nháy LED 3 lần |
| Heartbeat | 1 | Ghi log định kỳ qua cùng mutex Serial |

```bash
pio run -e mutex -t upload
pio device monitor
```

```
[   5120 ms][BlinkLED     ] Waiting to TAKE binary semaphore...
[   7480 ms][MonitorButton] GIVE binary semaphore -> OK
[   7480 ms][BlinkLED     ] TAKE OK -> blink LED 3 times
[   8682 ms][BlinkLED     ] Blink done
```

Mutex của FreeRTOS có **kế thừa độ ưu tiên** (priority inheritance), hạn chế hiện tượng đảo ưu tiên
khi các task có độ ưu tiên khác nhau cùng dùng Serial.
