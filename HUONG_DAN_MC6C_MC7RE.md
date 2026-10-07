# Thay bộ phát/thu bằng MC6C + MC7RE-V2

Bản sửa dựa trên dự án https://github.com/electrondrift123/stm32-flight-controller,
commit `5e0b0876b424970ebf454aec68df33d2ef8dc9d6`.
Bo mạch mục tiêu: BlackPill STM32F411CE. Dùng SBUS, không dùng các dây PWM riêng.
Báo cáo đầy đủ: [PDF](docs/BAO_CAO_DO_AN_MC6C_MC7RE.pdf), [bản có thể sửa](docs/BAO_CAO_DO_AN_MC6C_MC7RE.md).
Kết quả phiên bản cuối: [VALIDATION.md](docs/VALIDATION.md).

Code đã biên dịch và kiểm thử trên máy tính; chưa thử với bộ thu, ESC hoặc bay thực tế.
Ảnh PCB và sơ đồ hệ thống trong README gốc chưa được thiết kế lại cho phần cứng mới.

## 1. Linh kiện và đấu dây

| Linh kiện | Cách dùng |
|---|---|
| MC6C | Tay điều khiển; không cần nạp firmware tay phát tự chế |
| MC7RE-V2 | Bộ thu; lấy tín hiệu ở cổng SBUS/M.BUS |
| BEC 5 V ổn áp | Cấp nguồn bộ thu, trong giới hạn 4,5–6 V |
| Transistor NPN 2N3904 hoặc tương đương | Đảo tín hiệu SBUS cho STM32F411 |
| Điện trở 1 kΩ | Từ tín hiệu SBUS tới chân B transistor |
| Điện trở 10 kΩ | Kéo chân C transistor lên 3,3 V của STM32 |

Đấu theo chữ S / + / − in trên bộ thu, không suy ra thứ tự chân từ màu dây.

```text
MC7RE-V2 (+)  ---------------- BEC +5 V
MC7RE-V2 (-)  ---------------- GND chung với STM32 và BEC

MC7RE-V2 SBUS (S) --[1 kΩ]-- B  NPN
                             E -------- GND chung
STM32 3V3 ---------[10 kΩ]--- C -------- PA10 (USART1 RX)

PA9 (USART1 TX): để hở, không nối vào bộ thu.
```

Kiểm tra đúng chân B/C/E theo datasheet transistor thực tế. Không cấp 5 V vào chân 3V3
hoặc dùng nguồn 3,3 V để cấp bộ thu. Nếu bo có sẵn mạch đảo SBUS, dùng đầu ra của mạch
đó tới PA10 và không đảo thêm một lần nữa. Chân CE/CSN/IRQ của nRF24 cũ không còn sử dụng.
Không ghép song song đầu ra nhiều BEC nếu nhà sản xuất không cho phép.

Cả ba cảm biến MPU6050, BMP280 và QMC5883P đang bắt buộc trong firmware.
Khi mẫu cảm biến không hợp lệ, PID khóa động cơ; watchdog không được làm mới.

Các chân còn lại giữ cách đấu của dự án gốc:

| Chức năng | Chân STM32 |
|---|---|
| SBUS đã đảo | PA10 |
| MPU6050 / BMP280 / QMC5883P: I²C | PB6 SCL, PB7 SDA |
| ESC 1, 2, 3, 4 | PA0, PA1, PA2, PA3 |
| Còi | PB12 |
| Đo pin qua mạch chia áp của dự án | PB0 |

## 2. Gán kênh và thao tác

| Kênh | Chức năng trong bản sửa |
|---|---|
| CH1 AIL | Roll, giới hạn ±20° |
| CH2 ELE | Pitch, giới hạn ±20°; code đảo dấu theo dự án gốc |
| CH3 THR | Lệnh vận tốc đứng: thấp −0,8 m/s, giữa 0, cao +1,0 m/s |
| CH4 RUD | Yaw, giới hạn ±360°/s |
| CH5 | Thấp = khóa động cơ; cao = cho phép thao tác arm |
| CH6 | Cao = yêu cầu hạ khẩn cấp; thấp = tắt yêu cầu |

