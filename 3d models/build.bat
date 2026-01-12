@echo off
setlocal enabledelayedexpansion

REM Ensure build dir exists
if not exist "build" mkdir "build"

REM Find OpenSCAD
set "OPENSCAD="
if exist "C:\Program Files\OpenSCAD\openscad.exe" set "OPENSCAD=C:\Program Files\OpenSCAD\openscad.exe"
if exist "C:\Program Files (x86)\OpenSCAD\openscad.exe" set "OPENSCAD=C:\Program Files (x86)\OpenSCAD\openscad.exe"
if exist "%LOCALAPPDATA%\Programs\OpenSCAD\openscad.exe" set "OPENSCAD=%LOCALAPPDATA%\Programs\OpenSCAD\openscad.exe"

if "%OPENSCAD%"=="" (
    echo Error: OpenSCAD not found. Please install it from https://openscad.org/
    exit /b 1
)

echo Using OpenSCAD at: "!OPENSCAD!"

REM Loop through all .scad files in design folder
for %%f in (design\*.scad) do (
    set "filename=%%~nf"
    echo Building !filename!...
    "!OPENSCAD!" -o "build\!filename!.stl" "%%f"
)

echo Build complete.
