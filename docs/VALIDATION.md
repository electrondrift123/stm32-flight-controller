# Kết quả kiểm chứng phần mềm

Ngày bàn giao: 07/10/2026. Bản nền: `5e0b0876b424970ebf454aec68df33d2ef8dc9d6`.

## Biên dịch firmware

- PlatformIO Core 6.2.0; ST STM32 20.0.0; Arduino STM32 3.0.0.
- FreeRTOS 10.3.3; ARM GCC 12.3.1; mục tiêu BlackPill STM32F411CE.
- Lệnh: `pio run -e blackpill_f411ce`.
- Kết quả: **SUCCESS**.
- RAM tĩnh: 7700 / 131072 byte; Flash: 63832 / 524288 byte.
- Kích thước firmware.bin gồm DFU suffix: 64312 byte.
- SHA256: `c7a29455b368502ddfda8c9fee9b4e9cf4bc6faedfc01584e55335899ad61a59`.

## Kiểm thử host

- Lệnh: `python tools/run_sbus_tests.py --compiler /path/to/g++`.
- GCC 5.1.0 trên Windows, C++11, `-Wall -Wextra -Werror`.
- Kết quả: **PASS: 2048 SBUS frames, resync, mapping, timeout, failsafe, re-enable, clock wrap**.
- Test dùng trực tiếp `SbusParser` và `RcInput` trong firmware.
- Đã kiểm tra đủ 16 kênh, giải mã các vị trí bit, framing, gán lệnh, cờ lost/failsafe,
  startup CH5 cao, timeout, nối lại, dữ liệu ngoài giới hạn và tràn millis.

## Những điều chưa được xác nhận

- Chưa nạp hoặc chạy trên bo STM32, bộ thu, ESC và máy bay thực tế.
- Chưa đo nguồn, SBUS, xung ESC, thời gian thực thi hoặc stack/heap FreeRTOS.
- Chưa ghép đôi MC6C với MC7RE-V2 hoặc đo hành vi failsafe khi tắt tay phát.
- Chưa hiệu chỉnh cảm biến, thang ADC pin hoặc PID cho máy mới.
- Workflow đã thêm nhưng kết quả CI trên GitHub phải kiểm tra riêng sau khi push.
- Không có PCB KiCad/Gerber mới hoặc số liệu bay.

Bảng nghiệm thu phần cứng nằm trong báo cáo và hướng dẫn. Biên dịch thành công
và bộ test host đạt không thay thế các phép đo này.
