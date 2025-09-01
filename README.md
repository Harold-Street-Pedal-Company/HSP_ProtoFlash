# HSP ProtoFlash

HSP ProtoFlash is a Windows 10/11 GUI application for rapid Flashing of Binaries to Daisy Seed (or theoretically any DFU bnased) devices, built with C++ and the JUCE framework.

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
  
## Installer
   the packaged installer `HSP_Install.exe` will install the HSP_Protoflash.exe file without you being required to build it yourself.  Double click and follow the instructions.  
  # NOTE:
  Windows Defender may report this as an unsafe download.  It is not, but I should still warn you to use at your own risk!  (even though you can see the source code)

## License

This project is licensed under the GNU General Public License v3.0.  
See [LICENSE](LICENSE) for details.

## Credits

- [JUCE](https://juce.com/) - C++ framework for audio and GUI applications.
