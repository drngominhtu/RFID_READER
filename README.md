# RFID Reader với Arduino Nano và RC522

## Mô tả
Dự án đọc thẻ RFID sử dụng Arduino Nano và module RC522, có khả năng xuất dữ liệu ra file txt.

## Phần cứng cần thiết
- Arduino Nano
- Module RFID RC522
- Thẻ RFID/NFC
- Dây jumper
- (Tùy chọn) Module SD card để lưu trực tiếp

## Sơ đồ kết nối

| RC522 Pin | Arduino Nano Pin | Mô tả |
|-----------|------------------|-------|
| SDA       | D10              | Slave Select |
| SCK       | D13              | Serial Clock |
| MOSI      | D11              | Master Out Slave In |
| MISO      | D12              | Master In Slave Out |
| IRQ       | Không kết nối    | Interrupt (không sử dụng) |
| GND       | GND              | Ground |
| RST       | D9               | Reset |
| 3.3V      | 3.3V             | Power Supply |

## Cài đặt và chạy

1. **Cài đặt PlatformIO**: Đảm bảo bạn đã cài đặt PlatformIO IDE
2. **Build dự án**: `pio run`
3. **Upload code**: `pio run --target upload`
4. **Mở Serial Monitor**: `pio device monitor`

## Tính năng

### 1. Đọc UID thẻ
- Đọc và hiển thị UID của thẻ RFID
- Nhận diện loại thẻ (MIFARE Classic, MIFARE Ultralight, etc.)

### 2. Đọc dữ liệu từ thẻ MIFARE
- Đọc dữ liệu từ các block của thẻ MIFARE Classic
- Hiển thị dữ liệu dạng HEX và ASCII

### 3. Xuất dữ liệu
- Xuất dữ liệu ra Serial Monitor
- Có thể chuyển hướng output vào file txt
- Tùy chọn lưu trực tiếp vào SD card

## Cách xuất dữ liệu ra file txt

### Phương pháp 1: Sử dụng Serial Monitor
1. Mở terminal/command prompt
2. Chạy lệnh: 
   ```bash
   pio device monitor > rfid_output.txt
   ```
3. Dữ liệu sẽ được lưu vào file `rfid_output.txt`

### Phương pháp 2: Sử dụng module SD card (tùy chọn)
1. Kết nối module SD card:
   - CS pin → D4
   - SCK pin → D13 (chia sẻ với RC522)
   - MOSI pin → D11 (chia sẻ với RC522)
   - MISO pin → D12 (chia sẻ với RC522)
   - VCC → 5V
   - GND → GND

2. Bỏ comment các dòng code liên quan đến SD card trong main.cpp

## Format dữ liệu output

```
UID thẻ: 04 52 C6 32 B2 5C 80
Loại thẻ: MIFARE 1KB
DATA: 12345, 04 52 c6 32 b2 5c 80, MIFARE 1KB
Dữ liệu từ thẻ MIFARE:
Dữ liệu block 4: FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF
ASCII: ................
-----------------------------------
```

## Ghi dữ liệu vào thẻ khác

Để ghi dữ liệu đã đọc vào thẻ khác, bạn có thể:

1. **Lưu dữ liệu**: Sử dụng các hàm đọc để lấy dữ liệu từ thẻ nguồn
2. **Ghi dữ liệu**: Sử dụng hàm `MIFARE_Write()` để ghi vào thẻ đích

Ví dụ code ghi dữ liệu:
```cpp
// Ghi dữ liệu vào block
byte dataBlock[] = {
    0x01, 0x02, 0x03, 0x04, //  1,  2,   3,  4,
    0x05, 0x06, 0x07, 0x08, //  5,  6,   7,  8,
    0x08, 0x09, 0xff, 0x0b, //  9, 10, 255, 12,
    0x0c, 0x0d, 0x0e, 0x0f  // 13, 14,  15, 16
};
status = mfrc522.MIFARE_Write(blockAddr, dataBlock, 16);
```

## Troubleshooting

1. **Không đọc được thẻ**: Kiểm tra kết nối dây, đảm bảo nguồn 3.3V ổn định
2. **Lỗi compile**: Đảm bảo các thư viện đã được cài đặt đúng
3. **Không ghi được vào thẻ**: Kiểm tra key xác thực và quyền ghi của block

## Tài liệu tham khảo
- [MFRC522 Library Documentation](https://github.com/miguelbalboa/rfid)
- [Arduino SPI Library](https://www.arduino.cc/en/reference/SPI)
