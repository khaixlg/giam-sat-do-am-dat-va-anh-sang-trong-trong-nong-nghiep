# 📡 Dashboard Giám sát và Điều khiển IoT (ESP32)

ESP32 đọc **ánh sáng** và **độ ẩm**, tự bật đèn khi trời tối, kêu còi cảnh báo theo ngưỡng độ ẩm, đồng thời gửi dữ liệu lên server qua HTTP. Server hiển thị dữ liệu trên **web dashboard** (Thymeleaf + Bootstrap) và điều khiển đèn LED từ xa.

## 📦 Thành phần

| File | Vai trò |
|---|---|
| `chuabt.ino` | Firmware ESP32 (Arduino + FreeRTOS) |
| `web.html` | Giao diện dashboard (Thymeleaf template, Bootstrap 5.3.3) |

## ✨ Tính năng

**Firmware ESP32** – 3 task chạy song song bằng FreeRTOS:

- **Task_LDR_LED**: đọc cảm biến ánh sáng (trung bình 5 lần), giá trị **dưới 750** thì bật đèn LED, ngược lại tắt. Chu kỳ 200 ms.
- **Task_Buzzer_Soil**: đọc độ ẩm, quy ra %, và điều khiển còi:

  | Độ ẩm | Hành vi của còi |
  |---|---|
  | ≤ 10% | Tắt, reset bộ đếm |
  | Trên 10% đến 50% | Cảnh báo: bíp tối đa 6 lần |
  | Trên 50% | Nguy hiểm: bíp mạnh hơn, tối đa 10 lần |

- **Task_SendData**: gửi dữ liệu lên server mỗi ~0,7 giây và nhận lệnh bật/tắt đèn từ server.

**Web dashboard** (`web.html`):

- Ô trạng thái **Đèn LED** (ĐANG BẬT / ĐANG TẮT) kèm nút **BẬT LED**, **TẮT LED**.
- Ô trạng thái **Còi báo động**.
- Bảng **dữ liệu cảm biến**: thời gian, ánh sáng, độ ẩm (%).

## 🧰 Phần cứng

| Linh kiện | Chân ESP32 |
|---|---|
| Cảm biến độ ẩm (Soil) | GPIO 34 (analog) |
| Cảm biến ánh sáng LDR | GPIO 35 (analog) |
| Đèn LED | GPIO 15 |
| Còi (buzzer) | GPIO 32 (PWM, 1500 Hz, 8 bit) |

## 🔌 Giao tiếp ESP32 ↔ Server

ESP32 gửi bằng **HTTP POST** (`application/x-www-form-urlencoded`) tới địa chỉ `serverName`:

```
light=<giá trị ánh sáng>&humidity=<độ ẩm %>
```

Server trả về JSON để điều khiển đèn:

```json
{ "led": 1 }
```

`1` = bật đèn, `0` = tắt đèn.

## 🚀 Cài đặt

### Firmware

1. Cài **Arduino IDE** và board **esp32 by Espressif Systems** (core **3.x**, code dùng `ledcAttach`).
2. Cài thư viện **ArduinoJson** (Library Manager).
3. Mở `chuabt.ino`, điền 3 dòng:
   ```cpp
   const char* ssid       = "YOUR_WIFI_NAME";
   const char* password   = "YOUR_WIFI_PASSWORD";
   const char* serverName = "http://<IP_SERVER>:<PORT>/<đường_dẫn>";
   ```
4. Chọn board **ESP32 Dev Module**, chọn cổng COM, bấm **Upload**.
5. Mở Serial Monitor (115200 baud) để xem IP và trạng thái kết nối.

> ESP32 và máy chạy server cần **cùng mạng WiFi/LAN** (WiFi 2.4 GHz).

### Web dashboard

`web.html` là template **Thymeleaf**, cần đặt trong project Java (ví dụ Spring Boot) tại `src/main/resources/templates/` và để controller truyền vào các biến:

| Biến | Ý nghĩa |
|---|---|
| `ledStatus` | `1` = LED đang bật, `0` = tắt |
| `buzzerStatus` | `1` = còi đang bật, `0` = tắt |
| `data` | Danh sách bản ghi có `timestamp`, `light`, `humidity` |

Nút điều khiển LED gọi hàm JavaScript `sendLedControl(1)` / `sendLedControl(0)`, hàm này cần gửi lệnh về server.

## ⚙️ Thông số có thể chỉnh (`chuabt.ino`)

| Thông số | Giá trị hiện tại |
|---|---|
| Ngưỡng bật đèn (LDR) | `rawValue < 750` |
| Ngưỡng cảnh báo độ ẩm | `10% < độ ẩm ≤ 50%` |
| Ngưỡng nguy hiểm độ ẩm | `độ ẩm > 50%` |
| Chu kỳ gửi dữ liệu | `700 ms` |
