# Bài 2 — Debounce đúng cách

Mạch giống Bài 1: nút GPIO4 nối xuống GND với INPUT_PULLUP; LED báo GPIO2. Mặc định debounce 50 ms. Mở Serial Monitor ở 115200 baud để theo dõi hai bộ đếm:

- raw_edges: mọi cạnh xuống mà ISR nhận được, gồm cả cạnh do dội nút.
- accepted: số lần nhấn được chấp nhận sau khi áp dụng cửa sổ debounce.

**Ngưỡng chọn: 50 ms** — lớn hơn thời gian dội nút thường gặp (khoảng 5–20 ms) nhưng vẫn đủ ngắn để tách hai lần nhấn nhanh cách nhau hơn 50 ms.

## Cách thử

1. Chạy lần lượt với DEBOUNCE_MS bằng 5, 50 và 500.
2. Trước mỗi ngưỡng, reset board để bộ đếm bắt đầu từ 0; mở Serial Monitor ở 115200.
3. Nhấn nút nhanh 5 lần riêng biệt, cách nhau khoảng 100 ms, rồi chờ dòng tiếp theo xuất hiện.
4. Lấy chênh lệch bộ đếm từ đầu đến cuối phép thử và ghi số cạnh thô cùng số lần được chấp nhận vào bảng.

Với 5 lần nhấn cách nhau khoảng 100 ms, kết quả kỳ vọng là: 5 ms có thể nhận thêm cạnh dội; 50 ms cho `accepted = 5`; 500 ms có thể gộp một số lần nhấn. `raw_edges` thường bằng hoặc lớn hơn 5. Đây là dự đoán để đối chiếu, chưa phải số đo; kết quả thật phụ thuộc nút và nhịp nhấn.

## Báo cáo

| DEBOUNCE_MS | raw_edges sau 5 lần nhấn nhanh | accepted | Nhận xét |
|---:|---:|---:|---|
| 5 | ______ | ______ | ____________________ |
| 50 | ______ | ______ | ____________________ |
| 500 | ______ | ______ | ____________________ |

Ngưỡng chọn: **50 ms.** Lý do: loại phần lớn dội 5–20 ms nhưng vẫn nhận riêng các lần nhấn nhanh cách nhau khoảng 100 ms.

Các kết quả đo phần cứng đang để trống để điền sau khi chạy; source không giả định trước số cạnh dội của nút thực tế.

### Ảnh cần chụp

Chụp Serial Monitor có tiêu đề `raw_edges accepted` và giá trị cuối sau 5 lần nhấn. Nạp lại board trước mỗi ngưỡng để đếm từ 0; dùng `Win + Shift + S` để chụp vùng Serial Monitor. Lưu riêng ba ảnh `Evidence/L2_debounce_5ms.png`, `Evidence/L2_debounce_50ms.png` và `Evidence/L2_debounce_500ms.png`, rồi chèn chúng vào báo cáo.
