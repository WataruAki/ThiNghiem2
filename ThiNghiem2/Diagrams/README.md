# Sơ đồ Fritzing

- `L1-L2.fzz`: sơ đồ nút GPIO4 và LED GPIO2 dùng chung cho Bài 1 (polling/ngắt) và Bài 2 (debounce), dựa trên file `Lesson1.fzz` người học đã tải. Part ESP32 được đóng gói kèm để Fritzing tìm thấy board khi mở file.
- `L3.fzz`: sơ đồ biến trở 10 kΩ vào GPIO34/ADC1 và LED PWM từ GPIO18, đã đổi board Arduino mẫu thành ESP32 và giữ điện trở nối tiếp LED.
- `L1-L2-wiring.png` / `.svg` và `L3-wiring.png` / `.svg`: sơ đồ đấu dây tĩnh để xem nhanh hoặc chèn vào báo cáo.

Mở các file `.fzz` bằng Fritzing để xem hoặc chỉnh sửa. Ở sơ đồ L1–L2 gốc, LED được nối trực tiếp theo hình trong tài liệu; khi lắp LED rời ngoài thực tế, dùng điện trở 220–330 Ω mắc nối tiếp. Sơ đồ xem nhanh đã thể hiện điện trở này. Nếu dùng LED tích hợp trên board, không cần lắp LED rời.

Chỉ nối biến trở giữa 3V3 và GND; chân giữa (wiper) đi vào GPIO34. Không cấp 5 V cho GPIO/ADC của ESP32.
