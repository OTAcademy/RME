@echo off
REM ========================================
REM RME Windows Build Script (vcpkg)
REM Always Release build, logs to build.log
REM ========================================

setlocal enabledelayedexpansion

set SCRIPT_DIR=%~dp0
set BUILD_DIR=%SCRIPT_DIR%build
set VCPKG_ROOT=c:\vcpkg
set LOG_FILE=%SCRIPT_DIR%build.log

echo ======================================== > "%LOG_FILE%"
echo  RME Windows Build Script >> "%LOG_FILE%"
echo  Started: %date% %time% >> "%LOG_FILE%"
echo ======================================== >> "%LOG_FILE%"

echo [1/3] Configuring CMake with vcpkg...
echo [1/3] Configuring CMake... >> "%LOG_FILE%"

cmake -B "%BUILD_DIR%" -S "%SCRIPT_DIR%." "-DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake" -DCMAKE_BUILD_TYPE=Release >> "%LOG_FILE%" 2>&1

if %ERRORLEVEL% neq 0 (
    echo ERROR: CMake configure failed. See build.log for details.
    echo ERROR: CMake configure failed >> "%LOG_FILE%"
    exit /b 1
)

echo [2/3] Building Release...
echo [2/3] Building Release... >> "%LOG_FILE%"

cmake --build "%BUILD_DIR%" --config Release --parallel >> "%LOG_FILE%" 2>&1

if %ERRORLEVEL% neq 0 (
    echo ERROR: Build failed. See build.log for details.
    echo ERROR: Build failed >> "%LOG_FILE%"
    exit /b 1
)

echo [3/3] Build complete!
echo ======================================== >> "%LOG_FILE%"
echo  BUILD SUCCESSFUL >> "%LOG_FILE%"
echo  Output: %BUILD_DIR%\Release\rme.exe >> "%LOG_FILE%"
echo  Finished: %date% %time% >> "%LOG_FILE%"
echo ======================================== >> "%LOG_FILE%"

echo ========================================
echo  BUILD SUCCESSFUL!
echo  Output: %BUILD_DIR%\Release\rme.exe
echo  Log: %LOG_FILE%
echo ========================================

endlocal
