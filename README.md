# HSP ProtoFlash

HSP ProtoFlash is a cross-platform GUI application for rapid prototyping of audio tools and plugins, built with C++ and the JUCE framework.

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

## Customization

- Edit `CMakeLists.txt` to add or remove JUCE modules.
- Place your source files in the `Source/` directory.
- Update resource files as needed in the project root.

## License

This project is licensed under the GNU General Public License v3.0.  
See [LICENSE](LICENSE) for details.

## Credits

- [JUCE](https://juce.com/) - C++ framework for audio and GUI applications.
