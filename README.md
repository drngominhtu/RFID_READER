# RFID Reader/Writer System

## Mô tả
Hệ thống đọc và ghi thẻ RFID hoàn chỉnh với Arduino Nano và RC522, hỗ trợ:
- ✅ Đọc thẻ RFID và lưu dữ liệu
- ✅ Ghi dữ liệu vào thẻ trống khác  
- ✅ Xuất dữ liệu ra file TXT
- ✅ Giao diện menu điều khiển
- ✅ Script Python tự động

## Phần cứng cần thiết
- Arduino Nano
- Module RFID RC522
- Thẻ RFID/NFC (MIFARE Classic khuyến nghị)
- Dây jumper
- Cáp USB

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

### 1. Upload code lên Arduino
```bash
# Nếu có PlatformIO
pio run --target upload

# Nếu dùng Arduino IDE
# Mở src/main.cpp và upload
```

### 2. Chạy chương trình

#### Cách 1: Sử dụng Batch File (Dễ nhất)
```bash
# Double-click vào file
run_rfid.bat
```

#### Cách 2: Sử dụng Python Manager
```bash
python rfid_manager.py
```

#### Cách 3: Serial Monitor trực tiếp
```bash
# Windows
mode COM3 BAUD=9600 PARITY=N data=8 stop=1
copy COM3 rfid_data.txt

# Arduino IDE Serial Monitor
# Tools -> Serial Monitor
```

## Hướng dẫn sử dụng

### 🔍 Đọc thẻ và lưu dữ liệu

1. **Khởi động chương trình**
2. **Chọn chế độ đọc**: Gõ `1` hoặc `read`
3. **Đặt thẻ gần reader**
4. **Dữ liệu sẽ được đọc và lưu tự động**

### ✍️ Ghi dữ liệu vào thẻ trống

1. **Đọc thẻ nguồn trước** (bước trên)
2. **Chọn chế độ ghi**: Gõ `2` hoặc `write`  
3. **Đặt thẻ TRỐNG gần reader**
4. **Dữ liệu sẽ được ghi tự động**

### 📁 Xuất dữ liệu ra file TXT

#### Phương pháp 1: Menu Export
1. Gõ `4` hoặc `export`
2. Copy dữ liệu hiển thị vào file .txt

#### Phương pháp 2: Python Manager
1. Chạy `python rfid_manager.py`
2. Chọn option 3
3. File sẽ được tạo tự động

#### Phương pháp 3: Chuyển hướng Serial
```bash
# PowerShell
Get-Content COM3 | Out-File rfid_data.txt

# Command Prompt  
copy COM3 rfid_data.txt
```

## Menu điều khiển Arduino

Khi kết nối Serial Monitor, bạn có thể sử dụng các lệnh:

| Lệnh | Chức năng |
|------|-----------|
| `1` hoặc `read` | Chế độ đọc thẻ |
| `2` hoặc `write` | Chế độ ghi thẻ |
| `3` hoặc `show` | Hiển thị dữ liệu đã lưu |
| `4` hoặc `export` | Xuất dữ liệu ra format TXT |
| `5` hoặc `menu` | Hiển thị menu |

## Format dữ liệu

### Serial Output
```
--- ĐỌC THẺ ---
UID: 04 52 C6 32 B2 5C 80
Loại thẻ: MIFARE 1KB
Dữ liệu HEX: 48 65 6C 6C 6F 20 52 46 49 44 21 00 00 00 00 00
Dữ liệu ASCII: Hello RFID!.....
✓ Đọc dữ liệu thành công!
--- KẾT THÚC ĐỌC ---
```

### File TXT Export
```
# RFID Card Data Export
# Timestamp: 12345678

UID=0452C632B25C80
TYPE=MIFARE 1KB
DATA_HEX=48656C6C6F20524649442100000000000000
DATA_ASCII=Hello RFID!.....
```

## Quy trình làm việc hoàn chỉnh

### 🔄 Copy thẻ RFID

1. **Chuẩn bị**: Thẻ nguồn + Thẻ trống
2. **Đọc thẻ nguồn**: 
   - Chạy chương trình
   - Chọn chế độ đọc (`1`)
   - Đặt thẻ nguồn gần reader
   - Đợi thông báo "✓ Đọc dữ liệu thành công!"

3. **Ghi vào thẻ đích**:
   - Chọn chế độ ghi (`2`) 
   - Đặt thẻ trống gần reader
   - Đợi thông báo "✓ Ghi dữ liệu thành công!"

4. **Kiểm tra**: Đọc lại thẻ đích để xác nhận

### 💾 Lưu trữ dữ liệu

1. **Đọc thẻ** (như trên)
2. **Xuất file**: Chọn `4` để export
3. **Lưu file**: Copy nội dung vào file .txt
4. **Backup**: Lưu file an toàn

## Troubleshooting

### ❌ Lỗi thường gặp

**Không đọc được thẻ:**
- ✅ Kiểm tra kết nối dây
- ✅ Đảm bảo nguồn 3.3V ổn định  
- ✅ Thẻ đặt gần reader (1-3cm)
- ✅ Thử thẻ khác

**Lỗi ghi thẻ:**
- ✅ Thẻ phải là MIFARE Classic
- ✅ Thẻ chưa bị khóa
- ✅ Đã đọc dữ liệu nguồn trước

**Lỗi Serial:**
- ✅ COM port đúng
- ✅ Baud rate 9600
- ✅ Driver Arduino đã cài

### 🔧 Debug

```bash
# Kiểm tra COM port
mode

# Test Serial connection  
echo "5" > COM3

# Monitor real-time
python -c "import serial; ser=serial.Serial('COM3',9600); [print(ser.readline()) for _ in range(10)]"
```

## Files trong dự án

```
RFID_READER/
├── src/
│   └── main.cpp              # Code Arduino chính
├── examples/
│   └── write_example.cpp     # Ví dụ ghi thẻ
├── platformio.ini            # Cấu hình PlatformIO
├── rfid_manager.py          # Python manager
├── run_rfid.bat             # Batch file chạy nhanh
└── README.md                # Hướng dẫn này
```

## Tính năng nâng cao

### 🔐 Bảo mật
- Sử dụng key tùy chỉnh thay vì key mặc định
- Mã hóa dữ liệu trước khi ghi
- Checksum validation

### 📊 Logging
- Lưu lịch sử đọc/ghi
- Timestamp chi tiết
- Error tracking

### 🌐 Mở rộng
- Kết nối WiFi/Bluetooth
- Database integration
- Web interface

## Liên hệ & Hỗ trợ

Nếu gặp vấn đề, hãy kiểm tra:
1. Kết nối phần cứng
2. Thẻ RFID tương thích
3. COM port và driver
4. Version thư viện

**Thư viện sử dụng:**
- MFRC522 by Miguel Balboa
- Arduino SPI Library
- Python pySerial
