#!/bin/bash
# ========================================
# RME Conan Dependency Setup Script
# Run ONCE to cache all dependencies
# ========================================

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build_conan"

echo "========================================"
echo " RME Conan Dependency Setup"
echo " Started: $(date)"
echo "========================================"

# Step 1: Detect/create Conan profile
echo "[1/3] Setting up Conan profile..."
if ! conan profile show &> /dev/null; then
    echo "  Creating default Conan profile..."
    conan profile detect
fi

# Show detected profile
echo "  Detected profile:"
conan profile show | head -20

# Step 2: Install all dependencies (download binaries or build from source)
echo ""
echo "[2/3] Installing dependencies (this may take a while on first run)..."
echo "  Build directory: $BUILD_DIR"

conan install "$SCRIPT_DIR" \
    -of "$BUILD_DIR" \
    --build=missing \
    -s build_type=Release \
    -c tools.system.package_manager:mode=install \
    -c tools.system.package_manager:sudo=True

# Step 3: Verify CMake presets were generated
echo ""
echo "[3/3] Verifying CMake presets..."
if [ -f "$BUILD_DIR/CMakePresets.json" ]; then
    echo "  ✅ CMakePresets.json generated"
else
    echo "  ❌ CMakePresets.json NOT found - something went wrong"
    exit 1
fi

if [ -f "$BUILD_DIR/build/Release/generators/conan_toolchain.cmake" ]; then
    echo "  ✅ Conan toolchain generated"
else
    echo "  ⚠️  Toolchain path may differ, check $BUILD_DIR"
fi

echo ""
echo "========================================"
echo " SETUP COMPLETE!"
echo " Dependencies cached in: ~/.conan2/p/"
echo " Build files in: $BUILD_DIR"
echo ""
echo " Next step - run the build:"
echo "   ./build_linux.sh"
echo "========================================"
