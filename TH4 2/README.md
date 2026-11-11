# TH4 – REST/MQTT và dashboard Node-RED

## Thông tin sinh viên

- **Họ và tên:** Vi Tuấn Kiệt
- **Mã sinh viên:** 25112515
- **Lớp:** BCSE2025
- **Vi điều khiển:** ESP32 DevKit V1
- **Môi trường:** VS Code + PlatformIO, mô phỏng bằng Wokwi (không có kit thật)
- **Dashboard:** Node-RED + node-red-dashboard
- **MQTT Broker:** broker.hivemq.com, cổng 1883

## Cách chạy

1. Mở từng thư mục bài bằng VS Code, build bằng `pio run`.
2. Nhấn `F1`, chọn `Wokwi: Start Simulator`.
3. Node-RED: cài palette `node-red-dashboard`, chọn Import, nạp file `NodeRED/flow.json`, bấm Deploy. Dashboard ở địa chỉ `http://localhost:1880/ui`.

## Nội dung thực hiện

### Bài 1 – REST API

- ESP32 kết nối `Wokwi-GUEST`, chạy HTTP server cổng 80.
- `GET /api/sensor` trả về JSON gồm `sv_id`, `temperature`, `humidity` đọc từ DHT22 (GPIO 15).
- Đã cấu hình `net.forward` trong `wokwi.toml` để thử truy cập `http://localhost:8180/api/sensor` từ trình duyệt, nhưng gặp lỗi `ERR_CONNECTION_RESET` do giới hạn của môi trường mô phỏng.
- ESP32 tự gọi `GET /api/sensor` vào chính địa chỉ IP của nó cũng báo `connection refused` trong mô phỏng.
- Để kiểm chứng nội dung API, ESP32 in ra Serial Monitor mỗi 10 giây đúng chuỗi JSON mà endpoint `/api/sensor` trả về (dùng chung hàm `taoJson()`), thay đổi khi chỉnh nhiệt độ/độ ẩm của DHT22.
- Mẫu JSON: `{"sv_id":"25112515","temperature":28.0,"humidity":65.0}`

### Bài 2 – MQTT telemetry

- Đọc DHT22 và publish JSON mỗi 5 giây lên topic `TH4/25112515/telemetry`.
- Dùng MQTT Explorer kết nối `broker.hivemq.com:1883` để quan sát topic.

### Bài 3 – Ứng dụng tổng hợp

- Dùng đủ bộ kit: DHT22 (GPIO 15), nút nhấn (GPIO 4, `INPUT_PULLUP`), LED (GPIO 2, điện trở 220 ohm), OLED SSD1306 (SDA 21, SCL 22).
- Bấm nút để đổi trạng thái LED, OLED hiển thị nhiệt độ, độ ẩm, trạng thái MQTT và trạng thái LED.
- Publish telemetry (kèm trạng thái `led`) lên `TH4/25112515/telemetry`.
- Subscribe `TH4/25112515/control`, nhận lệnh `ON`, `OFF`, `TOGGLE` để điều khiển LED từ xa.

### Node-RED

- Flow nhận telemetry qua MQTT, hiển thị 2 đồng hồ nhiệt độ/độ ẩm, biểu đồ lịch sử và trạng thái LED.
- Công tắc LED trên dashboard publish `ON`/`OFF` lên topic control.

## Minh chứng

- Ảnh JSON trả về từ `/api/sensor`: (thêm link)
- Ảnh MQTT Explorer thấy topic `TH4/25112515/telemetry`: (thêm link)
- Ảnh/video dashboard Node-RED đang chạy: (thêm link)
- Video bấm nút, LED và OLED đổi trạng thái: (thêm link)

## Cấu trúc thư mục

```
TH4/
├── Bai1_REST_API/
├── Bai2_MQTT_Telemetry/
├── Bai3_Ung_Dung_Tong_Hop/
├── NodeRED/
│   └── flow.json
├── images/
└── README.md
```

Mỗi bài firmware là một project PlatformIO gồm `src/main.cpp`, `platformio.ini`, `wokwi.toml` và `diagram.json`.
