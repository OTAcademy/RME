# Building RME

## Prerequisites

| Tool | Windows | Linux (Ubuntu) |
|------|---------|----------------|
| C++ Compiler | Visual Studio 2022 | GCC 11+ or Clang 14+ |
| CMake | 3.23+ | 3.23+ |
| Package Manager | vcpkg | Conan 2.x |

---

## Windows Setup

### 1. Install Visual Studio 2022
Download from [visualstudio.microsoft.com](https://visualstudio.microsoft.com/) with **Desktop development with C++** workload.

### 2. Install CMake
```cmd
winget install Kitware.CMake
```

### 3. Install vcpkg
```cmd
git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
C:\vcpkg\bootstrap-vcpkg.bat
```

### 4. Build
```cmd
build_windows.bat
```
Output: `build\Release\rme.exe`  
Log: `build.log`

---

## Linux (Ubuntu) Setup

### 1. Install Build Tools
```bash
sudo apt update
sudo apt install -y build-essential cmake git python3 python3-pip
sudo apt install -y libgtk-3-dev libgl1-mesa-dev libglu1-mesa-dev freeglut3-dev
```

### 2. Install Conan
```bash
pip3 install conan
export PATH="$HOME/.local/bin:$PATH"  # Add to ~/.bashrc
```

### 3. Build
```bash
chmod +x build_linux.sh
./build_linux.sh
```
Output: `build_conan/build/rme`  
Log: `build.log`
