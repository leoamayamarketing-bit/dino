@echo off
REM ===========================================================================
REM DinoRunner — Build & Package Script (Windows cmd.exe)
REM ===========================================================================
REM Cleans the build directory, recompiles from scratch, and packages the
REM executable into a dist/ folder ready for distribution.
REM
REM Usage:
REM   cd dino_game
REM   scripts\package.bat
REM
REM Requirements:
REM   - MSYS2 MINGW64 with MinGW-w64 tools in PATH
REM   - CMake, mingw32-make, g++
REM
REM Output:
REM   dist\
REM   ├── DinoRunner.exe      Standalone executable (no DLLs needed)
REM   └── README.txt           Quick-start instructions
REM ===========================================================================

setlocal enabledelayedexpansion
set RED=[31m
set GREEN=[32m
set YELLOW=[33m
set CYAN=[36m
set NC=[0m

echo.
echo  ██████╗ ██╗███╗   ██╗ ██████╗ ██████╗ ██╗   ██╗███╗   ██╗███╗   ██╗███████╗██████╗
echo  ██╔══██╗██║████╗  ██║██╔════╝ ██╔══██╗██║   ██║████╗  ██║████╗  ██║██╔════╝██╔══██╗
echo  ██║  ██║██║██╔██╗ ██║██║  ███╗██████╔╝██║   ██║██╔██╗ ██║██╔██╗ ██║█████╗  ██████╔╝
echo  ██║  ██║██║██║╚██╗██║██║   ██║██╔══██╗██║   ██║██║╚██╗██║██║╚██╗██║██╔══╝  ██╔══██╗
echo  ██████╔╝██║██║ ╚████║╚██████╔╝██║  ██║╚██████╔╝██║ ╚████║██║ ╚████║███████╗██║  ██║
echo  ╚═════╝ ╚═╝╚═╝  ╚═══╝ ╚═════╝ ╚═╝  ╚═╝ ╚═════╝ ╚═╝  ╚═══╝╚═╝  ╚═══╝╚══════╝╚═╝  ╚═╝
echo                                     Build ^& Package Script
echo.

REM Store the project root (directory where this script is located, go up one)
set SCRIPT_DIR=%~dp0
set PROJECT_DIR=%SCRIPT_DIR%..
set BUILD_DIR=%PROJECT_DIR%\build
set DIST_DIR=%PROJECT_DIR%\dist
set EXECUTABLE=DinoRunner.exe

REM Get number of processors
if not defined NUMBER_OF_PROCESSORS set NUMBER_OF_PROCESSORS=4

echo [INFO]  Project directory: %PROJECT_DIR%
echo [INFO]  Build directory:   %BUILD_DIR%
echo [INFO]  Output directory:  %DIST_DIR%
echo [INFO]  Build jobs:        %NUMBER_OF_PROCESSORS%
echo.

REM ===========================================================================
REM Step 1: Clean build directory
REM ===========================================================================
echo ============================================================
echo [INFO]  Step 1/5: Cleaning build directory...
echo ============================================================

if exist "%BUILD_DIR%" (
    rmdir /s /q "%BUILD_DIR%"
    if !errorlevel! equ 0 (
        echo [OK]    Build directory removed: %BUILD_DIR%
    ) else (
        echo [WARN]  Could not remove %BUILD_DIR% (maybe it's in use?)
    )
) else (
    echo [INFO]  Build directory does not exist.
)

mkdir "%BUILD_DIR%" 2>nul
echo [OK]    Build directory created: %BUILD_DIR%

REM ===========================================================================
REM Step 2: CMake configure
REM ===========================================================================
echo.
echo ============================================================
echo [INFO]  Step 2/5: Configuring with CMake...
echo ============================================================

cd /d "%BUILD_DIR%"
cmake "%PROJECT_DIR%" -G "MinGW Makefiles"
if %errorlevel% neq 0 (
    echo [ERROR] CMake configuration failed!
    exit /b 1
)
echo [OK]    CMake configuration complete.

REM ===========================================================================
REM Step 3: Build
REM ===========================================================================
echo.
echo ============================================================
echo [INFO]  Step 3/5: Building DinoRunner (%NUMBER_OF_PROCESSORS% parallel jobs)...
echo ============================================================

cd /d "%BUILD_DIR%"
mingw32-make -j%NUMBER_OF_PROCESSORS%
if %errorlevel% neq 0 (
    echo [ERROR] Build failed!
    exit /b 1
)

if not exist "%BUILD_DIR%\%EXECUTABLE%" (
    echo [ERROR] Build succeeded but %EXECUTABLE% not found!
    exit /b 1
)
echo [OK]    Build complete: %EXECUTABLE%

REM ===========================================================================
REM Step 4: Package
REM ===========================================================================
echo.
echo ============================================================
echo [INFO]  Step 4/5: Packaging into dist/...
echo ============================================================

REM Clean dist folder
if exist "%DIST_DIR%" (
    rmdir /s /q "%DIST_DIR%"
)
mkdir "%DIST_DIR%"

REM Copy executable
copy "%BUILD_DIR%\%EXECUTABLE%" "%DIST_DIR%\%EXECUTABLE%" >nul
if %errorlevel% equ 0 (
    echo [OK]    Copied: %EXECUTABLE%
) else (
    echo [ERROR] Failed to copy executable!
    exit /b 1
)

REM Create README.txt
(
echo ============================================
echo       DinoRunner: Primal Rush
echo              v1.0.0
echo ============================================
echo.
echo HOW TO RUN:
echo   Double-click DinoRunner.exe
echo   No installation needed.
echo.
echo CONTROLS:
echo   Space / W / Up    - Jump (hold for higher jump)
echo   W + W (mid-air)   - Double jump
echo   S / Down          - Crouch / Slide
echo   Shift             - Dash
echo   Escape            - Pause / Menu
echo   Enter             - Confirm / Select
echo.
echo FEATURES:
echo   - 3 playable dinosaurs (T-Rex, Raptor, Triceratops)
echo   - 5 levels (Desert Day, Desert Night, Cave, Volcano, Infinite)
echo   - Power-ups: Shield, Magnet, Double Points, Speed Boost, Extra Life
echo   - Configurable key bindings (from the Options menu)
echo   - Everything generated procedurally - no external files needed!
echo.
echo REQUIREMENTS:
echo   - Windows 7 or later (64-bit)
echo   - OpenGL-compatible graphics card
echo   - No additional DLLs or runtime dependencies
echo.
echo BUILD INFO:
echo   - C++17, SFML 2.6 (static), MinGW-w64 GCC
echo   - Compiled: %DATE%
echo.
)>"%DIST_DIR%\README.txt"
echo [OK]    Created: README.txt

REM Show file sizes
for %%F in ("%DIST_DIR%\%EXECUTABLE%") do set EXE_SIZE=%%~zF
set /a EXE_SIZE_KB=EXE_SIZE / 1024

echo.
echo   Package contents:
echo     %DIST_DIR%\
echo     +-- %EXECUTABLE%  (%EXE_SIZE_KB% KB)
echo     +-- README.txt

REM ===========================================================================
REM Step 5: Verify DLL dependencies
REM ===========================================================================
echo.
echo ============================================================
echo [INFO]  Step 5/5: Verifying executable...
echo ============================================================

echo [INFO]  Executable size: %EXE_SIZE_KB% KB
echo [INFO]  DLL verification: skipped on cmd.exe (use package.sh in MSYS2)

echo.
echo ============================================================
echo [OK]    Package ready: %DIST_DIR%
echo ============================================================
echo.
echo You can now distribute the contents of: %DIST_DIR%
echo.

endlocal
