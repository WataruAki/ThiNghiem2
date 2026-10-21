# Bài 3 — ADC 12-bit, lọc nhiễu và LEDC

Mắc biến trở 10 kΩ: hai đầu ngoài vào 3V3 và GND, chân giữa (wiper) vào ADC1 GPIO34. Mắc LED ở GPIO18 qua điện trở 220–330 Ω xuống GND. Không cấp 5 V vào GPIO. Nếu LED tối dần khi quay theo chiều mong muốn tăng, đổi hai đầu ngoài của biến trở.

Code đọc ADC độ phân giải 12-bit (0–4095), tính moving average 8 mẫu và điều khiển LEDC 5 kHz/8-bit. Serial Plotter chỉ vẽ hai chuỗi: `>tho:` (giá trị thô) và `>loc:` (giá trị đã lọc); PWM vẫn được tính và cập nhật ra LED nhưng không đưa lên đồ thị để hai đường ADC dễ so sánh. Quay biến trở từ min đến max; giá trị đã lọc và độ sáng LED cần đổi đơn điệu, ít giật hơn dữ liệu thô.

### Chụp ảnh so sánh thô và đã lọc

1. Nạp code bằng PlatformIO rồi đóng Serial Monitor để nhả cổng COM.
2. Mở Arduino IDE 2, chọn cùng cổng COM và mở **Tools → Serial Plotter**, đặt tốc độ 115200 baud. Chỉ một chương trình có thể mở cổng COM cùng lúc.
3. Giữ biến trở yên ở khoảng giữa vài giây để thấy đường `tho` dao động và `loc` ổn định hơn; sau đó quay chậm từ min đến max để thấy độ trễ/làm mượt của bộ lọc.
4. Chụp vùng đồ thị có tên hai đường `tho` và `loc` bằng `Win + Shift + S`; lưu tại `Evidence/L3_adc_raw_vs_filtered.png` và chèn vào báo cáo.

Nếu Serial Plotter không hiện đường, kiểm tra tốc độ 115200 và dòng dữ liệu bắt đầu bằng `>tho:`, `>loc:`. Có thể chụp cả giai đoạn giữ yên và giai đoạn quay biến trở trong cùng ảnh.

## Thử ADC2

Trên ESP32 cổ điển, đổi ADC_PIN từ GPIO34 sang GPIO27 để thử ADC2. Để quan sát xung đột ADC2/Wi-Fi, bật ENABLE_WIFI_FOR_ADC2_TEST. ADC2 bị hạn chế khi Wi-Fi hoạt động; ADC1 GPIO34 là lựa chọn của bài chính. Nếu board khác là ESP32-S3, hãy kiểm tra pin ADC tương ứng trước khi dùng cấu hình ADC2 này.

## Báo cáo

| Lần chạy | Nhiễu khi giữ nguyên biến trở | Độ mượt khi quay min→max | Ảnh/Serial Plotter |
|---|---|---|---|
| Thô và đã lọc | ____________________ | ____________________ | ____________________ |
| Thử ADC2 (GPIO27) | ____________________ | ____________________ | ____________________ |

Kết luận về moving average và ADC2: __________________________________________.

Số liệu và ảnh quan sát thực tế cần bổ sung sau khi chạy phần cứng; chưa có thiết bị nào được đo trong môi trường tạo file.
