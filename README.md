# LithoMaker v1.0

Creates 3D lithophanes from image files and exports them to STL, OBJ, or 3MF files, ready for slicing and 3D printing.

**This is a modernized fork of [LithoMaker by Lars Muldjord](https://github.com/muldjord/lithomaker).**

Download the latest release [here](https://github.com/YOUR_USERNAME/lithomaker/releases).

Check out the original video tutorial [here](https://youtu.be/Bjbdk0XiiNY).

LithoMaker DOES NOT upload or process your image files online. All processing is done on your own computer requiring no internet access.

## What's New in v1.0

- **Multiple image formats**: Now supports JPEG, PNG, WEBP, TIFF, and BMP
- **3D Preview**: See your lithophane before exporting
- **Multiple export formats**: STL (binary/ASCII), OBJ, and 3MF
- **Drag & drop**: Simply drop your image onto the window
- **Faster processing**: Multi-threaded mesh generation
- **Dark theme**: Modern look and feel
- **Improved stability**: Better handling of large images

## Running LithoMaker

### Windows
* Download the latest `LithoMaker-Windows-x64.zip` from the [releases page](https://github.com/LibreArbitre/lithomaker/releases)
* Unzip to any folder and run `LithoMaker.exe`

### Ubuntu Linux
* Download the latest `LithoMaker-Linux-x86_64.AppImage` from the [releases page](https://github.com/LibreArbitre/lithomaker/releases)
* Right-click → Properties → Permissions → "Allow executing file as program"
* Double-click to run

### macOS
* Download the latest `LithoMaker-macOS-Universal.dmg` from the [releases page](https://github.com/LibreArbitre/lithomaker/releases)
* Open and drag to Applications

## Using LithoMaker

Most options are self-explanatory. Here are the main settings:

* **Minimum thickness**: The thinnest part of the lithophane (brightest areas). Keep at 0.8mm minimum.
* **Total thickness**: The thickest part (darkest areas). 4-5mm works well for most prints.
* **Frame border**: Width of the frame in millimeters.
* **Width**: Total width of the lithophane including frame. Height adjusts automatically.
* **Input image**: The image you want to convert (drag & drop or click to browse)
* **Output file**: Where to save the 3D file
* **Export format**: Choose STL, OBJ, or 3MF

### Tips for Best Results
* Use high-quality images (avoid compressed JPEGs with visible artifacts)
* Color images are automatically converted to grayscale
* For large images, consider resizing to 1500-2000 pixels width
* Print lithophanes vertically with a light behind them

### Stabilizers
Stabilizers are small feet that support the lithophane during vertical printing. They prevent wobbling and print failures. After printing, you can easily snap them off.

### Hangers
Small loops at the top allow you to hang your lithophane in a window or light box.

## Preparing Your Photo

For optimal results, prepare your photo using an image editor like [GIMP](https://www.gimp.org/):

1. Apply noise reduction to smooth out grainy areas
2. Use Auto → White Balance to maximize contrast
3. Crop to your desired composition
4. Scale to about 1500 pixels width (optional but recommended)
5. Convert to grayscale
6. Export as PNG

## Building from Source

### Prerequisites
- A C++ compiler (GCC, Clang, or MSVC)
- CMake 3.21 or later
- Qt 6.2 or later with OpenGL support

### Linux
```bash
# Install dependencies (Ubuntu/Debian)
sudo apt install qt6-base-dev qt6-opengl-dev libomp-dev cmake build-essential

# Build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

### Windows
```powershell
mkdir build
cd build
cmake .. -DCMAKE_PREFIX_PATH="C:\Qt\6.x.x\msvc2019_64"
cmake --build . --config Release
```

### macOS
```bash
brew install qt@6 cmake libomp
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH=$(brew --prefix qt@6)
make -j$(sysctl -n hw.ncpu)
```

## Release Notes

#### Version 1.0.0 (December 2024)
Complete modernization:
* Added support for JPEG, WEBP, TIFF, and BMP images
* Real-time 3D preview with mouse rotation and zoom
* Export to OBJ and 3MF in addition to STL
* Drag & drop image loading
* Multi-threaded mesh generation using OpenMP
* Modern dark theme
* Migrated build system to CMake
* Refactored codebase to modular architecture

#### Version 0.7.1 (25th Nov 2021)
* Last release by original author Lars Muldjord
* See [original repository](https://github.com/muldjord/lithomaker) for full history

## Credits

* **Original author**: [Lars Muldjord](https://github.com/muldjord)
* **v1.0 modernization**: Contributors

## License

LithoMaker is licensed under the [GNU General Public License v2](LICENSE).

This is free software: you can redistribute it and/or modify it under the terms of the GPL as published by the Free Software Foundation.
