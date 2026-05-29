# VulkanEngine

A simple engine written in C++ using Vulkan API.

![PBR Rendering](Resources/PBR-Rendering.png)

## Getting Started with test app

Make sure you have [Vulkan SDK](https://vulkan.lunarg.com/sdk/home) installed

### Clone the repository
```bash
git clone --recursive https://github.com/012MykA/VulkanEngine.git
cd VulkanEngine
```

### 1. Configure the project
Select your preset from `CMakePresets.json`
* Windows (Standard):
```bash
cmake --preset vcpkg-windows
```
* Windows (Distribution mode without console)
```bash
cmake --preset vcpkg-windows-dist
```
* Linux:
```bash
cmake --preset vcpkg-linux
```

### 2. Build the project
* Windows:
```bash
# Debug build
cmake --build --preset win-debug

# Release build
cmake --build --preset win-release

# Distribution build (Release + VE_DIST_MODE=ON)
cmake --build --preset win-dist
```
* Linux:
```bash
# Debug build
cmake --build --preset linux-debug

# Release build
cmake --build --preset linux-release
```

Now that you have the executable, make sure it is located next to the ```VulkanEngine/assets/``` folder before running it.
