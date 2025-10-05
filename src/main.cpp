#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>

// Định nghĩa các chân kết nối
#define RST_PIN         9
#define SS_PIN          10

// Tạo đối tượng MFRC522
MFRC522 mfrc522(SS_PIN, RST_PIN);

// Cấu trúc dữ liệu để lưu thông tin thẻ
struct CardData {
  byte uid[10];
  byte uidSize;
  byte data[16];
  bool isValid;
};

CardData savedCardData;
bool writeMode = false;

// Khai báo các hàm
void readSourceCard();
void writeTargetCard();
bool readMifareBlock(byte blockAddr, byte* data);
bool writeMifareBlock(byte blockAddr, byte* data);
void exportToFile();

void setup() {
  Serial.begin(9600);
  while (!Serial);
  
  Serial.println("=== RFID READER/WRITER ===");
  Serial.println("Chế độ tự động: Đọc thẻ đầu tiên, sau đó ghi vào thẻ thứ hai");
  
  SPI.begin();
  mfrc522.PCD_Init();
  delay(100);
  
  // Kiểm tra firmware RC522
  byte version = mfrc522.PCD_ReadRegister(MFRC522::VersionReg);
  Serial.print("RC522 Firmware: 0x");
  Serial.print(version, HEX);
  if (version == 0x00 || version == 0xFF) {
    Serial.println(" - FAIL! Kiểm tra kết nối!");
  } else {
    Serial.println(" - OK!");
  }
  
  savedCardData.isValid = false;
  
  Serial.println("Đặt thẻ NGUỒN gần reader để đọc...");
}

void loop() {
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  if (!writeMode && !savedCardData.isValid) {
    // Chế độ đọc thẻ nguồn
    readSourceCard();
  } else if (writeMode && savedCardData.isValid) {
    // Chế độ ghi thẻ đích
    writeTargetCard();
  }
  
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
  delay(2000);
}

void readSourceCard() {
  Serial.println("\n--- ĐỌC THẺ NGUỒN ---");
  
  // Đọc UID
  Serial.print("UID: ");
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (mfrc522.uid.uidByte[i] < 0x10) Serial.print("0");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    if (i < mfrc522.uid.size - 1) Serial.print(" ");
    
    savedCardData.uid[i] = mfrc522.uid.uidByte[i];
  }
  savedCardData.uidSize = mfrc522.uid.size;
  Serial.println();
  
  // Đọc dữ liệu từ block 4
  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  if (piccType == MFRC522::PICC_TYPE_MIFARE_MINI ||
      piccType == MFRC522::PICC_TYPE_MIFARE_1K ||
      piccType == MFRC522::PICC_TYPE_MIFARE_4K) {
    
    if (readMifareBlock(4, savedCardData.data)) {
      savedCardData.isValid = true;
      writeMode = true;
      
      Serial.print("Dữ liệu: ");
      for (byte i = 0; i < 16; i++) {
        if (savedCardData.data[i] < 0x10) Serial.print("0");
        Serial.print(savedCardData.data[i], HEX);
        Serial.print(" ");
      }
      Serial.println();
      
      Serial.println("✓ Đọc thành công!");
      Serial.println("BỎ thẻ cũ và đặt thẻ ĐÍCH gần reader để ghi...");
    } else {
      Serial.println("✗ Lỗi đọc dữ liệu!");
    }
  } else {
    Serial.println("✗ Thẻ không hỗ trợ!");
  }
}

void writeTargetCard() {
  Serial.println("\n--- GHI THẺ ĐÍCH ---");
  
  Serial.print("UID thẻ đích: ");
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (mfrc522.uid.uidByte[i] < 0x10) Serial.print("0");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    if (i < mfrc522.uid.size - 1) Serial.print(" ");
  }
  Serial.println();
  
  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
      piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
      piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println("✗ Thẻ đích không hỗ trợ ghi!");
    return;
  }
  
  if (writeMifareBlock(4, savedCardData.data)) {
    Serial.println("✓ Ghi dữ liệu thành công!");
    
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
        Serial.println("✓ Xác nhận thành công - Dữ liệu khớp!");
        Serial.println("=== COPY THẺ HOÀN TẤT ===");
        
        // Export dữ liệu ra file
        exportToFile();
        
        // Reset để copy thẻ mới
        savedCardData.isValid = false;
        writeMode = false;
        Serial.println("\nSẵn sàng copy thẻ mới...");
        Serial.println("Đặt thẻ NGUỒN gần reader để đọc...");
      } else {
        Serial.println("⚠ Cảnh báo: Dữ liệu không khớp!");
      }
    }
  } else {
    Serial.println("✗ Lỗi ghi dữ liệu!");
  }
}

bool readMifareBlock(byte blockAddr, byte* data) {
  MFRC522::StatusCode status;
  byte buffer[18];
  byte size = sizeof(buffer);
  byte trailerBlock = ((blockAddr / 4) * 4) + 3;
  
  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
  
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 
                                   trailerBlock, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    return false;
  }
  
  status = mfrc522.MIFARE_Read(blockAddr, buffer, &size);
  if (status != MFRC522::STATUS_OK) {
    return false;
  }
  
  memcpy(data, buffer, 16);
  return true;
}

bool writeMifareBlock(byte blockAddr, byte* data) {
  MFRC522::StatusCode status;
  byte trailerBlock = ((blockAddr / 4) * 4) + 3;
  
  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
  
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 
                                   trailerBlock, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    return false;
  }
  
  status = mfrc522.MIFARE_Write(blockAddr, data, 16);
  if (status != MFRC522::STATUS_OK) {
    return false;
  }
  
  return true;
}

void exportToFile() {
  Serial.println("\n=== EXPORT DATA TO FILE ===");
  Serial.print("UID=");
  for (byte i = 0; i < savedCardData.uidSize; i++) {
    if (savedCardData.uid[i] < 0x10) Serial.print("0");
    Serial.print(savedCardData.uid[i], HEX);
  }
  Serial.println();
  
  Serial.print("DATA=");
  for (byte i = 0; i < 16; i++) {
    if (savedCardData.data[i] < 0x10) Serial.print("0");
    Serial.print(savedCardData.data[i], HEX);
  }
  Serial.println();
  Serial.println("============================");
}