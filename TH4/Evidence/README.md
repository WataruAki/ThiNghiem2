# Minh chứng TH4

| Bài | File gợi ý | Nội dung cần thấy |
|---|---|---|
| 1 | `Bai1-REST-JSON.png` | Địa chỉ `http://<IP>/api/sensor` và JSON có `rssi`. |
| 2 | `Bai2-MQTT-Telemetry.png` | MQTT Explorer đang nhận `aet2022/<SV_ID>/telemetry` và JSON có `rssi`. |
| 3 | `Bai3-Dashboard.png` | Gauge, chart và trạng thái nút/LED cập nhật từ ESP32. |
| 3 | `Bai3-LED-OLED.mp4` | Bấm bật/tắt trên dashboard và thấy LED cùng OLED đổi. Có thể quay thêm thao tác nút vật lý để thấy bộ đếm thay đổi. |
| 4 | `Bai4-MQTT-Explorer.png` | Cây topic theo mã riêng, payload telemetry và bản tin `cmd/led` với giá trị `1` hoặc `0`. Nếu cần, chụp hai ảnh để thấy đủ payload. |

IP ESP32 ở Bài 1: …

Mã dùng trong topic và quan sát khi chạy: …

Dùng `Win + Shift + S` để chụp cửa sổ trình duyệt hoặc Explorer. Để thấy LED và OLED vật lý, quay kit cùng lúc với thao tác trên dashboard.
