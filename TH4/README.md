# TH4 · REST, MQTT và Node-RED

| Bài | Nội dung |
|---|---|
| 1 | [REST](bai1-rest/) trả JSON ở `GET /api/sensor` với `temp`, `hum`, `uptime` (giây), `rssi` (dBm) và `source`. |
| 2 | [MQTT publish](bai2-mqtt-pub/) gửi cùng cấu trúc JSON mỗi 2 giây vào `aet2022/<SV_ID>/telemetry`. |
| 3 | [MQTT và bộ kit](bai3-mqtt-sub-led/) gửi số đo SHT31, trạng thái nút và LED. Dashboard gửi lệnh bật/tắt LED và OLED hiển thị trạng thái mới. |
| 4 | MQTT Explorer dùng lại firmware Bài 2 hoặc Bài 3. Dùng Bài 3 để quan sát cả telemetry lẫn lệnh LED. |

## Cấu hình ESP32

1. Sao chép [`common/secrets.example.h`](common/secrets.example.h) thành `common/secrets.h`. Điền Wi-Fi, `TH4_SV_ID` bằng mã của bạn và broker lớp dùng. Mã dùng tối đa 48 ký tự ASCII gồm chữ, số, `_` hoặc `-`. Git bỏ qua `secrets.h`.
2. Mở thư mục bài tương ứng trong PlatformIO. Chọn `esp32dev`, Upload rồi mở Serial Monitor ở 115200 baud để xem IP, broker và topic. Nếu dùng terminal từ thư mục `TH4`, ví dụ `pio run -d bai1-rest -t upload` và `pio device monitor -d bai1-rest`.
3. Bài 1 mở `http://<IP-ESP32>/api/sensor` từ máy cùng mạng. Bài 1–2 dùng dữ liệu demo có trường `source: "demo"`. Bài 3 dùng `source: "sht31"` và trả `null` cho `temp`, `hum` khi cảm biến lỗi.

Trên Windows, nếu Xtensa báo `Invalid argument` khi biên dịch, mở repo từ đường dẫn chỉ dùng ký tự ASCII, ví dụ `C:\Projects\ThiNghiem2`.

### Đấu kit Bài 3

| Linh kiện | Chân ESP32 / cách đấu |
|---|---|
| SHT31 I²C (`0x44`) và OLED SSD1306 128×64 I²C (`0x3C`) | Cả hai dùng SDA → GPIO21, SCL → GPIO22, VCC → 3.3 V, GND → GND. |
| Nút vật lý | GPIO4 → nút → GND. Firmware dùng `INPUT_PULLUP` và debounce 35 ms. |
| LED rời | GPIO14 → điện trở 220 Ω → anode LED, cathode LED → GND. |

Địa chỉ I²C và chân nằm đầu [firmware Bài 3](bai3-mqtt-sub-led/src/main.cpp). Nút vật lý đảo trạng thái LED và tăng `presses` khi nhấn. Nút web gửi trạng thái đích `1` hoặc `0`. OLED và telemetry phản hồi trạng thái LED thực tế sau cả hai thao tác.

## Node-RED và MQTT Explorer

1. Nếu máy chưa có Node-RED, [cài Node-RED](https://nodered.org/docs/getting-started/local) rồi chạy `node-red`. Mở `http://localhost:1880`. Trong **Manage palette → Install**, cài [`node-red-dashboard`](https://flows.nodered.org/node/node-red-dashboard) theo đề.
2. **Import** [`nodered-flow/flow.json`](nodered-flow/flow.json). Đổi `YOUR_SV_ID` trong node **Nhận telemetry** (`aet2022/<SV_ID>/telemetry`) và **Gửi lệnh LED** (`aet2022/<SV_ID>/cmd/led`). Đặt broker, cổng và thông tin đăng nhập trùng firmware. Mặc định cả hai dùng `test.mosquitto.org:1883`.
3. Nạp firmware Bài 3, bấm **Deploy**, mở `http://localhost:1880/ui`. Dashboard có gauge nhiệt/ẩm, biểu đồ nhiệt độ, uptime/RSSI và trạng thái nút/LED. Bấm **BẬT LED** rồi **TẮT LED** và quan sát LED, OLED cùng trạng thái trả về dashboard.
4. Trong MQTT Explorer, kết nối cùng broker và subscribe `aet2022/<SV_ID>/#`. Mở `telemetry` để xem JSON và `cmd/led` để xem lệnh khi bấm nút web. Mở Explorer trước khi bấm vì lệnh không bật retain.

REST trả một mẫu khi client gửi GET. MQTT chủ động gửi mẫu định kỳ qua broker, đồng thời nhận lệnh qua một topic riêng.

## Minh chứng

Xem [danh sách ảnh/video cần chụp](Evidence/README.md). Thư mục này đang chờ kết quả từ kit và Node-RED của bạn. Sau khi đổi topic/broker và chạy dashboard, export lại flow từ Node-RED để lưu cấu hình đã dùng.