CH3 điều khiển vận tốc đứng như code gốc, không điều khiển trực tiếp phần trăm công suất.
Sau khi đã cất cánh, vị trí giữa yêu cầu giữ vận tốc đứng bằng 0; điều này phụ thuộc
cảm biến và bộ điều khiển độ cao hoạt động đúng.
CH6 giữ logic gốc: yêu cầu hạ khoảng −0,4 m/s khi không có lệnh leo lên đáng kể.
CH5 khóa động cơ có ưu tiên cao hơn CH6. Kênh 7 không dùng vì tay MC6C chỉ có 6 kênh.

Tắt mixing delta/V-tail, đưa trim về giữa và dùng hành trình đầy đủ khi kiểm tra.
Đảo kênh trên tay phát nếu chiều cần không khớp sau khi kiểm tra lệnh và phản ứng của FC.
Code dùng giá trị SBUS danh định 200 / 1000 / 1800; cần đo lại trên bộ thu thực tế.
Ngưỡng CH5/CH6 cao là >1400; CH5 thấp để mở liên động là <600.

Trình tự arm, ban đầu phải tháo cánh quạt:

1. Đặt CH5 thấp, CH6 thấp và cần ga thấp trước khi cấp nguồn FC.
2. Đợi bộ thu kết nối và cảm biến ổn định. Đảm bảo FC nằm ngang.
3. Đưa CH5 cao. Thao tác này chỉ cho phép arm, chưa làm động cơ quay.
4. Giữ ga sát thấp và cần pitch gần một đầu hành trình trong 2,1 giây;
   góc roll/pitch của FC phải nhỏ hơn 10°. Đây là cử chỉ arm được giữ từ dự án gốc.
5. Khi arm thành công, FC kêu một tiếng và ESC nhận mức chạy chờ 1150 µs.
6. Nâng ga khỏi vùng thấp sẽ chuyển sang trình tự cất cánh gốc.
7. CH5 thấp khóa động cơ và trở về trạng thái chưa arm, xung ESC 1000 µs.

## 3. Ghép đôi và mất tín hiệu

Cấp nguồn bộ thu, khi tay phát đang tắt thì nhấn nút BIND bộ thu rồi bật tay phát;
kiểm tra đèn bộ thu sáng ổn định. Phải xác nhận bộ MC6C của bạn ghép đôi được với
MC7RE-V2 thực tế: manual bộ thu liệt kê 6C-MINI/C7-MINI/8B-MINI, nên không thể xác nhận
tương thích mọi phiên bản MC6C chỉ qua tên gọi.

Code khóa động cơ ngay khi giải mã cờ failsafe hoặc dữ liệu ngoài giới hạn,
và sau 200 ms không có khung khỏe. Khung có cờ lost không kéo dài thời gian sống của link.
Sau mất tín hiệu, CH5 phải được đưa thấp rồi cao và arm lại; kết nối lại không tự bật động cơ.
Chính sách mất link của bản này là **dừng động cơ**, khác với yêu cầu hạ khẩn cấp của code
nRF24 gốc. Khi đang bay, dừng động cơ sẽ làm máy rơi; đây phải là lựa chọn bạn đã kiểm tra
và chấp nhận trước khi bay. CH6 là yêu cầu hạ riêng do người điều khiển chủ động bật.

Giữ failsafe mặc định CH3 khoảng 900 µs. Code có kiểm tra giá trị SBUS thấp bất thường
để bổ sung phát hiện trạng thái này, nhưng phải đo xác nhận cách bộ thu phát SBUS khi mất sóng.
Không đổi failsafe sang giữ nguyên các cần hoặc CH3 giữa 1500 µs: nếu bộ thu tiếp tục
phát khung và không đánh dấu failsafe, FC không thể phân biệt mất sóng với điều khiển thật.

## 4. Biên dịch, nạp và kiểm tra

