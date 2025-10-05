#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <EEPROM.h>

// Định nghĩa các chân kết nối
#define RST_PIN         9           // Chân RST
#define SS_PIN          10          // Chân SDA/SS
#define MODE_BUTTON     2           // Nút chọn chế độ (tùy chọn)

// Tạo đối tượng MFRC522
MFRC522 mfrc522(SS_PIN, RST_PIN);

// Chế độ hoạt động
enum Mode {
  READ_MODE = 0,    // Chế độ đọc thẻ và lưu dữ liệu
  WRITE_MODE = 1    // Chế độ ghi dữ liệu vào thẻ
};

Mode currentMode = READ_MODE;

// Cấu trúc dữ liệu để lưu thông tin thẻ
struct CardData {
  byte uid[10];           // UID thẻ (tối đa 10 bytes)
  byte uidSize;           // Kích thước UID
  byte data[16];          // Dữ liệu từ block 4
  char cardType[20];      // Loại thẻ
  bool isValid;           // Dữ liệu có hợp lệ không
};

CardData savedCardData;   // Dữ liệu thẻ đã lưu

void setup() {
  // Khởi tạo Serial communication
  Serial.begin(9600);
  while (!Serial);
  
  Serial.println(F("=== RFID READER/WRITER ==="));
  Serial.println(F("RC522 với Arduino Nano"));
  Serial.println(F("=========================="));
  
  // Khởi tạo SPI bus
  SPI.begin();
  
  // Khởi tạo MFRC522
  mfrc522.PCD_Init();
  delay(4);
  
  // Hiển thị thông tin về reader
  mfrc522.PCD_DumpVersionToSerial();
  
  // Khởi tạo dữ liệu lưu trữ
  savedCardData.isValid = false;
  
  // Hiển thị menu
  showMenu();
}

void loop() {
  // Kiểm tra lệnh từ Serial
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    
    if (command == "1" || command.equalsIgnoreCase("read")) {
      currentMode = READ_MODE;
      Serial.println(F("\n--- CHẾ ĐỘ ĐỌC THẺ ---"));
      Serial.println(F("Đặt thẻ gần reader để đọc dữ liệu..."));
    }
    else if (command == "2" || command.equalsIgnoreCase("write")) {
      currentMode = WRITE_MODE;
      Serial.println(F("\n--- CHẾ ĐỘ GHI THẺ ---"));
      if (savedCardData.isValid) {
        Serial.println(F("Đặt thẻ TRỐNG gần reader để ghi dữ liệu..."));
        printSavedData();
      } else {
        Serial.println(F("CẢNH BÁO: Chưa có dữ liệu để ghi!"));
        Serial.println(F("Hãy đọc thẻ trước khi ghi."));
      }
    }
    else if (command == "3" || command.equalsIgnoreCase("show")) {
      showSavedData();
    }
    else if (command == "4" || command.equalsIgnoreCase("export")) {
      exportDataToTxt();
    }
    else if (command == "5" || command.equalsIgnoreCase("menu")) {
      showMenu();
    }
    else {
      Serial.println(F("Lệnh không hợp lệ! Gõ '5' để xem menu."));
    }
  }
  
  // Xử lý thẻ RFID
  handleRFID();
  
  delay(100);
}

void showMenu() {
  Serial.println(F("\n========== MENU =========="));
  Serial.println(F("1 (read)   - Chế độ đọc thẻ"));
  Serial.println(F("2 (write)  - Chế độ ghi thẻ"));
  Serial.println(F("3 (show)   - Hiển thị dữ liệu đã lưu"));
  Serial.println(F("4 (export) - Xuất dữ liệu ra file TXT"));
  Serial.println(F("5 (menu)   - Hiển thị menu"));
  Serial.println(F("=========================="));
  Serial.println(F("Nhập lệnh:"));
}

void handleRFID() {
  // Kiểm tra xem có thẻ mới không
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Chọn một trong những thẻ
  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  if (currentMode == READ_MODE) {
    readCard();
  } else if (currentMode == WRITE_MODE) {
    writeCard();
  }
  
  // Dừng đọc thẻ hiện tại
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
  
  delay(2000); // Đợi 2 giây trước khi đọc thẻ tiếp theo
}

