@echo off
setlocal EnableDelayedExpansion

echo ==========================================
echo  All-Seeing-Eye OTA Uploader
echo ==========================================

:: 1. LOCATE TOOLS
set "CLI_PATH=C:\Users\CJ\AppData\Local\Programs\Arduino IDE\resources\app\lib\backend\resources\arduino-cli.exe"
:: Found this path in previous steps
set "ESPOTA_PATH=C:\Users\CJ\AppData\Local\Arduino15\packages\esp32\hardware\esp32\3.3.3\tools\espota.exe"

if not exist "!CLI_PATH!" (
    echo [ERROR] Could not find arduino-cli.exe
    pause
    exit /b 1
)
if not exist "!ESPOTA_PATH!" (
    echo [ERROR] Could not find espota.exe
    pause
    exit /b 1
)

:: 2. CONFIGURATION
:: Must match the settings used for the initial upload
set "FQBN=esp32:esp32:esp32s3:CDCOnBoot=cdc,FlashSize=16M,PSRAM=opi,USBMode=hwcdc,FlashMode=qio"
set "SKETCH_DIR=%~dp0AllSeeingEye"
set "BUILD_PATH=%~dp0build"

echo.
echo [INFO] Compiling sketch for OTA...
echo This creates a binary file to send over the network.
echo.

:: 3. COMPILE
:: We use --build-path to easily find the .bin file
if not exist "!BUILD_PATH!" mkdir "!BUILD_PATH!"

"!CLI_PATH!" compile --fqbn !FQBN! --build-path "!BUILD_PATH!" "!SKETCH_DIR!"
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Compilation failed.
    pause
    exit /b 1
)

:: 4. OTA UPLOAD
echo.
echo [INFO] Compilation successful.
echo.
set /p IP_ADDR="Enter the IP Address of All-Seeing-Eye (e.g. 192.168.1.50): "

echo.
echo [INFO] Uploading firmware to !IP_ADDR!...
echo.

"!ESPOTA_PATH!" -i !IP_ADDR! -p 3232 -f "!BUILD_PATH!\AllSeeingEye.ino.bin"

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [ERROR] OTA Upload failed.
    echo Check:
    echo  1. The device is powered on and connected to the same WiFi.
    echo  2. The IP address is correct.
    echo  3. Firewalls are allowing port 3232.
    pause
    exit /b 1
)

echo.
echo [SUCCESS] OTA Update Complete!
echo The device should now restart.
echo.
pause
endlocal