Mở thư mục dự án bằng PlatformIO. Môi trường `blackpill_f411ce` dùng
ST STM32 20.0.0, STM32duino FreeRTOS 10.3.3; đã bỏ thư viện RF24.

```sh
pio run
pio run -t upload
```

Cấu hình mặc định nạp qua DFU. Nếu dùng ST-Link, đổi `upload_protocol = stlink`
trong `platformio.ini`. Bản firmware cung cấp dành cho STM32F411CE của dự án,
không dùng cho STM32F103/F401 hay bo khác mà chưa chỉnh cấu hình.

Để xem kênh khi kiểm tra, thêm `-D RC_INPUT_DIAGNOSTICS` vào `build_flags`, biên dịch
lại và mở USB Serial Monitor 115200. Dòng `SBUS CH1..6:` hiện số thô mỗi 200 ms,
kèm cờ lost/failsafe. Đây là chế độ kiểm tra trên bàn; bỏ cờ chẩn đoán trước khi bay.

Trước khi gắn cánh quạt, phải kiểm tra trên bàn:

- Khởi động khi CH5 cao, hoặc chưa có bộ thu: bốn ngõ ESC vẫn 1000 µs.
- CH5 thấp → cao: vẫn 1000 µs cho tới khi thực hiện cử chỉ arm.
- Các cần đúng kênh và đúng dấu; giữa gần 1000, hai đầu gần 200/1800.
- Sau arm, CH5 thấp trả cả bốn ngõ về 1000 µs.
- Tắt tay phát khi đã arm: kiểm tra cờ failsafe/dữ liệu bộ thu và FC thực sự khóa.
- Ngắt dây tín hiệu hoặc nguồn bộ thu: FC khóa sau khoảng 200 ms cộng độ trễ lập lịch.
- Bật lại tay phát khi CH5 vẫn cao: động cơ không tự khởi động lại.
- Đo xung PA0–PA3: mức dừng 1000 µs, chu kỳ 20 ms; xác nhận ESC thực tế dừng ở mức đó.

Không có telemetry gửi trở lại MC6C trong bản SBUS này; telemetry cảm biến vẫn được
tính nội bộ. Các PID, lọc cảm biến và thông số pin/động cơ còn dựa trên cấu hình drone
450 mm của tác giả, cần hiệu chỉnh cho máy của bạn.

## 5. Kết quả kiểm tra phần mềm

- Biên dịch firmware BlackPill F411CE thành công; đã sửa bước tích phân PI vận tốc đứng thành 0,01 s ở 100 Hz.
- Đã sửa xử lý mẫu cảm biến lỗi, trả mutex serial và khóa khi cảm biến không hợp lệ.
- Kiểm thử bộ giải mã với 2048 khung, đủ 16 kênh và các ranh giới 11 bit.
- Kiểm thử phục hồi khi sai footer/mất byte, gán kênh, timeout, cờ failsafe/lost,
  khóa khi khởi động/kết nối lại và tràn bộ đếm thời gian.
- Kiểm thử C++ trên máy tính: `g++ -std=c++11 -Wall -Wextra -Werror test/sbus_test.cpp -o sbus_test`, rồi chạy `sbus_test`.
- Chưa xác nhận phần cứng, nhiễu sóng, tải FreeRTOS thực tế hoặc bay thử.

Tài liệu tham khảo:

- [MC7RE-V2: manual gốc của Microzone, bản PDF lưu tại Device.report](https://device.report/m/aa4ff02c962e9d93241ebc12b489c6b3278a3c1c4446142fa1834557c976935a.pdf)
- [MC6C: manual của hãng lưu tại ManualsLib](https://www.manualslib.com/manual/3364329/Microzone-Mc6c.html)

Manual MC7RE-V2 mô tả SBUS 100000 baud, 8E2, 25 byte, chu kỳ 14 ms và mạch đảo NPN;
code dùng cùng giao thức. UART STM32duino tự chọn độ dài phần cứng phù hợp để chứa 8 bit
dữ liệu cộng parity khi dùng `SERIAL_8E2`.