void readCard() {
  Serial.println(F("\n--- ĐỌC THẺ ---"));
  
  // Đọc UID
  Serial.print(F("UID: "));
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (mfrc522.uid.uidByte[i] < 0x10) Serial.print(F("0"));
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    if (i < mfrc522.uid.size - 1) Serial.print(F(" "));
    
    // Lưu UID
    savedCardData.uid[i] = mfrc522.uid.uidByte[i];
  }
  savedCardData.uidSize = mfrc522.uid.size;
  Serial.println();
  
  // Đọc loại thẻ
  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  String typeString = mfrc522.PICC_GetTypeName(piccType);
  typeString.toCharArray(savedCardData.cardType, 20);
  
  Serial.print(F("Loại thẻ: "));
  Serial.println(typeString);
  
  // Đọc dữ liệu từ block 4 (nếu là MIFARE)
  if (piccType == MFRC522::PICC_TYPE_MIFARE_MINI ||
      piccType == MFRC522::PICC_TYPE_MIFARE_1K ||
      piccType == MFRC522::PICC_TYPE_MIFARE_4K) {
    
    if (readMifareBlock(4, savedCardData.data)) {
      savedCardData.isValid = true;
      Serial.println(F("✓ Đọc dữ liệu thành công!"));
      
      // Hiển thị dữ liệu
      Serial.print(F("Dữ liệu HEX: "));
      for (byte i = 0; i < 16; i++) {
        if (savedCardData.data[i] < 0x10) Serial.print(F("0"));
        Serial.print(savedCardData.data[i], HEX);
        Serial.print(F(" "));
      }
      Serial.println();
      
      Serial.print(F("Dữ liệu ASCII: "));
      for (byte i = 0; i < 16; i++) {
        if (savedCardData.data[i] >= 32 && savedCardData.data[i] <= 126) {
          Serial.print((char)savedCardData.data[i]);
        } else {
          Serial.print(F("."));
        }
      }
      Serial.println();
      
    } else {
      Serial.println(F("✗ Lỗi đọc dữ liệu từ thẻ!"));
      savedCardData.isValid = false;
    }
  } else {
    // Với thẻ không phải MIFARE, chỉ lưu UID
    memset(savedCardData.data, 0, 16);
    savedCardData.isValid = true;
    Serial.println(F("✓ Đọc UID thành công!"));
  }
  
  Serial.println(F("--- KẾT THÚC ĐỌC ---"));
}

void writeCard() {
  if (!savedCardData.isValid) {
    Serial.println(F("✗ Không có dữ liệu để ghi!"));
    return;
  }
  
  Serial.println(F("\n--- GHI THẺ ---"));
  
  // Hiển thị UID thẻ hiện tại
  Serial.print(F("UID thẻ đích: "));
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (mfrc522.uid.uidByte[i] < 0x10) Serial.print(F("0"));
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    if (i < mfrc522.uid.size - 1) Serial.print(F(" "));
  }
  Serial.println();
  
  // Kiểm tra loại thẻ
  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
      piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
      piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("✗ Thẻ không hỗ trợ ghi dữ liệu!"));
    return;
  }
  
  // Ghi dữ liệu vào block 4
  if (writeMifareBlock(4, savedCardData.data)) {
    Serial.println(F("✓ Ghi dữ liệu thành công!"));
    
    // Đọc lại để xác nhận
    byte verifyData[16];
    if (readMifareBlock(4, verifyData)) {
      bool isMatch = true;
      for (byte i = 0; i < 16; i++) {
        if (verifyData[i] != savedCardData.data[i]) {
          isMatch = false;
          break;
        }
      }
      
      if (isMatch) {
        Serial.println(F("✓ Xác nhận dữ liệu chính xác!"));
      } else {
        Serial.println(F("⚠ Cảnh báo: Dữ liệu không khớp!"));
      }
    }
  } else {
    Serial.println(F("✗ Lỗi ghi dữ liệu!"));
  }
  
  Serial.println(F("--- KẾT THÚC GHI ---"));
}

