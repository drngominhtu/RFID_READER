@echo off
echo ================================
echo    RFID READER/WRITER SYSTEM
echo ================================
echo.

REM Kiểm tra Python
python --version >nul 2>&1
if errorlevel 1 (
    echo Python không được cài đặt!
    echo Vui lòng cài đặt Python từ https://python.org
    pause
    exit /b 1
)

REM Cài đặt thư viện cần thiết
echo Kiểm tra và cài đặt thư viện...
pip install pyserial >nul 2>&1

echo.
echo Chọn chế độ hoạt động:
echo 1. Chạy Python Manager (Giao diện menu)
echo 2. Monitor Arduino Serial trực tiếp
echo 3. Đọc thẻ và lưu vào file txt
echo 4. Thoát
echo.

set /p choice="Nhập lựa chọn (1-4): "

if "%choice%"=="1" (
    echo Khởi động Python RFID Manager...
    python rfid_manager.py
) else if "%choice%"=="2" (
    set /p port="Nhập COM port (VD: COM3): "
    echo Mở Serial Monitor cho %port%...
    echo Nhấn Ctrl+C để thoát
    python -c "import serial; import time; ser=serial.Serial('%port%', 9600); [print(ser.readline().decode('utf-8', errors='ignore'), end='') for _ in iter(int, 1)]"
) else if "%choice%"=="3" (
    set /p port="Nhập COM port (VD: COM3): "
    set /p filename="Nhập tên file output (rfid_output.txt): "
    if "%filename%"=="" set filename=rfid_output.txt
    echo Đọc dữ liệu từ %port% và lưu vào %filename%...
    echo Nhấn Ctrl+C để dừng
    python -c "import serial; import time; ser=serial.Serial('%port%', 9600); f=open('%filename%', 'w'); ser.write(b'1\n'); [f.write(ser.readline().decode('utf-8', errors='ignore')) or print(ser.readline().decode('utf-8', errors='ignore'), end='') for _ in iter(int, 1)]"
) else if "%choice%"=="4" (
    echo Thoát...
    exit /b 0
) else (
    echo Lựa chọn không hợp lệ!
)

echo.
pause
