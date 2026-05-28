# VulkanEngine

A simple engine written in C++/Vulkan.

![PBR Rendering](Resources/PBR-Rendering.png)

## Getting Started with test app

### Download the [Vulkan SDK](https://vulkan.lunarg.com/sdk/home)

### Clone the repository
```bash
git clone --recursive https://github.com/012MykA/VulkanEngine.git
cd VulkanEngine
```

### Select your preset from `CMakePresets.json`
* Windows:
```bash
cmake --preset vcpkg-windows
```
* Linux:
```bash
cmake --preset vcpkg-linux
```

### Build
* Windows:
```bash
cmake --build build/vcpkg-windows --config Release
```
* Windows:
```bash
cmake --build build/vcpkg-linux --config Release
```

Now that you have the executable, make sure it is located next to the ```VulkanEngine/assets/``` folder before running it.