bool readMifareBlock(byte blockAddr, byte* data) {
  MFRC522::StatusCode status;
  byte buffer[18];
  byte size = sizeof(buffer);
  byte trailerBlock = ((blockAddr / 4) * 4) + 3;
  
  // Key mặc định
  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
  
  // Xác thực
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 
                                   trailerBlock, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Lỗi xác thực: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return false;
  }
  
  // Đọc dữ liệu
  status = mfrc522.MIFARE_Read(blockAddr, buffer, &size);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Lỗi đọc: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return false;
  }
  
  // Copy dữ liệu
  memcpy(data, buffer, 16);
  return true;
}

bool writeMifareBlock(byte blockAddr, byte* data) {
  MFRC522::StatusCode status;
  byte trailerBlock = ((blockAddr / 4) * 4) + 3;
  
  // Key mặc định
  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
  
  // Xác thực
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 
                                   trailerBlock, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Lỗi xác thực: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return false;
  }
  
  // Ghi dữ liệu
  status = mfrc522.MIFARE_Write(blockAddr, data, 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Lỗi ghi: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return false;
  }
  
  return true;
}

void showSavedData() {
  Serial.println(F("\n--- DỮ LIỆU ĐÃ LƯU ---"));
  if (!savedCardData.isValid) {
    Serial.println(F("Chưa có dữ liệu!"));
    return;
  }
  
  printSavedData();
}

void printSavedData() {
  Serial.print(F("UID: "));
  for (byte i = 0; i < savedCardData.uidSize; i++) {
    if (savedCardData.uid[i] < 0x10) Serial.print(F("0"));
    Serial.print(savedCardData.uid[i], HEX);
    if (i < savedCardData.uidSize - 1) Serial.print(F(" "));
  }
  Serial.println();
  
  Serial.print(F("Loại thẻ: "));
  Serial.println(savedCardData.cardType);
  
  Serial.print(F("Dữ liệu HEX: "));
  for (byte i = 0; i < 16; i++) {
    if (savedCardData.data[i] < 0x10) Serial.print(F("0"));
    Serial.print(savedCardData.data[i], HEX);
    Serial.print(F(" "));
  }
  Serial.println();
  
  Serial.print(F("Dữ liệu ASCII: "));
  for (byte i = 0; i < 16; i++) {
    if (savedCardData.data[i] >= 32 && savedCardData.data[i] <= 126) {
      Serial.print((char)savedCardData.data[i]);
    } else {
      Serial.print(F("."));
    }
  }
  Serial.println();
}

void exportDataToTxt() {
  Serial.println(F("\n--- XUẤT DỮ LIỆU RA FILE TXT ---"));
  if (!savedCardData.isValid) {
    Serial.println(F("Chưa có dữ liệu để xuất!"));
    return;
  }
  
  Serial.println(F("Sao chép dữ liệu dưới đây vào file .txt:"));
  Serial.println(F("====================================="));
  
  // Header
  Serial.println(F("# RFID Card Data Export"));
  Serial.print(F("# Timestamp: "));
  Serial.println(millis());
  Serial.println();
  
  // UID
  Serial.print(F("UID="));
  for (byte i = 0; i < savedCardData.uidSize; i++) {
    if (savedCardData.uid[i] < 0x10) Serial.print(F("0"));
    Serial.print(savedCardData.uid[i], HEX);
  }
  Serial.println();
  
  // Card Type
  Serial.print(F("TYPE="));
  Serial.println(savedCardData.cardType);
  
  // Data in HEX
  Serial.print(F("DATA_HEX="));
  for (byte i = 0; i < 16; i++) {
    if (savedCardData.data[i] < 0x10) Serial.print(F("0"));
    Serial.print(savedCardData.data[i], HEX);
  }
  Serial.println();
  
  // Data in ASCII
  Serial.print(F("DATA_ASCII="));
  for (byte i = 0; i < 16; i++) {
    if (savedCardData.data[i] >= 32 && savedCardData.data[i] <= 126) {
      Serial.print((char)savedCardData.data[i]);
    } else {
      Serial.print(F("."));
    }
  }
  Serial.println();
  
  Serial.println(F("====================================="));
  Serial.println(F("Để lưu vào file, sử dụng lệnh terminal:"));
  Serial.println(F("arduino-cli monitor -p COM_PORT > rfid_data.txt"));
  Serial.println(F("hoặc copy thủ công vào file txt."));
}