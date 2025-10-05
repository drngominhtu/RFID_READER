import serial
import time
import sys

class RFIDManager:
    def __init__(self, port='COM3', baudrate=9600):
        """
        Khởi tạo kết nối serial với Arduino
        """
        try:
            self.ser = serial.Serial(port, baudrate, timeout=1)
            time.sleep(2)  # Đợi Arduino khởi động
            print(f"Kết nối thành công với {port}")
        except Exception as e:
            print(f"Lỗi kết nối: {e}")
            sys.exit(1)
    
    def send_command(self, command):
        """
        Gửi lệnh đến Arduino
        """
        self.ser.write((command + '\n').encode())
        time.sleep(0.5)
    
    def read_response(self, timeout=5):
        """
        Đọc phản hồi từ Arduino
        """
        start_time = time.time()
        response = ""
        
        while time.time() - start_time < timeout:
            if self.ser.in_waiting > 0:
                response += self.ser.read(self.ser.in_waiting).decode('utf-8', errors='ignore')
            time.sleep(0.1)
        
        return response
    
    def read_card_to_file(self, filename='rfid_data.txt'):
        """
        Đọc thẻ và lưu vào file
        """
        print("Chuyển sang chế độ đọc thẻ...")
        self.send_command('1')  # Chế độ đọc
        
        print("Đặt thẻ gần reader...")
        print("Đang chờ dữ liệu...")
        
        # Đọc dữ liệu trong 30 giây
        start_time = time.time()
        data_collected = ""
        
        while time.time() - start_time < 30:
            response = self.read_response(1)
            if response:
                data_collected += response
                print(response, end='')
                
                # Kiểm tra xem đã đọc xong thẻ chưa
                if "--- KẾT THÚC ĐỌC ---" in response:
                    break
        
        if data_collected:
            # Lưu vào file
            with open(filename, 'w', encoding='utf-8') as f:
                f.write(f"# RFID Data Export - {time.strftime('%Y-%m-%d %H:%M:%S')}\n")
                f.write(data_collected)
            
            print(f"\n✓ Dữ liệu đã được lưu vào {filename}")
            return True
        else:
            print("✗ Không có dữ liệu nào được đọc!")
            return False
    
    def write_card_from_data(self):
        """
        Ghi dữ liệu đã lưu vào thẻ trống
        """
        print("Chuyển sang chế độ ghi thẻ...")
        self.send_command('2')  # Chế độ ghi
        
        print("Đặt thẻ TRỐNG gần reader...")
        print("Đang chờ ghi dữ liệu...")
        
        # Đọc kết quả ghi trong 30 giây
        start_time = time.time()
        
        while time.time() - start_time < 30:
            response = self.read_response(1)
            if response:
                print(response, end='')
                
                # Kiểm tra xem đã ghi xong chưa
                if "--- KẾT THÚC GHI ---" in response:
                    if "✓ Ghi dữ liệu thành công!" in response:
                        print("\n✓ Ghi thẻ thành công!")
                        return True
                    else:
                        print("\n✗ Ghi thẻ thất bại!")
                        return False
        
        print("\n⏱ Timeout - Không có phản hồi!")
        return False
    
    def export_to_txt(self, filename='rfid_export.txt'):
        """
        Xuất dữ liệu đã lưu ra file txt có định dạng
        """
        print("Xuất dữ liệu ra file txt...")
        self.send_command('4')  # Lệnh export
        
        response = self.read_response(10)
        
        if response and "XUẤT DỮ LIỆU RA FILE TXT" in response:
            # Tìm phần dữ liệu giữa các dấu =
            lines = response.split('\n')
            export_data = []
            capturing = False
            
            for line in lines:
                if "=====================================" in line and not capturing:
                    capturing = True
                    continue
                elif "=====================================" in line and capturing:
                    break
                elif capturing:
                    export_data.append(line)
            
            if export_data:
                with open(filename, 'w', encoding='utf-8') as f:
                    f.write(f"# RFID Data Export - {time.strftime('%Y-%m-%d %H:%M:%S')}\n")
                    for line in export_data:
                        f.write(line + '\n')
                
                print(f"✓ Dữ liệu đã được xuất ra {filename}")
                return True
        
        print("✗ Không có dữ liệu để xuất!")
        return False
    
    def show_menu(self):
        """
        Hiển thị menu điều khiển
        """
        print("\n" + "="*50)
        print("           RFID MANAGER - PYTHON CONTROL")
        print("="*50)
        print("1. Đọc thẻ và lưu vào file")
        print("2. Ghi dữ liệu vào thẻ trống")
        print("3. Xuất dữ liệu ra file txt")
        print("4. Hiển thị dữ liệu đã lưu")
        print("5. Thoát")
        print("="*50)
    
    def run(self):
        """
        Chạy chương trình chính
        """
        print("Khởi động RFID Manager...")
        time.sleep(2)
        
        while True:
            self.show_menu()
            choice = input("Nhập lựa chọn (1-5): ").strip()
            
            if choice == '1':
                filename = input("Nhập tên file để lưu (rfid_data.txt): ").strip()
                if not filename:
                    filename = 'rfid_data.txt'
                self.read_card_to_file(filename)
            
            elif choice == '2':
                self.write_card_from_data()
            
            elif choice == '3':
                filename = input("Nhập tên file export (rfid_export.txt): ").strip()
                if not filename:
                    filename = 'rfid_export.txt'
                self.export_to_txt(filename)
            
            elif choice == '4':
                print("Hiển thị dữ liệu đã lưu...")
                self.send_command('3')
                response = self.read_response(5)
                print(response)
            
            elif choice == '5':
                print("Thoát chương trình...")
                break
            
            else:
                print("Lựa chọn không hợp lệ!")
            
            input("\nNhấn Enter để tiếp tục...")
    
    def close(self):
        """
        Đóng kết nối serial
        """
        if hasattr(self, 'ser'):
            self.ser.close()
            print("Đã đóng kết nối serial.")

if __name__ == "__main__":
    # Thay đổi COM port tương ứng với Arduino của bạn
    port = input("Nhập COM port (VD: COM3): ").strip().upper()
    if not port:
        port = 'COM3'
    
    try:
        manager = RFIDManager(port)
        manager.run()
    except KeyboardInterrupt:
        print("\nChương trình bị ngắt bởi người dùng.")
    finally:
        if 'manager' in locals():
            manager.close()
