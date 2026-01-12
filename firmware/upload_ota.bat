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

:: 2.5 EMBED WEB ASSETS
echo.
echo [INFO] Packing Web Assets...
python "%~dp0pack_web.py"
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Web packing failed.
    exit /b 1
)

:: 3. COMPILATION
echo.
echo [INFO] Compiling sketch for OTA...

if not exist "!BUILD_PATH!" mkdir "!BUILD_PATH!"

echo [CMD] Compiling...
call "!CLI_PATH!" compile --fqbn !FQBN! --build-path "!BUILD_PATH!" --libraries "libraries" "!SKETCH_DIR!"
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Compilation failed.
    exit /b 1
)

:: 4. OTA UPLOAD
echo.
echo [INFO] Compilation successful.
echo.

set "MANUAL_TARGET=%~1"

if not "!MANUAL_TARGET!"=="" (
    call :DoUpload "!MANUAL_TARGET!"
    exit /b !ERRORLEVEL!
)

if exist "%~dp0known_hosts.txt" (
    echo [INFO] Reading hosts from known_hosts.txt...
    for /F "usebackq tokens=*" %%H in ("%~dp0known_hosts.txt") do (
        :: Cleanup whitespace if any
        set "HOST=%%H"
        :: Skip empty lines or comments if needed (simple check)
        if not "!HOST!"=="" (
            call :DoUpload "!HOST!"
        )
    )
    exit /b 0
) else (
    echo [ERROR] No target IP specified and known_hosts.txt not found.
    echo Usage: upload_ota.bat [IP_ADDRESS]
    exit /b 1
)

:DoUpload
set "CURRENT_HOST=%~1"
echo.
echo ---------------------------------------------------
echo [TARGET] !CURRENT_HOST!
echo ---------------------------------------------------

:: Run espota in a separate process to prevent it from killing the loop on crash
:: AND redirect input to NUL to prevent it from consuming the loop's file stream
cmd /c ""!ESPOTA_PATH!" -i !CURRENT_HOST! -p 3232 -f "!BUILD_PATH!\AllSeeingEye.ino.bin" < NUL"

if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] OTA Upload to !CURRENT_HOST! failed (Exit Code: !ERRORLEVEL!).
    :: We don't exit here so we can try the next host in the list
) else (
    echo [SUCCESS] OTA Upload to !CURRENT_HOST! Complete.
)
echo [INFO] Finished processing !CURRENT_HOST!, returning to loop...
goto :eof
