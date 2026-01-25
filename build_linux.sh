#!/bin/bash
# ========================================
# RME Linux Build Script (Conan)
# Always Release build, logs to build.log
# ========================================

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build_conan"
LOG_FILE="$SCRIPT_DIR/build.log"

{
    echo "========================================"
    echo " RME Linux Build Script"
    echo " Started: $(date)"
    echo "========================================"
} > "$LOG_FILE"

echo "[1/4] Checking Conan profile..."
echo "[1/4] Checking Conan profile..." >> "$LOG_FILE"

if ! conan profile show &> /dev/null; then
    echo "  Creating default Conan profile..."
    conan profile detect >> "$LOG_FILE" 2>&1
fi

echo "[2/4] Installing dependencies with Conan..."
echo "[2/4] Installing dependencies..." >> "$LOG_FILE"

conan install "$SCRIPT_DIR" -of "$BUILD_DIR" --build=missing -s build_type=Release -s compiler.cppstd=20 -c tools.system.package_manager:mode=install -c tools.system.package_manager:sudo=True >> "$LOG_FILE" 2>&1

echo "[3/4] Configuring CMake..."
echo "[3/4] Configuring CMake..." >> "$LOG_FILE"

cmake --preset conan-default >> "$LOG_FILE" 2>&1

echo "[4/4] Building Release..."
echo "[4/4] Building Release..." >> "$LOG_FILE"

cmake --build "$BUILD_DIR/build" --config Release --parallel "$(nproc)" >> "$LOG_FILE" 2>&1

{
    echo "========================================"
    echo " BUILD SUCCESSFUL"
    echo " Output: $BUILD_DIR/build/rme"
    echo " Finished: $(date)"
    echo "========================================"
} >> "$LOG_FILE"

echo "========================================"
echo " BUILD SUCCESSFUL!"
echo " Output: $BUILD_DIR/build/rme"
echo " Log: $LOG_FILE"
echo "========================================"
