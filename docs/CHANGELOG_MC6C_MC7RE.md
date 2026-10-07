# Thay đổi tích hợp MC6C và MC7RE V2

Bản nền: `5e0b0876b424970ebf454aec68df33d2ef8dc9d6` từ
`electrondrift123/stm32-flight-controller`.

## Nhận điều khiển

- Bỏ RF24, cấu hình SPI cho radio, ngắt GPIO và ACK telemetry.
- Thêm UART SBUS ở PA10, 100000 baud, 8E2, qua mạch đảo ngoài.
- Giải mã 16 kênh; sử dụng CH1–CH6 cho lệnh điều khiển.
- Chuẩn hóa đầu vào, thêm vùng giữa và giới hạn lệnh.
- Thêm timeout 200 ms, xử lý lost/failsafe và kiểm tra giới hạn dữ liệu.
- Liên động CH5: phải thấy OFF khi khởi động và sau mất tín hiệu trước khi nhận ON.
- Gán CH6 cho yêu cầu hạ; bỏ yêu cầu hạ tự động của timeout nRF24 cũ.

## Điều khiển và xử lý lỗi

- Khởi động trong trạng thái khóa; không arm nếu link hoặc cảm biến chưa hợp lệ.
- Bốn đầu ra ESC giữ 1000 µs khi DISARMED; ARMED_IDLE giữ 1150 µs.
- Sửa góc thân cho phép arm từ so sánh 10 rad sang 10 độ.
- Tính prescaler TIM2 theo clock APB1 thực tế thay vì hằng 84 MHz.
- Không công bố mẫu cảm biến chưa khởi tạo hoặc đọc lỗi.
- Nhánh báo lỗi cảm biến trả mutex serial; không chờ vô hạn và không giữ mutex I2C trong thời gian báo còi.
- PID khóa khi trạng thái cảm biến không hợp lệ; watchdog chỉ được làm mới khi trạng thái hợp lệ.
- Dùng bước thời gian 0,01 s cho PI vận tốc đứng gọi tại 100 Hz.
- Bỏ macro GPIO và hằng toán học trùng với framework để tránh ghi đè định nghĩa.

## Tài liệu và kiểm tra

- Thêm báo cáo tiếng Việt, sơ đồ khối, mạch đảo SBUS và trạng thái bay.
- Thêm hướng dẫn lắp, nạp, gán kênh và nghiệm thu phần cứng.
- Thêm kiểm thử host cho parser, mapping, timeout và liên động.
- Thêm workflow GitHub kiểm tra host và biên dịch firmware.
- Ghi lại kết quả build và mã băm bản nhị phân trong `VALIDATION.md`.

Không có telemetry về MC6C hoặc file KiCad/Gerber mới. Cảm biến, PID, ADC pin,
thứ tự động cơ và chính sách mất link cần được xác nhận trên mô hình thực tế.
