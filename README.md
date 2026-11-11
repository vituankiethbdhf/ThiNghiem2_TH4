# TH4 – ESP32, REST API, MQTT và Node-RED

## 1. Thông tin sinh viên
- **Họ và tên:** Vi Tuấn Kiệt
- **Mã sinh viên:** 25112063
- **Tên đề tài:** Xây dựng ứng dụng IoT sử dụng ESP32, REST API, MQTT và Node-RED
- **Môi trường thực hiện:** PlatformIO, VS Code, Wokwi, Node-RED
- **MQTT Broker:** broker.hivemq.com
- **MQTT Port:** 1883

## 2. Giới thiệu

Repo này lưu trữ mã nguồn và kết quả thực hiện bài thực hành TH4 về lập trình IoT với ESP32.

Nội dung gồm 3 bài firmware:

1. Xây dựng REST API, sử dụng phương thức GET tại `/api/sensor` để trả về dữ liệu cảm biến dưới dạng JSON.
2. Sử dụng giao thức MQTT để gửi dữ liệu telemetry lên MQTT Broker theo mã sinh viên riêng.
3. Xây dựng ứng dụng IoT hoàn chỉnh sử dụng bộ kit ESP32 gồm cảm biến nhiệt độ, độ ẩm DHT22, nút nhấn, LED và màn hình OLED. Dữ liệu được gửi qua MQTT và có thể điều khiển LED từ Node-RED Dashboard.

## 3. Cấu trúc thư mục

```text
TH4/
│
├── Bai1_REST_API/
│   └── main.cpp
│
├── Bai2_MQTT/
│   └── main.cpp
│
├── Bai3_ESP32_Application/
│   ├── src/
│   │   └── main.cpp
│   ├── platformio.ini
│   └── diagram.json
│
├── Node-RED/
│   └── flow.json
│
└── README.md
```

## 4. Nội dung thực hiện

### Bài 1 – REST API

- ESP32 tạo REST API.
- Sử dụng phương thức GET tại địa chỉ `/api/sensor`.
- API trả về dữ liệu cảm biến dưới dạng JSON.
- Kiểm tra kết quả bằng trình duyệt hoặc công cụ gửi HTTP request.

Ví dụ dữ liệu JSON:

```json
{
  "temperature": 25.0,
  "humidity": 60.0
}
```

### Bài 2 – MQTT Publish Telemetry

- ESP32 kết nối WiFi Wokwi-GUEST.
- Kết nối MQTT Broker công khai HiveMQ.
- Gửi dữ liệu cảm biến theo chu kỳ 2 giây.
- Sử dụng mã sinh viên riêng: `25112515`.

Thông tin MQTT:

| Thuộc tính | Giá trị |
|---|---|
| Broker | broker.hivemq.com |
| Port | 1883 |
| Topic | AET2022/25112515/telemetry |
| QoS | 0 |

Ví dụ MQTT payload:

```json
{
  "sv_id": "25112515",
  "temperature": 25.0,
  "humidity": 60.0,
  "led": "ON"
}
```

### Bài 3 – Ứng dụng IoT hoàn chỉnh

Ứng dụng sử dụng các linh kiện:

- ESP32 DevKit V1.
- Cảm biến nhiệt độ, độ ẩm DHT22.
- Nút nhấn điều khiển.
- LED và điện trở 220Ω.
- Màn hình OLED SSD1306.

**Chức năng chính:**

- Đọc nhiệt độ và độ ẩm từ DHT22.
- Hiển thị nhiệt độ, độ ẩm, trạng thái MQTT và LED trên OLED.
- Bấm nút trên Wokwi để bật/tắt LED.
- Gửi dữ liệu telemetry lên MQTT.
- Nhận lệnh ON/OFF từ Node-RED Dashboard thông qua MQTT.
- Cập nhật trạng thái LED và OLED khi nhận lệnh điều khiển.

**Sơ đồ chân kết nối:**

| Linh kiện | Chân ESP32 |
|---|---|
| DHT22 DATA | GPIO 15 |
| LED | GPIO 2 |
| Button | GPIO 4 |
| OLED SDA | GPIO 21 |
| OLED SCL | GPIO 22 |

### Bài 4 – Node-RED Dashboard

Sử dụng file `Node-RED/flow.json` để import flow vào Node-RED.

Dashboard có các chức năng:

- Hiển thị nhiệt độ.
- Hiển thị độ ẩm.
- Hiển thị biểu đồ lịch sử nhiệt độ và độ ẩm.
- Hiển thị trạng thái LED.
- Điều khiển LED bằng công tắc ON/OFF.

**Các MQTT Topic sử dụng:**

| Chức năng | Topic |
|---|---|
| Nhận dữ liệu cảm biến | `AET2022/25112515/telemetry` |
| Gửi lệnh điều khiển LED | `AET2022/25112515/led/set` |

Lệnh điều khiển:

- `ON`: Bật LED.
- `OFF`: Tắt LED.

Node-RED nhận telemetry từ ESP32 và gửi lệnh điều khiển LED qua MQTT.

## 5. Hướng dẫn chạy chương trình

### 5.1. Chạy firmware ESP32

1. Mở project bằng VS Code và PlatformIO.
2. Kiểm tra các thư viện cần thiết.
3. Mở file `src/main.cpp`.
4. Chạy mô phỏng Wokwi.
5. Quan sát dữ liệu cảm biến và trạng thái thiết bị trên Serial Monitor.

### 5.2. Chạy Node-RED

1. Khởi động Node-RED.
2. Truy cập `http://localhost:1880`.
3. Chọn Import.
4. Chọn file `Node-RED/flow.json`.
5. Nhấn Import và Deploy.
6. Mở Dashboard tại `http://localhost:1880/ui`.

**Lưu ý:** Cần cài đặt gói `node-red-dashboard` để sử dụng giao diện Dashboard.

### 5.3. Kiểm tra điều khiển LED

1. Chạy mô phỏng ESP32 trên Wokwi.
2. Đảm bảo ESP32 đã kết nối MQTT.
3. Mở Node-RED Dashboard.
4. Gạt công tắc LED sang ON hoặc OFF.
5. Kiểm tra LED trên Wokwi và trạng thái hiển thị trên OLED.
6. Có thể bấm nút vật lý trên Wokwi để kiểm tra chức năng bật/tắt LED.

## 6. Kết quả thực hiện

Các hình ảnh và video minh chứng được lưu trong thư mục `Images/`.

| STT | Nội dung minh chứng | Tệp |
|---|---|---|
| 1 | Kết quả REST GET trả về JSON | `rest-api.png` |
| 2 | Giao diện Node-RED Dashboard | `dashboard.png` |
| 3 | LED và OLED thay đổi trạng thái | `wokwi-led-oled.png` |
| 4 | MQTT Explorer hiển thị topic và dữ liệu | `mqtt-explorer.png` |

## 7. Thư viện sử dụng

- WiFi.h
- PubSubClient
- DHT sensor library
- Adafruit Unified Sensor
- Adafruit GFX Library
- Adafruit SSD1306

## 8. Kết luận

Qua bài thực hành TH4, em đã thực hiện các nội dung cơ bản về lập trình IoT với ESP32, REST API, MQTT và Node-RED.

Ứng dụng có khả năng thu thập dữ liệu nhiệt độ, độ ẩm, gửi dữ liệu lên MQTT Broker, hiển thị thông tin trên OLED và điều khiển LED từ cả nút nhấn trên Wokwi và Node-RED Dashboard.

