#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>

// Định nghĩa các chân kết nối
#define RST_PIN         9           
#define SS_PIN          10          

// Tạo đối tượng MFRC522
MFRC522 mfrc522(SS_PIN, RST_PIN);

// Dữ liệu cần ghi (có thể đọc từ file hoặc thẻ khác)
byte dataToWrite[16] = {
  0x48, 0x65, 0x6C, 0x6C,  // "Hell"
  0x6F, 0x20, 0x52, 0x46,  // "o RF"
  0x49, 0x44, 0x21, 0x00,  // "ID!."
  0x00, 0x00, 0x00, 0x00   // Padding
};

void setup() {
  Serial.begin(9600);
  while (!Serial);
  
  SPI.begin();
  mfrc522.PCD_Init();
  
  Serial.println(F("RFID Writer - Ghi dữ liệu vào thẻ"));
  Serial.println(F("Đặt thẻ gần reader để ghi dữ liệu..."));
}

void loop() {
  // Kiểm tra thẻ mới
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    return;
  }
  
  Serial.println(F("Thẻ được phát hiện!"));
  
  // Hiển thị UID
  Serial.print(F("UID: "));
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
  }
  Serial.println();
  
  // Ghi dữ liệu vào block 4
  if (writeDataToCard(4, dataToWrite)) {
    Serial.println(F("Ghi dữ liệu thành công!"));
    
    // Đọc lại để xác nhận
    readDataFromCard(4);
  } else {
    Serial.println(F("Lỗi ghi dữ liệu!"));
  }
  
  // Dừng giao tiếp với thẻ
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
  
  Serial.println(F("==========================="));
  delay(2000);
}

bool writeDataToCard(byte blockAddr, byte* data) {
  MFRC522::StatusCode status;
  byte trailerBlock = ((blockAddr / 4) * 4) + 3; // Tính trailer block
  
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

void readDataFromCard(byte blockAddr) {
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
    Serial.print(F("Lỗi xác thực đọc: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  
  // Đọc dữ liệu
  status = mfrc522.MIFARE_Read(blockAddr, buffer, &size);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Lỗi đọc: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  
  Serial.print(F("Dữ liệu đã ghi (Block "));
  Serial.print(blockAddr);
  Serial.print(F("): "));
  
  // Hiển thị HEX
  for (byte i = 0; i < 16; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
  Serial.println();
  
  // Hiển thị ASCII
  Serial.print(F("ASCII: "));
  for (byte i = 0; i < 16; i++) {
    if (buffer[i] >= 32 && buffer[i] <= 126) {
      Serial.print((char)buffer[i]);
    } else {
      Serial.print(F("."));
    }
  }
  Serial.println();
}
