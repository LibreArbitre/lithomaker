cmake_minimum_required(VERSION 3.21)

project(LithoMaker
    VERSION 1.0.0
    DESCRIPTION "Creates 3D lithophanes from images"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# WASM build option
option(BUILD_WASM "Build for WebAssembly (browser)" OFF)

# Find Qt6 - different modules for WASM vs Desktop
if(BUILD_WASM)
    # WASM build - OpenGLWidgets not needed, use different approach
    find_package(Qt6 REQUIRED COMPONENTS
        Core
        Widgets
        Gui
        OpenGL
    )
    message(STATUS "Configuring for WebAssembly build")
else()
    # Desktop build
    find_package(Qt6 REQUIRED COMPONENTS
        Core
        Widgets
        Gui
        OpenGL
        OpenGLWidgets
    )
    # Find OpenMP for desktop only (not supported in WASM)
    find_package(OpenMP)
endif()

# Version from file
file(READ "${CMAKE_SOURCE_DIR}/VERSION" VERSION_FILE)
string(REGEX MATCH "VERSION=([0-9]+\\.[0-9]+\\.[0-9]+)" _ ${VERSION_FILE})
set(LITHOMAKER_VERSION ${CMAKE_MATCH_1})

# Configure version header
configure_file(
    "${CMAKE_SOURCE_DIR}/src/version.h.in"
    "${CMAKE_BINARY_DIR}/generated/version.h"
)

# Source files - Core
set(CORE_SOURCES
    src/core/settings.cpp
    src/core/imageloader.cpp
)

set(CORE_HEADERS
    src/core/settings.h
    src/core/imageloader.h
)

# Source files - Mesh
set(MESH_SOURCES
    src/mesh/meshgenerator.cpp
)

set(MESH_HEADERS
    src/mesh/meshgenerator.h
)

# Source files - Export
set(EXPORT_SOURCES
    src/export/stlexporter.cpp
    src/export/objexporter.cpp
    src/export/threemfexporter.cpp
)

set(EXPORT_HEADERS
    src/export/exporter.h
    src/export/stlexporter.h
    src/export/objexporter.h
    src/export/threemfexporter.h
)

# Source files - UI
set(UI_SOURCES
    src/ui/mainwindow.cpp
    src/ui/configdialog.cpp
    src/ui/configpages.cpp
    src/ui/aboutbox.cpp
    src/ui/widgets/slider.cpp
    src/ui/widgets/checkbox.cpp
    src/ui/widgets/combobox.cpp
    src/ui/widgets/lineedit.cpp
)

set(UI_HEADERS
    src/ui/mainwindow.h
    src/ui/configdialog.h
    src/ui/configpages.h
    src/ui/aboutbox.h
    src/ui/widgets/slider.h
    src/ui/widgets/checkbox.h
    src/ui/widgets/combobox.h
    src/ui/widgets/lineedit.h
)

# PreviewWidget uses QOpenGLWidget - not available in WASM
if(NOT BUILD_WASM)
    list(APPEND UI_SOURCES src/ui/previewwidget.cpp)
    list(APPEND UI_HEADERS src/ui/previewwidget.h)
endif()

# Resources
set(RESOURCES
    lithomaker.qrc
)

# All sources
set(ALL_SOURCES
    src/main.cpp
    ${CORE_SOURCES}
    ${MESH_SOURCES}
    ${EXPORT_SOURCES}
    ${UI_SOURCES}
)

set(ALL_HEADERS
    ${CORE_HEADERS}
    ${MESH_HEADERS}
    ${EXPORT_HEADERS}
    ${UI_HEADERS}
)

# Create executable
add_executable(${PROJECT_NAME}
    ${ALL_SOURCES}
    ${ALL_HEADERS}
    ${RESOURCES}
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_BINARY_DIR}/generated
)

# Link libraries - different for WASM vs Desktop
if(BUILD_WASM)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Gui
        Qt6::OpenGL
    )
    
    # WASM-specific linker options
    target_link_options(${PROJECT_NAME} PRIVATE
        "SHELL:-s WASM=1"
        "SHELL:-s ALLOW_MEMORY_GROWTH=1"
        "SHELL:-s MAXIMUM_MEMORY=2GB"
        "SHELL:-s ASYNCIFY=1"
        "SHELL:-s EXPORTED_RUNTIME_METHODS=['ccall','cwrap']"
        # Required for Qt WASM - enables embind for QSettings/permissions
        "--bind"
        "SHELL:-s FETCH=1"
    )
    
    # Define BUILD_WASM for conditional compilation in C++ code
    target_compile_definitions(${PROJECT_NAME} PRIVATE BUILD_WASM)
    
    message(STATUS "WASM build configured with async support and 2GB max memory")
else()
    # Desktop linking
    target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Gui
        Qt6::OpenGL
        Qt6::OpenGLWidgets
    )
    
    # OpenMP (desktop only)
    if(OpenMP_CXX_FOUND)
        target_link_libraries(${PROJECT_NAME} PRIVATE OpenMP::OpenMP_CXX)
        target_compile_definitions(${PROJECT_NAME} PRIVATE USE_OPENMP)
    endif()
    
    # Platform-specific
    if(WIN32)
        set_target_properties(${PROJECT_NAME} PROPERTIES
            WIN32_EXECUTABLE TRUE
        )
        target_sources(${PROJECT_NAME} PRIVATE lithomaker.rc)
    endif()
    
    if(APPLE)
        set_target_properties(${PROJECT_NAME} PROPERTIES
            MACOSX_BUNDLE TRUE
            MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME}
            MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
            MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
        )
    endif()
endif()

# Install
install(TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Copy examples
install(DIRECTORY examples/ DESTINATION share/${PROJECT_NAME}/examples)
