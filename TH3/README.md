# Thí nghiệm 3 — Tầng mạng không dây trên ESP32

Bộ nộp gồm 5 project PlatformIO: Wi-Fi AP/STA, BLE GATT server, ESP-NOW transmitter/receiver, HTTP điều khiển LED và MQTT publish/subscribe. Các project theo cùng board `esp32dev`, framework Arduino, Espressif32 6.6.0 và Serial 115200 baud như bộ TH2.

## Cấu trúc và cách chạy

| Bài | Project | Môi trường / lệnh build | Quan sát chính |
|---|---|---|---|
| 1 | `Bai1-WiFi/` | `pio run -d TH3/Bai1-WiFi -e ap` hoặc `-e sta` | AP phát SSID và IP; STA in IP được cấp |
| 2 | `Bai2-BLE/` | `pio run -d TH3/Bai2-BLE` | nRF Connect quét thấy server, đọc/ghi characteristic |
| 3 | `Bai3-ESP-NOW/` | `pio run -d TH3/Bai3-ESP-NOW -e receiver` hoặc `-e transmitter` | Receiver in gói tin tăng dần; ghi MAC hai kit |
| 4 | `Bai4-HTTP/` | `pio run -d TH3/Bai4-HTTP` | Trình duyệt gọi `/on`, `/off`, `/toggle` để điều khiển LED GPIO2 |
| 5 | `Bai5-MQTT/` | `pio run -d TH3/Bai5-MQTT` | Publish bộ đếm, subscribe lệnh `1`/`0` điều khiển LED |

Mở riêng thư mục bài làm làm project PlatformIO cũng được. Mở Serial Monitor ở 115200 baud. Các kết quả phần cứng phải được chạy trên kit thật rồi ghi lại; bộ mã không điền trước IP, MAC, số đếm hay ảnh minh chứng.

## Wi-Fi và thông tin riêng

Trong các project STA (`Bai1-WiFi`, `Bai4-HTTP`, `Bai5-MQTT`), sao chép `include/secrets.example.h` thành `include/secrets.h`, rồi điền SSID/mật khẩu mạng. `secrets.h` được Git bỏ qua để tránh đẩy mật khẩu lên repo. SSID/mật khẩu AP demo của Bài 1 cũng nên đổi theo nhóm.

Với MQTT, sửa `TH3_MQTT_GROUP` thành topic riêng của nhóm, ví dụ `nhom-07`. Mỗi kit nên có `TH3_MQTT_CLIENT_ID` khác nhau. Mặc định broker là `test.mosquitto.org:1883`, theo đề bài; broker công cộng không bảo đảm luôn sẵn sàng. Có thể đổi sang IP máy chạy Mosquitto trong LAN.

## Bài 4 — sơ đồ

`Bai4-HTTP/Bai4-Fritzing.fzz` là project Fritzing có thể mở và chỉnh sửa; `wiring-diagram.svg` là sơ đồ xem nhanh. Mạch Fritzing điều khiển LED rời tại GPIO2 qua điện trở 220 Ω xuống GND. Chỉ đưa 3.3 V vào GPIO.

## Báo cáo và minh chứng

Thêm ảnh/video chạy thật vào `Evidence/` của từng bài theo tên gợi ý trong các file `Evidence/README.md`. Bảng dưới để trống để điền sau khi đo; không thay bằng kết quả dự đoán.

| Bài | Kết quả cần ghi |
|---|---|
| 1 | SSID AP; IP AP; SSID STA đã nối; IP STA |
| 2 | Tên BLE; UUID dịch vụ/đặc tính; giá trị đọc và giá trị ghi lại |
| 3 | MAC Transmitter/Receiver; số gói nhận; kết quả thử chéo nhóm nếu có |
| 4 | IP ESP32; trạng thái LED ở `/on` và `/off`; ảnh trình duyệt và video/ảnh LED |
| 5 | Broker/topic; client ID hai kit; ảnh app nhận dữ liệu và kết quả lệnh LED nếu thử |

**Trạng thái hiện tại:** source và hướng dẫn đã được chuẩn bị; chưa có ảnh/video hay số đo phần cứng. Cần nạp từng project, thực hiện phép thử và bổ sung minh chứng trước khi nộp.
