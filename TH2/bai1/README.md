# TH2 - Bài 1: delay, millis và timer phần cứng

Code dành cho **ESP32-S3**. Mỗi ví dụ là **một file `.cpp` tự chứa**;
không cần các header riêng của project. `platformio.ini` chọn đúng một file
để build/nạp mỗi lần, tránh trùng `setup()` và `loop()`.

## Đấu nối

| Linh kiện | ESP32-S3 |
|---|---|
| LED rời | GPIO2 → điện trở 220–330 Ω → anode LED; cathode → GND |
| Biến trở | Hai chân ngoài → 3V3/GND; chân giữa → GPIO1 |
| Nút GPIO4 | Không dùng ở Bài 1 |

Chỉ dùng 3,3 V cho biến trở/ADC. LED RGB tích hợp trên board không phải LED
rời GPIO2. Đề dùng GPIO34 cho ESP32 thường; trên S3, bài này đổi sang
GPIO1 (ADC1_CH0). Vì đổi chân ADC, ghi rõ thay đổi so với sơ đồ TH1.

## Chạy từ thư mục bai1

```powershell
pio run -e delay -t upload
pio device monitor -b 115200
```

Đóng monitor bằng Ctrl+C trước khi nạp ví dụ tiếp theo. Nếu có nhiều cổng,
thêm `--upload-port COM5` khi upload hoặc `-p COM5` khi monitor, thay COM5
bằng cổng từ `pio device list`.

| Env | File | Công việc |
|---|---|---|
| `delay` | `src/vd1_dung_delay.cpp` | Các delay nối tiếp: 500 + 500 + 2000 + 5000 ms |
| `millis` | `src/vd2_non_blocking.cpp` | LED 500 ms, ADC 2000 ms, heartbeat 5000 ms |
| `hardware` | `src/vd3_timer_phan_cung.cpp` | LED và thống kê trong ISR; ADC/heartbeat trong loop |
| `delay_busy` | File delay | Có tác vụ giữ loop bận 1200 ms |
| `millis_busy` | File millis | Có tác vụ giữ loop bận 1200 ms |
| `hardware_busy` | File hardware | Có tác vụ giữ loop bận 1200 ms |

Đổi tên env trong lệnh upload để chạy từng bản. Theo dõi mỗi bản khoảng
60 giây, xoay biến trở, lưu log và chụp kết quả. Chế độ busy không khóa ngắt;
đây là tình huống so sánh loop bị bận với ISR timer vẫn được phục vụ.

## Đọc kết quả

- `led`: trạng thái LED; `bienTro`: ADC thô 12 bit; `heartbeat`: bộ đếm.
- `led_dt_us`, `adc_dt_us`, `heart_dt_us`: khoảng giữa các sự kiện, µs.
- `*_error_us`: khoảng thực tế trừ chu kỳ đặt.
- `*_mae_us`: trung bình trị tuyệt đối sai lệch từ khi reset.
- Dòng `# ... n=... min_us=... max_us=...`: số khoảng đo và cực trị.

Sự kiện đầu chỉ tạo mốc, chưa có khoảng để thống kê. MAE bao gồm sai nhịp
có hệ thống; không đồng nhất MAE với độ dao động jitter. Xem thêm min/max.

Với hardware, timestamp lấy trong ISR. Khi loop bận, Serial chỉ báo snapshot
mới nhất; thống kê vẫn tích lũy mỗi sự kiện ISR. Không đo jitter từ thời gian
máy tính nhận dòng Serial. Timer phần cứng vẫn có thể trễ bởi ưu tiên ngắt
hoặc đoạn khóa ngắt; không đảm bảo jitter bằng 0.

## Hiện tượng dự kiến, chưa phải số đo thật

- `delay`: một vòng khoảng 8 s cộng chi phí thực thi. LED sáng khoảng 0,5 s,
  tắt khoảng 7,5 s; ADC/heartbeat lặp khoảng 8 s. Khoảng có thể đều nhưng sai
  nhịp mục tiêu; không nên kết luận mọi delay đều tạo khoảng ngẫu nhiên.
- `millis`: mỗi tác vụ gần đúng chu kỳ riêng khi loop quay lại đủ nhanh.
  Lịch giữ pha, bỏ các mốc đã lỡ thay vì chạy dồn nhiều sự kiện bù.
- `millis_busy`: sự kiện trong lúc bận bị xử lý muộn/bỏ nhịp.
- `hardware_busy`: LED ISR bám nhịp tốt hơn khi ngắt vẫn được phục vụ;
  ADC, heartbeat và việc in Serial trong loop vẫn có thể trễ.

## Thuyết minh để nộp

| Env | LED min/max (µs) | LED MAE (µs) | ADC/heartbeat | Nhận xét |
|---|---|---|---|---|
| delay | Chưa đo | Chưa đo | Chưa đo | |
| millis | Chưa đo | Chưa đo | Chưa đo | |
| hardware | Chưa đo | Chưa đo | Chưa đo | |
| delay_busy | Chưa đo | Chưa đo | Chưa đo | |
| millis_busy | Chưa đo | Chưa đo | Chưa đo | |
| hardware_busy | Chưa đo | Chưa đo | Chưa đo | |

Thêm ảnh/log đo vào `results/`. Giải thích vì sao tổng delay ảnh hưởng toàn
loop, vì sao millis cần loop đáp ứng kịp và vì sao timer ISR hoạt động được
trong tình huống loop bận. Một chu kỳ sáng-tắt đủ của LED bản millis là 1 s,
mỗi lần đổi trạng thái cách nhau 0,5 s.

Đã có công cụ `plot_serial_th2_bai1.py` từ bản trước: tùy chọn đọc log đã lưu,
không phải thành phần cần để firmware chạy. Các `.h`/test cũ vẫn được giữ để
bảo toàn file sẵn có; các `.cpp` hiện tại không phụ thuộc chúng.

## Cấu hình và kiểm tra

PlatformIO Espressif32 6.6.0 / Arduino-ESP32 2.0.14. Bản hardware dùng
`hw_timer_t` theo demo, không dùng API `esp_timer`. API timer Arduino 3.x
khác; không tự nâng core rồi giữ nguyên lời gọi cũ.

Cấu hình cơ sở S3 DevKit: flash DIO 4 MB, không cần PSRAM. Mặc định Serial
qua cổng USB-UART. Nếu chỉ dùng USB native, thêm
`-DARDUINO_USB_CDC_ON_BOOT=1` vào `build_flags` chung rồi build lại; cấu hình
native này chưa được kiểm tra riêng. Cần đối chiếu sơ đồ nếu board S3 tích hợp
LCD/camera hoặc không đưa GPIO1/GPIO2 ra ngoài.

Sáu cấu hình Bài 1 đã biên dịch thành công cho ESP32-S3.
Chưa đo phần cứng, chưa có ảnh thực nghiệm và chưa có xác nhận trải nghiệm.
