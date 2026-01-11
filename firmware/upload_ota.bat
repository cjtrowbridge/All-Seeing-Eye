@echo off
setlocal EnableDelayedExpansion

echo ==========================================
echo  All-Seeing-Eye OTA Uploader
echo ==========================================

:: 1. LOCATE TOOLS
set "CLI_PATH=C:\Users\CJ\AppData\Local\Programs\Arduino IDE\resources\app\lib\backend\resources\arduino-cli.exe"
set "ESPOTA_PATH=C:\Users\CJ\AppData\Local\Arduino15\packages\esp32\hardware\esp32\3.3.3\tools\espota.exe"

:: 2. CONFIGURATION
set "FQBN=esp32:esp32:esp32s3:CDCOnBoot=cdc,FlashSize=16M,PSRAM=opi,USBMode=hwcdc,FlashMode=qio,PartitionScheme=app3M_fat9M_16MB"
set "SKETCH_DIR=%~dp0AllSeeingEye"
set "BUILD_PATH=%~dp0build"

:: 3. COMPILATION
echo.
echo [INFO] Compiling sketch for OTA...

if not exist "!BUILD_PATH!" mkdir "!BUILD_PATH!"

"!CLI_PATH!" compile --fqbn !FQBN! --build-path "!BUILD_PATH!" --libraries "libraries" "!SKETCH_DIR!"
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Compilation failed.
    exit /b 1
)

:: 4. OTA UPLOAD
echo.
echo [INFO] Compilation successful.
echo.

:: Use first argument as IP if provided, else default
set "IP_ADDR=%~1"
if "!IP_ADDR!"=="" set "IP_ADDR=allseeingeye.local"

echo [INFO] Uploading firmware to !IP_ADDR!...

"!ESPOTA_PATH!" -i !IP_ADDR! -p 3232 -f "!BUILD_PATH!\AllSeeingEye.ino.bin"
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] OTA Upload failed.
    exit /b 1
)

echo [SUCCESS] OTA Upload Complete.
exit /b 0
