# TH2 - Bài 2: active, các chế độ ngủ và RTC memory

Hai file `.cpp` độc lập, mỗi env chỉ build/nạp một file:

| Env | File | Nội dung |
|---|---|---|
| `compare` | `src/vd1_cac_che_do.cpp` | Active/modem-sleep/light-sleep qua lệnh Serial |
| `deep_sleep` | `src/vd2_deep_sleep.cpp` | Thức 5 s, deep sleep 10 s, theo dõi biến RTC/RAM |
| `deep_sleep_button` | Cùng file deep sleep | Thêm đánh thức EXT0 bằng GPIO4 |

Chọn cách quan sát LED, Serial, kết nối Wi-Fi và biến nhớ. Không cần OLED,
web hay cảm biến dòng ngoài. Đây là phương án quan sát khác được phần bài tập
cho phép; không phải thực hiện lại toàn bộ demo giao diện web.

## 1. Phần cứng và cấu hình

- ESP32-S3 DevKit, cùng cấu hình S3/flash 4 MB tối thiểu như Bài 1.
- LED rời: GPIO2 → điện trở 220–330 Ω → anode; cathode → GND.
- Bản `deep_sleep` chỉ cần LED và cáp USB, không cần Wi-Fi hay nút.
- Bản `deep_sleep_button`: thêm nút GPIO4 ↔ GND và điện trở kéo lên
  10 kΩ giữa GPIO4 ↔ 3V3. Code cũng cấu hình pull-up RTC.
- Không dùng nút BOOT GPIO0 thay cho GPIO4. Biến trở Bài 1 không dùng ở bài này.

Ưu tiên cổng USB-UART của board cho các thí nghiệm sleep để dễ theo dõi boot
và log. USB native có thể ngắt/kết nối lại, đổi COM hoặc làm mất log khi sleep;
trạng thái cổng USB trên máy tính không đủ để kết luận CPU đã ngủ đúng.
Chưa xác minh mẫu board/cổng cụ thể trên thiết bị thật.

## 2. Làm deep sleep trước: không cần Wi-Fi

Mở terminal tại thư mục `bai2`:

```powershell
pio run -e deep_sleep -t upload
pio device monitor -b 115200
```

Nếu cần chọn cổng, dùng `pio device list`, thêm `--upload-port COM5` khi nạp
và `-p COM5` khi monitor. Thay COM5 bằng cổng thật. Nhấn RESET sau khi mở
monitor để xem từ đầu; đóng monitor trước lần nạp tiếp theo.

Mỗi lần boot/thức dậy:

1. `setup()` tăng cả `rtcWakeCount` và `normalWakeCount`, in nguyên nhân thức.
2. LED nháy 500 ms trong khoảng 5 s.
3. LED tắt, Serial được flush, cấu hình timer đánh thức sau 10 s rồi deep sleep.
4. Sau khi thức, chương trình chạy lại từ `setup()`.

**Dự kiến để đối chiếu, chưa phải log đo:** RTC tăng 1, 2, 3… qua các lần
deep sleep; biến thường luôn là 1 khi in sau bước tăng vì được khởi tạo lại
từ 0. `cause=TIMER` ở lần thức do timer; lần cấp nguồn/reset thông thường
trả `UNDEFINED`, không đồng nghĩa chắc chắn chỉ có nguyên nhân cấp nguồn.
`RTC_DATA_ATTR` không phải bộ nhớ lưu bền khi rút nguồn; không so sánh qua
các lần nạp lại firmware như một phép thử giữ RAM qua sleep.

Chụp ít nhất ba chu kỳ cho thấy `SETUP` lặp lại, RTC tăng và biến thường không
cộng dồn. Thời gian quan sát gồm cả boot/thức và ngủ nên không đúng 10 s
giữa mọi dòng log.

## 3. Mở rộng: đánh thức bằng nút GPIO4

```powershell
pio run -e deep_sleep_button -t upload
pio device monitor -b 115200
```

Nhả nút trước khi ngủ; nhấn sau khi LED tắt nhưng trước khi hết 10 s.
Kỳ vọng nguyên nhân thức là `EXT0`. Nếu không nhấn, timer vẫn đánh thức sau
10 s. EXT0 kích theo **mức LOW**, không phải ISR bắt cạnh nút. Giữ nút LOW
lúc vào sleep có thể làm thức ngay; code chờ nhả nút trước khi vào sleep.

Sau khi thức, code trả GPIO4 từ chức năng RTC về digital để đọc nút, rồi
cấu hình RTC pull-up khi chuẩn bị ngủ tiếp. GPIO4 trên S3 là RTC GPIO.

## 4. So sánh active và modem-sleep

