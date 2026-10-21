# Thí nghiệm trong Khoa học Kỹ thuật 2 — Buổi 1

Bộ bài có ba thư mục project chính (`L1`, `L2`, `L3`), mỗi thư mục có `src/main.cpp` và `platformio.ini`. Bài 1 có hai bản code riêng để so sánh; có thể build qua hai environment trong `L1` hoặc mở trực tiếp `Polling`/`Interrupt`, mỗi mode cũng có `platformio.ini` riêng.

## Cấu trúc

- `L1/` — LED fading; hai environment chọn source polling hoặc ngắt GPIO ở `Polling/` và `Interrupt/`; hai thư mục mode có cấu hình độc lập nếu muốn mở riêng.
- `L2/` — đếm cạnh nút, so sánh cạnh thô với cạnh đã debounce.
- `L3/` — ADC 12-bit, moving average 8 mẫu và LEDC.
- `Diagrams/` — sơ đồ mạch Fritzing/ảnh xuất dùng chung L1–L2 và sơ đồ riêng L3.

## Cấu hình chung

Các project dùng board PlatformIO `esp32dev`, framework Arduino và Espressif32 platform 6.6.0 (Arduino-ESP32 2.x). Cấu hình này dùng API LEDC `ledcSetup`, `ledcAttachPin` và `ledcWrite(channel, duty)`, tương ứng với project trong ảnh. Serial Monitor chạy ở 115200 baud.

Mở một trong ba thư mục `L1`, `L2`, `L3` làm project PlatformIO. Với L1, build/nạp mode cần thử bằng `pio run -e polling` hoặc `pio run -e interrupt`; L2 và L3 dùng `pio run`. Serial Monitor đặt 115200 baud.

## Báo cáo tổng hợp

### Bài 1 — Polling và ngắt ngoài

Hai mode dùng cùng nút GPIO4 kéo xuống GND và LED GPIO2. Cả hai làm LED tăng/giảm độ sáng bằng PWM 5 kHz, độ phân giải 8-bit. Bản polling kiểm tra nút theo từng bước fade 10 ms; bản ngắt đếm cạnh xuống trong ISR rồi loop xử lý sự kiện và nháy LED. ISR không gọi Serial hay delay.

| Chế độ | Phản hồi quan sát được | Ghi nhận sau khi chạy |
|---|---|---|
| Polling | In một dòng khi phát hiện cạnh nhấn; kiểm tra nút theo bước fade | Thời gian/trải nghiệm thực tế: điền sau khi đo |
| Ngắt ngoài | LED chớp theo sự kiện ISR đã xếp hàng | Thời gian/trải nghiệm thực tế: điền sau khi đo |

Polling hiện phát hiện chuyển HIGH→LOW để tránh in lặp khi giữ nút. Cách này chưa debounce; dội tiếp điểm vẫn có thể tạo thêm sự kiện.

### Bài 2 — Debounce

Mạch giữ nguyên như Bài 1. ISR đếm mọi cạnh xuống vào `raw_edges`, chỉ tăng `accepted` nếu đã qua ngưỡng debounce. Ngưỡng mặc định là 50 ms; code cho phép thử 5, 50 và 500 ms.

| Ngưỡng | Cạnh thô sau 5 lần nhấn | Lần nhấn hợp lệ | Nhận xét thực nghiệm |
|---:|---:|---:|---|
| 5 ms | ______ | ______ | ____________________ |
| 50 ms | ______ | ______ | ____________________ |
| 500 ms | ______ | ______ | ____________________ |

Chọn ngưỡng cuối: ______ ms. Lý do: __________________________________________.

### Bài 3 — ADC, lọc và LEDC

Biến trở đưa điện áp wiper vào ADC1 GPIO34; code đọc ADC 12-bit, tính trung bình trượt 8 mẫu và điều khiển LED GPIO18 bằng PWM. Serial xuất cả dữ liệu thô và đã lọc để đối chiếu trên Serial Plotter.

| Trạng thái | Quan sát số liệu khi không chạm biến trở | Quan sát khi quay min→max |
|---|---|---|
| ADC thô | ____________________ | ____________________ |
| Moving average 8 mẫu | ____________________ | ____________________ |

Đánh giá phép thử ADC2 (GPIO27 trên ESP32 cổ điển), gồm điều kiện Wi-Fi và kết quả đọc: ________________________________.

Các ô kết quả đo để trống cho tới khi chạy mạch thật; không có số liệu phần cứng được giả lập trong bộ source này.

## Sơ đồ đấu nối

Mở `Diagrams/L1-L2.fzz` và `Diagrams/L3.fzz` bằng Fritzing để xem/sửa project. Ảnh PNG và SVG trong cùng thư mục là sơ đồ đấu dây để xem nhanh. File L1–L2 giữ nguyên sơ đồ Fritzing đã có trong tài liệu; sơ đồ xem nhanh bổ sung điện trở 220–330 Ω nối tiếp LED rời để bảo vệ LED và GPIO.

Lưu ý điện áp: chỉ cấp 3.3 V cho GPIO/ADC; không đưa 5 V vào chân ESP32. GPIO34 chỉ dùng làm input. Với LED rời, mắc điện trở 220–330 Ω nối tiếp.
