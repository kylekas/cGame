# cGame - Vulkan Game Engine

A C++ game engine built with Vulkan for learning modern graphics programming.

## Project Overview

This project aims to create a game engine using Vulkan, providing hands-on experience with:
- Modern graphics APIs
- Low-level GPU programming
- Game engine architecture
- Performance optimization
- Cross-platform development

## Features

- **Vulkan Rendering**: Modern, low-level graphics API
- **Cross-Platform**: Windows, Linux, macOS support
- **Modern C++**: C++17 standard with best practices
- **Modular Design**: Clean, extensible architecture
- **Performance Focused**: Direct GPU control for maximum performance

## Prerequisites

### Required Dependencies

- **CMake** (3.16 or higher)
- **C++17 compatible compiler** (GCC 7+, Clang 5+, MSVC 2017+)
- **Vulkan SDK** (1.2 or higher)
- **GLFW3** (3.3 or higher)
- **GLM** (Mathematics library)

### Installation

#### Ubuntu/Debian
```bash
sudo apt update
sudo apt install build-essential cmake
sudo apt install vulkan-tools vulkan-validationlayers
sudo apt install libglfw3-dev libglm-dev
```

#### Windows
1. Install [Vulkan SDK](https://vulkan.lunarg.com/sdk/home)
2. Install [CMake](https://cmake.org/download/)
3. Install [Visual Studio](https://visualstudio.microsoft.com/) or [MinGW](https://www.mingw-w64.org/)
4. Use vcpkg for GLFW and GLM:
   ```bash
   vcpkg install glfw3 glm
   ```

#### macOS
```bash
brew install cmake vulkan-headers glfw glm
```

## Building the Project

### Clone and Build
```bash
git clone <repository-url>
cd cGame
mkdir build
cd build
cmake ..
make -j$(nproc)
```

### Run the Application
```bash
./bin/cGame
```

## Project Structure

```
cGame/
├── src/                    # Source files
│   ├── main.cpp           # Application entry point
│   ├── Window.cpp         # Window management
│   └── VulkanRenderer.cpp # Vulkan rendering
├── include/               # Header files
│   ├── Window.h
│   └── VulkanRenderer.h
├── assets/               # Game assets (textures, models, etc.)
├── shaders/              # GLSL shader files
├── CMakeLists.txt        # Build configuration
└── README.md
```

## Development Roadmap

### Phase 1: Foundation ✅
- [x] Project structure setup
- [x] CMake build system
- [x] Basic Vulkan initialization
- [x] Window creation with GLFW
- [x] Device selection and setup

### Phase 2: Basic Rendering 🚧
- [ ] Swap chain creation
- [ ] Render pass setup
- [ ] Graphics pipeline
- [ ] Command buffer recording
- [ ] First triangle rendering

### Phase 3: Game Objects 📋
- [ ] Vertex buffer management
- [ ] Texture loading
- [ ] Game object system
- [ ] Basic game loop
- [ ] Input handling

### Phase 4: Game Logic 📋
- [ ] Game mechanics
- [ ] Collision detection
- [ ] Audio system
- [ ] Performance optimization
- [ ] Game states

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests if applicable
5. Submit a pull request

## Learning Resources

- [Vulkan Tutorial](https://vulkan-tutorial.com/)
- [Vulkan Specification](https://www.khronos.org/registry/vulkan/)
- [Vulkan Samples](https://github.com/KhronosGroup/Vulkan-Samples)
- [Vulkan Programming Guide](https://www.amazon.com/Vulkan-Programming-Guide-Official-Learning/dp/0134464540)

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- Khronos Group for Vulkan
- GLFW developers
- GLM library contributors
- Vulkan community and tutorials 