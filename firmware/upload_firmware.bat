@echo off
setlocal EnableDelayedExpansion

echo ==========================================
echo  All-Seeing-Eye Firmware Uploader (Arduino CLI)
echo ==========================================

:: 1. LOCATE ARDUINO CLI
set "CLI_PATH=C:\Users\CJ\AppData\Local\Programs\Arduino IDE\resources\app\lib\backend\resources\arduino-cli.exe"

if not exist "!CLI_PATH!" (
    echo [ERROR] Could not find arduino-cli.exe at:
    echo !CLI_PATH!
    pause
    exit /b 1
)

:: 2. CONFIGURATION
:: ESP32-S3 Settings for N16R8
set "FQBN=esp32:esp32:esp32s3:CDCOnBoot=cdc,FlashSize=16M,PSRAM=opi,USBMode=hwcdc,FlashMode=qio,PartitionScheme=app3M_fat9M_16MB"
set "SKETCH_DIR=%~dp0AllSeeingEye"

:: 2.5 EMBED WEB ASSETS
echo.
echo [INFO] Packing Web Assets...
python "%~dp0pack_web.py"
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Web packing failed.
    exit /b 1
)

echo.
echo [INFO] Compiling sketch...
echo Board: !FQBN!
echo.

:: 3. COMPILE
"!CLI_PATH!" compile --fqbn !FQBN! "!SKETCH_DIR!"
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [ERROR] Compilation failed.
    echo attempting to install library...
    "!CLI_PATH!" lib install "Adafruit NeoPixel"
    "!CLI_PATH!" compile --fqbn !FQBN! "!SKETCH_DIR!"
    if !ERRORLEVEL! NEQ 0 (
        echo [FATAL] Compilation still failed.
        pause
        exit /b 1
    )
)

:: 4. FLASH
echo.
echo [INFO] Available Boards:
"!CLI_PATH!" board list
echo.

if "%~1"=="" (
    set /p COM_PORT="Enter your COM Port (e.g., COM3): "
) else (
    set "COM_PORT=%~1"
)

echo.
echo [INFO] Uploading to !COM_PORT!...
"!CLI_PATH!" upload -p !COM_PORT! --fqbn !FQBN! "!SKETCH_DIR!"

if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Upload failed.
    exit /b 1
)

echo.
echo [SUCCESS] Firmware uploaded.
echo [INFO] Opening Serial Monitor on !COM_PORT!...
echo (Press Ctrl+C to stop)
echo.

"!CLI_PATH!" monitor -p !COM_PORT! --config baudrate=115200

endlocal
