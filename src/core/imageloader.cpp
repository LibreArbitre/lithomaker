/**
 * @file imageloader.cpp
 * @brief Multi-format image loading implementation
 *
 * Copyright 2021-2024 Lars Muldjord / Contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "imageloader.h"

#include <QImageReader>
#include <QFileInfo>
#include <QDebug>

namespace LithoMaker {

QString ImageLoader::supportedFormatsFilter() {
    return QObject::tr("Images (*.png *.jpg *.jpeg *.webp *.tiff *.tif *.bmp);;")
         + QObject::tr("PNG Images (*.png);;")
         + QObject::tr("JPEG Images (*.jpg *.jpeg);;")
         + QObject::tr("WebP Images (*.webp);;")
         + QObject::tr("TIFF Images (*.tiff *.tif);;")
         + QObject::tr("BMP Images (*.bmp);;")
         + QObject::tr("All Files (*)");
}

QStringList ImageLoader::supportedExtensions() {
    return {"png", "jpg", "jpeg", "webp", "tiff", "tif", "bmp"};
}

bool ImageLoader::isFormatSupported(const QString& extension) {
    QString ext = extension.toLower();
    if (ext.startsWith('.')) {
        ext = ext.mid(1);
    }
    return supportedExtensions().contains(ext);
}

std::optional<ImageLoadResult> ImageLoader::load(
    const QString& filePath,
    int maxSize,
    bool forceResize
) {
    QImageReader reader(filePath);
    if (!reader.canRead()) {
        qWarning() << "Cannot read image:" << filePath << "-" << reader.errorString();
        return std::nullopt;
    }

    ImageLoadResult result;
    result.originalFormat = QString::fromLatin1(reader.format()).toUpper();
    result.originalSize = reader.size();

    // Load the image
    result.image = reader.read();
    if (result.image.isNull()) {
        qWarning() << "Failed to decode image:" << filePath << "-" << reader.errorString();
        return std::nullopt;
    }

    // Check for JPEG quality issues
    if (result.originalFormat == "JPEG" || result.originalFormat == "JPG") {
        result.hasQualityWarning = detectJpegArtifacts(result.image);
        if (result.hasQualityWarning) {
            qInfo() << "JPEG quality warning for:" << filePath;
        }
    }

    // Resize if needed
    if (maxSize > 0 && 
        (result.image.width() > maxSize || result.image.height() > maxSize)) {
        if (forceResize) {
            if (result.image.width() > result.image.height()) {
                result.image = result.image.scaledToWidth(maxSize, Qt::SmoothTransformation);
            } else {
                result.image = result.image.scaledToHeight(maxSize, Qt::SmoothTransformation);
            }
            result.wasResized = true;
            qInfo() << "Image resized to:" << result.image.size();
        }
    }

    // Convert to grayscale if not already
    if (!result.image.isGrayscale()) {
        result.image = result.image.convertToFormat(QImage::Format_Grayscale8);
        result.wasConverted = true;
        qInfo() << "Image converted to grayscale";
    }

    return result;
}

bool ImageLoader::detectJpegArtifacts(const QImage& image) {
    // Simple heuristic: check for blocky artifacts by analyzing 8x8 blocks
    // JPEG uses 8x8 DCT blocks, so artifacts often appear at block boundaries
    
    if (image.width() < 16 || image.height() < 16) {
        return false;
    }

    int blockBoundaryDiffs = 0;
    int internalDiffs = 0;
    int samples = 0;

    // Sample some 8x8 block boundaries
    for (int y = 8; y < image.height() - 8; y += 32) {
        for (int x = 8; x < image.width() - 8; x += 32) {
            // Difference at block boundary (x = 8, 16, 24, ...)
            int boundaryDiff = std::abs(
                qGray(image.pixel(x, y)) - qGray(image.pixel(x - 1, y))
            );
            
            // Difference inside block (x = 4, 12, 20, ...)
            int internalDiff = std::abs(
                qGray(image.pixel(x - 4, y)) - qGray(image.pixel(x - 5, y))
            );
            
            blockBoundaryDiffs += boundaryDiff;
            internalDiffs += internalDiff;
            samples++;
        }
    }

    if (samples == 0) return false;

    // If boundary differences are significantly higher than internal differences,
    // it's likely there are visible compression artifacts
    double boundaryAvg = static_cast<double>(blockBoundaryDiffs) / samples;
    double internalAvg = static_cast<double>(internalDiffs) / samples;

    // Threshold: if boundary average is 50% higher than internal average
    return boundaryAvg > internalAvg * 1.5 && boundaryAvg > 10;
}

} // namespace LithoMaker
