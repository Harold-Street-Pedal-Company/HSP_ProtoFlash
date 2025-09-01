# HSP ProtoFlash

HSP ProtoFlash is a Windows 10/11 GUI application for rapid Flashing of Binaries to Daisy Seed (or theoretically any DFU bnased) devices, built with C++ and the JUCE framework.

## Features

- Modern C++ GUI and audio support via JUCE
- Windows resource file integration
- Automatic source file collection
- Customizable company and product metadata

## Requirements

- CMake 3.22 or higher
- JUCE library (set `JUCE_DIR` when configuring)
- C++17 compatible compiler

## Building

1. **Clone the repository**:
   ```cpp
   git clone <your-repo-url> cd HSP_ProtoFlash
   
2. **Configure the project with CMake**:
   ```cpp
   cmake -B build -S . -DJUCE_DIR="C:/JUCE/"
3. **Build the application**:
   ```cpp
   cmake --build build
  
## Customization

- Edit `CMakeLists.txt` to add or remove JUCE modules.
- Place your source files in the `Source/` directory.
- Update resource files as needed in the project root.

## License

This project is licensed under the GNU General Public License v3.0.  
See [LICENSE](LICENSE) for details.

## Credits

- [JUCE](https://juce.com/) - C++ framework for audio and GUI applications.
