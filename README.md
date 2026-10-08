# 🏠 Nhà thông minh – ESP32 + Web Dashboard IoT

Hệ thống giám sát và điều khiển IoT gồm **ESP32** (đọc cảm biến, điều khiển đèn, còi cảnh báo) và **web dashboard** (Java Spring Boot + Thymeleaf) hiển thị dữ liệu theo thời gian thực, đồng thời điều khiển đèn LED từ xa.

## ✨ Tính năng

- **Đo ánh sáng** bằng cảm biến quang trở (LDR): tự bật đèn LED khi trời tối.
- **Đo độ ẩm** bằng cảm biến độ ẩm, quy ra phần trăm.
- **Còi cảnh báo** theo hai mức:
  - Cảnh báo: độ ẩm từ trên 10% đến 50% → còi bíp ngắt quãng.
  - Nguy hiểm: độ ẩm trên 50% → còi bíp mạnh hơn.
  - Độ ẩm từ 10% trở xuống → còi tắt.
- **Gửi dữ liệu lên server** qua HTTP POST, server lưu lại lịch sử.
- **Điều khiển đèn LED từ xa**: server trả về trạng thái `led`, ESP32 bật/tắt đèn theo đó.
- **Web dashboard**:
  - Hiển thị trạng thái đèn LED và còi báo động.
  - Nút **BẬT LED / TẮT LED**.
  - Bảng dữ liệu cảm biến (thời gian, ánh sáng, độ ẩm).
- Chạy đa luồng bằng **FreeRTOS** (mỗi chức năng một task riêng).

## 🧱 Kiến trúc

```
┌────────────┐   POST /sensor/add    ┌──────────────────────┐
│   ESP32    │ ────────────────────► │  Server Spring Boot  │
│ LDR, Soil  │  light, humidity      │  (Thymeleaf + DB)    │
│ LED, Buzzer│ ◄──────────────────── │                      │
└────────────┘   JSON {"led": 0|1}   └──────────┬───────────┘
                                                │
                                         ┌──────▼──────┐
                                         │ Web Dashboard│
                                         └─────────────┘
```

## 🧰 Phần cứng

| Linh kiện | Chân ESP32 |
|---|---|
| Cảm biến độ ẩm (Soil) | GPIO 34 (analog) |
| Cảm biến ánh sáng LDR | GPIO 35 (analog) |
| Đèn LED | GPIO 15 |
| Còi (buzzer) | GPIO 32 (PWM) |

Sơ đồ nối dây xem trong `assets/so-do.png`.

## 💻 Công nghệ

- **Firmware**: Arduino framework cho ESP32, FreeRTOS, `HTTPClient`, `ArduinoJson`.
- **Backend / Web**: Java Spring Boot, Thymeleaf, Bootstrap 5.3.
- **Giao tiếp**: HTTP (form-urlencoded) giữa ESP32 và server.

## 🔌 Giao tiếp ESP32 ↔ Server

**ESP32 gửi** mỗi ~0,7 giây:

```
POST /sensor/add
Content-Type: application/x-www-form-urlencoded

light=<giá trị LDR>&humidity=<độ ẩm %>
```

**Server trả về** JSON để điều khiển đèn:

```json
{ "led": 1 }
```

`1` = bật đèn, `0` = tắt đèn.

## 🚀 Cài đặt

### 1. Chạy server (web dashboard)

1. Cài **JDK** và (tuỳ dự án) **Maven/Gradle**.
2. Chạy ứng dụng Spring Boot, mặc định cổng **8080**.
3. Mở trình duyệt: `http://localhost:8080`.
4. Ghi lại **địa chỉ IP máy chạy server** trong mạng LAN (ví dụ `192.168.1.233`).

### 2. Nạp firmware cho ESP32

1. Cài **Arduino IDE** và board **esp32 by Espressif Systems** (core 3.x).
2. Cài thư viện **ArduinoJson** trong Library Manager.
3. Mở file `.ino` và chỉnh các dòng sau:
   ```cpp
   const char* ssid       = "YOUR_WIFI_NAME";
   const char* password   = "YOUR_WIFI_PASSWORD";
   const char* serverName = "http://<IP_SERVER>:8080/sensor/add";
   ```
4. Chọn board **ESP32 Dev Module**, chọn cổng COM, bấm **Upload**.
5. Mở Serial Monitor (115200 baud) để xem địa chỉ IP và log kết nối.

> ESP32 và máy chạy server phải **cùng mạng WiFi/LAN**.

## ⚙️ Thông số có thể chỉnh

| Thông số | Mặc định | Ý nghĩa |
|---|---|---|
| Ngưỡng LDR | `750` | Giá trị đọc được nhỏ hơn mức này thì bật đèn |
| Ngưỡng cảnh báo độ ẩm | `10 – 50 %` | Còi bíp ở mức cảnh báo |
| Ngưỡng nguy hiểm độ ẩm | `> 50 %` | Còi bíp ở mức nguy hiểm |
| Tần số còi | `1500 Hz` | PWM 8 bit |
| Chu kỳ gửi dữ liệu | `700 ms` | Task gửi dữ liệu lên server |
