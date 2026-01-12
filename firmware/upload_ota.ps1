<#
.SYNOPSIS
    All Seeing Eye - Fleet Deployment Script (OTA)
    
.DESCRIPTION
    This script handles the full build and deployment lifecycle for the fleet:
    1. Resource Packing: Compresses `web/index.html` into `src/WebStatic.h`.
    2. Compilation: Builds the firmware using `arduino-cli`.
       - Logs output to `build/compile.log`.
    3. Discovery: Reads targets from `known_hosts.txt`.
    4. Deployment: Checks status and uploads via `espota`.
       - Logs individual upload results to `build/upload_<hostname>.log`.

.NOTES
    - Requires: Python 3, Arduino CLI, ESP32 Core (espota.exe).
    - Paths below are hardcoded for the current environment but use $PSScriptRoot for portability within the repo.
    - If `HTTPClient.h` errors occur, ensure standard libraries are linked correctly.

.AUTHOR
    CJ Trowbridge / GitHub Copilot
#>

$ErrorActionPreference = "Stop"

# --- CONFIGURATION ---
$CliPath    = "C:\Users\CJ\AppData\Local\Programs\Arduino IDE\resources\app\lib\backend\resources\arduino-cli.exe"
$EspOtaPath = "C:\Users\CJ\AppData\Local\Arduino15\packages\esp32\hardware\esp32\3.3.3\tools\espota.exe"
$Fqbn       = "esp32:esp32:esp32s3:CDCOnBoot=cdc,FlashSize=16M,PSRAM=opi,USBMode=hwcdc,FlashMode=qio,PartitionScheme=app3M_fat9M_16MB"

# Directories
$SketchDir  = "$PSScriptRoot\AllSeeingEye"
$BuildDir   = "$PSScriptRoot\build"
$LibPath    = "$PSScriptRoot\libraries"
$HostsFile  = "$PSScriptRoot\known_hosts.txt"

# Outputs
$BinPath    = "$BuildDir\AllSeeingEye.ino.bin"
$CompileLog = "$BuildDir\compile.log"

# --- EXECUTION ---

# 1. Pack Web Assets
Write-Host "[1/3] Packing Web Assets..." -ForegroundColor Cyan
python "$PSScriptRoot\pack_web.py"

# 2. Compile Firmware
Write-Host "[2/3] Compiling Firmware..." -ForegroundColor Cyan
if (!(Test-Path $BuildDir)) { New-Item -ItemType Directory -Path $BuildDir | Out-Null }

Write-Host "      Logs: $CompileLog" -ForegroundColor Gray
# Use Start-Process or direct invocation with redirection. 
# 2>&1 merges stderr to stdout, Tee-Object shows it in console AND writes to file.
& $CliPath compile --fqbn $Fqbn --build-path $BuildDir --libraries $LibPath $SketchDir 2>&1 | Tee-Object -FilePath $CompileLog

if ($LASTEXITCODE -ne 0) { 
    Write-Error "Compilation Failed. Check $CompileLog for details."
    exit 1 
}

# 3. Fleet Deployment
Write-Host "[3/3] Deploying to Fleet..." -ForegroundColor Cyan

# Relax error preference for the loop so one failure doesn't stop the rest
$ErrorActionPreference = "Continue"

if (Test-Path $HostsFile) {
    $hosts = Get-Content $HostsFile
    foreach ($h in $hosts) {
        if ([string]::IsNullOrWhiteSpace($h)) { continue }
        $target = $h.Trim()
        $uploadLog = "$BuildDir\upload_$target.log"

        Write-Host "`n---------------------------------------------------" -ForegroundColor Gray
        Write-Host "Target: $target" -ForegroundColor Yellow
        
        # Simple Ping Check
        if (Test-Connection -ComputerName $target -Count 1 -Quiet -ErrorAction SilentlyContinue) {
            Write-Host "  Status: ONLINE" -ForegroundColor Green
            Write-Host "  Uploading..." -NoNewline
            
            # Run espota and capture output
            & $EspOtaPath -i $target -p 3232 -f $BinPath 2>&1 | Out-File -FilePath $uploadLog -Encoding UTF8
            
            if ($LASTEXITCODE -eq 0) {
                Write-Host " SUCCESS" -ForegroundColor Green
            } else {
                Write-Host " FAILED" -ForegroundColor Red
                Write-Host "    (See $uploadLog)" -ForegroundColor Red
            }
        } else {
            Write-Host "  Status: OFFLINE (Skipping)" -ForegroundColor Red
        }
    }
} else {
    Write-Warning "known_hosts.txt not found!"
}

Write-Host "`nAll Tasks Completed." -ForegroundColor Cyan
