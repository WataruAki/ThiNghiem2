# Bài 1 — LED fading: polling và ngắt ngoài

Hai mode chạy trên cùng mạch theo tài liệu: nút nhấn ở GPIO4 dùng INPUT_PULLUP, LED ở GPIO2. Nút nối GPIO4–GND; LED rời cần điện trở nối tiếp 220–330 Ω. Hai bản code vẫn nằm riêng trong `Polling` và `Interrupt`, còn `L1` là một project PlatformIO với hai environment.

**Build/nạp:** Có thể mở `L1` làm project PlatformIO rồi chọn environment `polling` hoặc `interrupt`, hoặc mở thẳng thư mục `Polling`/`Interrupt` để build mode đó như project riêng. Cách build từ `L1`:

```powershell
pio run -e polling
pio run -e interrupt
```

Nạp một mode bằng `pio run -e polling -t upload` hoặc `pio run -e interrupt -t upload`. Project có `L1/src/main.cpp` làm điểm vào; file này chọn đúng bản code theo environment. Vì vậy build từ thư mục `L1` là đúng cấu hình.

Nếu mở trực tiếp một thư mục mode, mỗi thư mục đã có `platformio.ini` riêng: chạy `pio run` và `pio run -t upload` ngay trong `Polling` hoặc `Interrupt`.

## Polling

Chọn environment `polling`, nạp chương trình và mở Serial Monitor 115200. LED tăng/giảm PWM 0–255; loop đọc nút sau mỗi bước 10 ms và in một dòng ở cạnh nhấn HIGH→LOW. Khi giữ nút, code không in lặp. Nếu nhả và nhấn lại sẽ ghi nhận lần mới.

## Interrupt

Chọn environment `interrupt`. Ngắt FALLING trên GPIO4 chỉ tăng bộ đếm `capturedButtonEvents`. Vòng lặp chính lấy từng sự kiện khỏi hàng đợi, in Serial và chớp LED tắt/sáng 80 ms, sau đó tiếp tục fade. Không gọi Serial, delay hay xử lý dài trong ISR.

## Báo cáo

Mục tiêu là quan sát phản hồi của nút khi LED đang fade. Polling phụ thuộc chu kỳ kiểm tra; ngắt ghi nhận cạnh xuống ngay cả khi loop đang bận. Chương trình vẫn xử lý hiệu ứng ở loop nên thời điểm LED chớp có thể trễ tới bước fade tiếp theo.

| Phiên bản | Kết quả cần quan sát | Kết quả thực tế |
|---|---|---|
| Polling | Serial báo nhấn khi loop đọc thấy LOW | ____________________ |
| Ngắt ngoài | LED chớp và loop xử lý từng sự kiện ISR đã xếp hàng | ____________________ |

### Kiểm tra bộ đếm khi loop bận

Ở bản ngắt, ISR tăng `capturedButtonEvents` cho mỗi cạnh FALLING; loop xử lý từng sự kiện đã xếp hàng. Khi nhấn nhanh trong lúc LED đang chớp, chờ dòng `DRAINED ISR=...`: tổng ISR đã nhận phải bằng số sự kiện đã xử lý, không còn sự kiện chờ. Bài 1 chưa debounce nên dội tiếp điểm có thể làm số cạnh ISR lớn hơn số lần nhấn vật lý.

| Lần thử | Số lần nhấn vật lý | ISR đã nhận (`DRAINED ISR`) | Sự kiện đã xử lý | Ghi chú |
|---|---:|---:|---:|---|
| Nhấn nhanh khi LED đang fade/chớp | 5 | ______ | ______ | ____________________ |

### Ảnh cần chụp

- Chụp cửa sổ Serial Monitor ở 115200 baud, có các dòng `handled=...`, `ISR=...`, `pending=...` và dòng kết thúc `DRAINED ISR=...`.
- Để lấy ảnh: nạp project Interrupt, mở PlatformIO Serial Monitor, nhấn nhanh 5 lần trong lúc LED đang chạy, đợi `DRAINED`, rồi nhấn `Win + Shift + S` và kéo vùng chọn quanh Serial Monitor.
- Ghi số đo từ ảnh vào bảng. Đừng kết luận rằng 5 lần nhấn vật lý luôn cho đúng 5 cạnh ở Bài 1; phần dội nút được đo và lọc ở Bài 2.

Ảnh minh chứng lưu tại `Evidence/L1_interrupt_serial.png`.