Trong đầu file `src/vd1_cac_che_do.cpp`, điền `WIFI_SSID` và `WIFI_PASSWORD`
của mạng 2,4 GHz đang có. Không đưa mật khẩu thật vào repo nộp.
Để trống vẫn chạy được active radio-off và light sleep; modem-sleep khi đó
phải ghi **chưa kiểm tra**, không coi Wi-Fi tắt là modem-sleep.

```powershell
pio run -e compare -t upload
pio device monitor -b 115200
```

Đợi dòng `Wi-Fi IP`, gửi phím trong Serial Monitor (có thể kèm Enter):

| Lệnh | Tác dụng |
|---|---|
| `a` | Khi Wi-Fi đã kết nối: đặt `WIFI_PS_NONE` để làm mốc active |
| `m` | Khi Wi-Fi đã kết nối: đặt `WIFI_PS_MIN_MODEM` |
| `l` | Tắt Wi-Fi, vào light sleep 5 s rồi tiếp tục ngay sau hàm sleep |
| `h` | In hướng dẫn |

Sau `a`/`m`, code kiểm tra lại cấu hình driver bằng `esp_wifi_get_ps()`.
LED/heartbeat tiếp tục ở cả hai chế độ; điều này cho thấy loop còn hoạt động,
**không chứng minh được dòng điện giảm**. Cấu hình MIN_MODEM cho phép radio
ngủ giữa các lần phục vụ kết nối; nó không dừng CPU như deep sleep.
Hành vi thực tế còn phụ thuộc AP, DTIM và lưu lượng mạng.

Sau `l`, Wi-Fi đã tắt: reset để kết nối lại trước khi so sánh `a` và `m` tiếp.
Không so active radio-off với modem radio-on rồi kết luận mức tiết kiệm điện.

## 5. Quan sát light sleep

Lệnh `l` in biến cục bộ `local=41` và heartbeat trước khi ngủ. Khi thức, code
tiếp tục sau `esp_light_sleep_start()`, tăng biến lên 42, in thời gian đã trôi
và nguyên nhân thức. `setup()` không chạy lại. LED/heartbeat không được cập
nhật bởi loop trong khoảng ngủ. Giá trị 41/42 là giá trị khởi tạo và phép tăng
trong code để minh họa giữ RAM, không phải dữ liệu cảm biến.

Nếu API trả lỗi, code in `ERROR`; không coi khoảng dừng đó là sleep thành công.

| Trạng thái | Loop/LED | RAM thường | Điểm tiếp tục |
|---|---|---|---|
| Active | Vẫn chạy | Giữ | Trong loop |
| Modem-sleep | Vẫn chạy; radio có power saving | Giữ | Trong loop |
| Light-sleep | Loop dừng trong khoảng ngủ | Giữ | Sau hàm sleep |
| Deep-sleep | Loop dừng, boot lại khi thức | Khởi tạo lại | setup |

LED tắt một mình chưa chứng minh CPU ngủ: đó là lệnh code chủ động tắt LED.
Dùng tổ hợp log trước/sau sleep, lỗi API, nguyên nhân thức và hành vi RAM
để phân biệt.

## 6. Kết quả cần ghi để nộp

| Phép thử | Ảnh/log thực tế | Nhận xét |
|---|---|---|
| Active Wi-Fi + PS_NONE | Chưa chạy | |
| Modem-sleep + MIN_MODEM + có kết nối AP | Chưa chạy | |
| Light sleep 5 s, giữ biến, không lặp setup | Chưa chạy | |
| Deep sleep 10 s, RTC tăng, biến thường = 1 | Chưa chạy | |
| EXT0 GPIO4 trước hạn timer | Chưa chạy | |

Lưu ảnh/log vào `results/`, ghi mẫu board, cổng Serial và cấu hình đã nạp.
Giới hạn: quan sát hành vi không cho số mA hay năng lượng. LED nguồn, cầu
USB-UART và regulator trên DevKit cũng góp vào dòng toàn board; đo dòng cần
thiết bị, cách mắc nối tiếp và dải đo đủ xử lý dòng động/đỉnh phát radio.
Không tự điền các giá trị dòng điện khi chưa đo.

## Tài liệu và kiểm tra

- [Espressif: Sleep Modes, ESP32-S3 / IDF 4.4.6](https://docs.espressif.com/projects/esp-idf/en/v4.4.6/esp32s3/api-reference/system/sleep_modes.html)
- [Espressif: Wi-Fi Driver / Modem-sleep](https://docs.espressif.com/projects/esp-idf/en/v4.4.3/esp32s3/api-guides/wifi.html)
- API cũng được đối chiếu với header ESP-IDF trong Arduino-ESP32 2.0.14 đã cài.

Không có thư viện ngoài Arduino core/ESP-IDF. Ba cấu hình Bài 2 đã biên dịch
thành công cho ESP32-S3. Chưa có kết quả đo phần cứng thật.
