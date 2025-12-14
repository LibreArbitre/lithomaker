/**
 * @file imageloader.h
 * @brief Multi-format image loading with quality validation
 *
 * Copyright 2021-2024 Lars Muldjord / Contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <QImage>
#include <QString>
#include <QStringList>
#include <optional>

namespace LithoMaker {

/**
 * @brief Image loading result with quality information
 */
struct ImageLoadResult {
    QImage image;
    bool wasConverted{false};      ///< True if image was converted from color to grayscale
    bool wasResized{false};        ///< True if image was resized
    QString originalFormat;        ///< Original image format
    QSize originalSize;            ///< Original image size before any processing
    bool hasQualityWarning{false}; ///< True if JPEG with potential artifacts
};

/**
 * @brief Multi-format image loader with preprocessing
 *
 * Supports: PNG, JPEG, WEBP, TIFF, BMP
 * Automatically converts to grayscale and optionally resizes.
 */
class ImageLoader {
public:
    /**
     * @brief Get supported file format filter for file dialogs
     * @return File filter string (e.g., "Images (*.png *.jpg ...)")
     */
    static QString supportedFormatsFilter();

    /**
     * @brief Get list of supported file extensions
     * @return List of extensions without dots (e.g., ["png", "jpg", ...])
     */
    static QStringList supportedExtensions();

    /**
     * @brief Check if a file extension is supported
     * @param extension File extension (with or without dot)
     * @return true if the format is supported
     */
    static bool isFormatSupported(const QString& extension);

    /**
     * @brief Load and preprocess an image for lithophane generation
     * @param filePath Path to the image file
     * @param maxSize Maximum dimension (width or height) for resizing. 0 = no resize
     * @param forceResize If true, always resize if larger than maxSize
     * @return ImageLoadResult with the processed image, or nullopt on error
     */
    static std::optional<ImageLoadResult> load(
        const QString& filePath,
        int maxSize = 0,
        bool forceResize = false
    );

    /**
     * @brief Detect if a JPEG image has visible compression artifacts
     * @param image The image to check
     * @return true if artifacts are likely present
     */
    static bool detectJpegArtifacts(const QImage& image);

private:
    ImageLoader() = default;
};

} // namespace LithoMaker
