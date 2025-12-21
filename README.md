# LithoMaker v1.0

Creates 3D lithophanes from image files and exports them to STL, OBJ, or 3MF files, ready for slicing and 3D printing.

**This is a modernized fork of [LithoMaker by Lars Muldjord](https://github.com/muldjord/lithomaker).**

Download the latest release [here](https://github.com/LibreArbitre/lithomaker/releases/download/1.0.0/LithoMaker-Windows-x64.zip).

Check out the original video tutorial [here](https://youtu.be/Bjbdk0XiiNY).

LithoMaker DOES NOT upload or process your image files online. All processing is done on your own computer requiring no internet access.

## What's New in v1.1

- **Separate Preview and Export**: Preview your lithophane in 3D before exporting
- **Flip Image Vertically**: Option to flip the image orientation
- **Detachable Stabilizer Feet**: Breakaway feet with thin necking for easy removal
- **Fixed Stabilizer Geometry**: Proper watertight mesh compatible with all slicers

### Previous Release (v1.0)
- Multiple image formats: JPEG, PNG, WEBP, TIFF, and BMP
- 3D Preview with mouse rotation and zoom
- Multiple export formats: STL (binary/ASCII), OBJ, and 3MF
- Drag & drop image loading
- Multi-threaded mesh generation
- Dark theme

## Running LithoMaker

### 🌐 Web Browser (No Installation)
* **Try it now**: [LithoMaker Web](https://librearbitre.github.io/lithomaker/)
* Works in Chrome, Firefox, Edge, and Safari
* All processing happens locally - your images never leave your device
* Requires WebAssembly support (all modern browsers)

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

### Main Settings
* **Minimum thickness**: The thinnest part of the lithophane (brightest areas). Keep at 0.8mm minimum.
* **Total thickness**: The thickest part (darkest areas). See thickness guide below.
* **Frame border**: Width of the frame in millimeters.
* **Width**: Total width of the lithophane including frame. Height adjusts automatically.
* **Flip image vertically**: Toggle to correct image orientation if needed.

### Workflow
1. **Load image**: Drag & drop or click to browse
2. **Adjust settings**: Thickness, frame, size
3. **Click Preview**: View the 3D result
4. **Adjust if needed**: Toggle flip, change settings, re-preview
5. **Click Export**: Save when satisfied

### Stabilizers
Stabilizers are small feet that support the lithophane during vertical printing. They prevent wobbling and print failures. 

**Options** (in Preferences → Render):
- **Enable stabilizers**: Toggle stabilizers on/off
- **Make stabilizers permanent**: When unchecked (default), feet have a thin breakaway connection for easy removal. When checked, feet are solid.

### Hangers
Small loops at the top allow you to hang your lithophane in a window or light box.

## 🎯 Printing Optimization Guide

### Thickness Settings (LithoMaker)

| Thickness | Effect | Recommended For |
|-----------|--------|-----------------|
| **3-4mm** | Standard contrast, faster print | Small pieces, tests |
| **5mm** | Good relief and detail | **Most lithophanes** |
| **6mm** | Maximum relief | Large portraits, exhibition pieces |

> **Tip**: Going from 4mm to 5mm provides noticeable improvement. 5mm is the sweet spot for quality vs print time.

### Layer Height (Slicer)

| Layer Height | Quality | Print Time |
|--------------|---------|------------|
| **0.20mm** | Acceptable, visible stepping | Fastest |
| **0.16mm** | **Good compromise** | Moderate |
| **0.12mm** | **Optimal for portraits** | Longer |
| **0.08mm** | Maximum quality | Very long |

> **Recommendation**: Use **0.16mm** for general use, **0.12mm** for detailed portraits.

### Print Settings

| Setting | Value | Why |
|---------|-------|-----|
| **Infill** | 100% | Required for light transmission |
| **Speed** | 30-40mm/s | Reduces vibration artifacts |
| **Orientation** | Vertical | How LithoMaker generates the model |
| **Supports** | None needed | Stabilizer feet provide support |
| **Filament** | White/Natural PLA | Best light transmission |

### Best Practices

1. **Image Preparation**: Use high-quality PNG, avoid heavily compressed JPEGs
2. **Convert to grayscale** before loading (optional but recommended)
3. **Resize large images** to 1500-2000 pixels width for faster processing
4. **Test with small prints** (50-80mm width) before printing large pieces
5. **Light source**: Place behind the lithophane for best effect

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

#### Version 1.1.0 (December 2024)
UX improvements and fixes:
* Separate Preview and Export buttons for better workflow
* Added "Flip image vertically" option
* Detachable stabilizer feet with thin breakaway connection
* Fixed stabilizer geometry for proper slicer compatibility
* Added comprehensive printing optimization guide

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
