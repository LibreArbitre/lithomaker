# LithoMaker 1.1.0

LithoMaker turns an image into a 3D lithophane that you can preview, adjust, and export for 3D printing. It is available as a desktop app and as a browser app.

[Open LithoMaker in your browser](https://librearbitre.github.io/lithomaker/) · [Download the latest desktop release](https://github.com/LibreArbitre/lithomaker/releases/latest)

This is a modernized fork of [LithoMaker by Lars Muldjord](https://github.com/muldjord/lithomaker).

## Browser version

The browser version runs locally in your browser. Images and generated models are not uploaded to a LithoMaker server. It opens with a sample image, so you can click **Preview** and try the 3D view immediately. You can also load your own image and export the model from the browser.

Use a modern browser with WebAssembly and WebGL 2 support. The browser version exports STL (binary or ASCII) and OBJ. 3MF export is available in the desktop app only.

## Features

- Preview a lithophane in 3D before exporting; rotate it by dragging and zoom with the mouse wheel.
- Load PNG, JPEG, WebP, TIFF, and BMP images.
- Set the minimum thickness, total thickness, frame border, and model width.
- Flip the image vertically when needed.
- Add stabilizer feet and optional hangers in the desktop app's Render preferences.
- Export STL (binary or ASCII), OBJ, or 3MF on desktop.
- Generate meshes on multiple CPU threads in desktop builds that support OpenMP.

## Download the desktop app

Download the package for your platform from the [latest release](https://github.com/LibreArbitre/lithomaker/releases/latest). Extract or install it, then launch LithoMaker. The browser version is available at [librearbitre.github.io/lithomaker](https://librearbitre.github.io/lithomaker/) and needs no installation.

## Using LithoMaker

1. Load an image by dragging it into the desktop app or using the image button. In the browser, use the image button to choose a file.
2. Set the minimum thickness, total thickness, frame border, and width.
3. Click **Preview** to generate and inspect the 3D model. Drag in the preview to rotate it.
4. Change the image orientation or dimensions if needed, then preview again.
5. Click **Export** and save the generated model.

### Thickness settings

Minimum thickness controls the thinnest, brightest parts of the lithophane. Total thickness controls its deepest, darkest parts. A minimum thickness of at least 0.8 mm is a useful starting point.

| Total thickness | Typical use |
| --- | --- |
| 3–4 mm | Small pieces and test prints |
| 5 mm | General use and detailed images |
| 6 mm | Larger images with stronger relief |

The frame border and width are measured in millimeters. LithoMaker preserves the input image's aspect ratio when calculating the model height.

### Print suggestions

- Use white or natural PLA and print the lithophane vertically.
- Use 100% infill so light can pass through the varying thickness.
- A layer height of 0.16 mm is a useful general setting; 0.12 mm can show more detail.
- Stabilizer feet can support a vertically printed model. In desktop preferences, choose whether they should break away or remain solid.
- Desktop preferences can also add hangers to the top of the model.

## Build from source

LithoMaker uses C++17, CMake 3.21 or later, and Qt 6.5.3.

### Desktop

Install Qt 6.5.3 with the Core, Widgets, Gui, OpenGL, and OpenGLWidgets modules, then configure and build:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

### WebAssembly

The browser build requires the Qt 6.5.3 WebAssembly kit and the Emscripten version supported by that kit (3.1.25):

```sh
<Qt-WASM>/bin/qt-cmake -S . -B build-wasm \
  -DBUILD_WASM=ON \
  -DCMAKE_BUILD_TYPE=Release \
  -DQT_HOST_PATH=<Qt-host>
cmake --build build-wasm --parallel
```

The generated HTML, JavaScript, and WebAssembly files must be served over HTTP; opening the HTML directly from disk is not supported. GitHub Pages deployment is configured in `.github/workflows/build-wasm.yml`.

## Release history

### 1.1.0

- Added separate Preview and Export actions and image-flip control.
- Improved stabilizer geometry and added breakaway feet.
- Added a browser edition with a working WebGL 2 preview, image loading, and STL/OBJ export.
- Added the bundled example image for an immediate browser preview.

### 1.0.0

- Modernized the application and moved the build system to CMake.
- Added JPEG, WebP, TIFF, and BMP support, plus OBJ and 3MF export.
- Added the 3D preview, image drag and drop, and multi-threaded desktop mesh generation.

### 0.7.1

Last release by original author Lars Muldjord. See the [original repository](https://github.com/muldjord/lithomaker) for its history.

## Credits and license

- Original author: [Lars Muldjord](https://github.com/muldjord)
- LithoMaker is licensed under [GNU GPL v2 or later](LICENSE).
