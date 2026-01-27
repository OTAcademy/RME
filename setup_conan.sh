#!/bin/bash
# ========================================
# RME Linux Dependency Setup Script
# Installs system packages first, then Conan for remaining deps
# ========================================

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build_conan"

echo "========================================"
echo " RME Linux Dependency Setup"
echo " Started: $(date)"
echo "========================================"

# Step 1: Install system dependencies via apt (fast, pre-compiled)
echo "[1/4] Installing system dependencies via apt..."
sudo apt update
sudo apt install -y \
    build-essential \
    cmake \
    ninja-build \
    git \
    python3 \
    python3-pip \
    libwxgtk3.2-dev \
    libgtk-3-dev \
    libgl1-mesa-dev \
    libglu1-mesa-dev \
    libboost-all-dev \
    libarchive-dev \
    zlib1g-dev \
    libglm-dev \
    libspdlog-dev \
    libfmt-dev \
    nlohmann-json3-dev \
    libasio-dev

echo "  ✅ System packages installed"

# Step 2: Setup Conan profile
echo ""
echo "[2/4] Setting up Conan profile..."
if ! conan profile show &> /dev/null; then
    echo "  Creating default Conan profile..."
    conan profile detect
fi

# Step 3: Install remaining dependencies via Conan (only what's not in apt)
echo ""
echo "[3/4] Installing remaining dependencies via Conan..."
echo "  Build directory: $BUILD_DIR"

conan install "$SCRIPT_DIR" \
    -of "$BUILD_DIR" \
    --build=missing \
    -s build_type=Release

# Step 4: Verify CMake presets were generated
echo ""
echo "[4/4] Verifying CMake presets..."
if [ -f "$BUILD_DIR/build/Release/generators/CMakePresets.json" ]; then
    echo "  ✅ CMakePresets.json generated"
else
    echo "  ⚠️  CMakePresets.json not in expected location, checking alternatives..."
    find "$BUILD_DIR" -name "CMakePresets.json" 2>/dev/null || echo "  Not found"
fi

# Step 5: Clean working tree (required for remote verification)
echo ""
echo "[5/4] Cleaning working tree..."
git reset --hard

echo ""
echo "========================================"
echo " SETUP COMPLETE!"
echo " System packages: apt"
echo " Remaining deps: ~/.conan2/p/"
echo " Build files: $BUILD_DIR"
echo ""
echo " Next step - run the build:"
echo "   ./build_linux.sh"
echo "========================================"
