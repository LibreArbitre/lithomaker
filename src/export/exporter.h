/**
 * @file exporter.h
 * @brief Abstract mesh exporter interface
 *
 * Copyright 2021-2024 Lars Muldjord / Contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <QList>
#include <QVector3D>
#include <QString>

namespace LithoMaker {

/**
 * @brief Export result
 */
struct ExportResult {
    bool success{false};
    QString errorMessage;
    qint64 bytesWritten{0};
};

/**
 * @brief Abstract base class for mesh exporters
 */
class Exporter {
public:
    virtual ~Exporter() = default;

    /**
     * @brief Export mesh to file
     * @param mesh List of vertices (triangles, 3 per triangle)
     * @param filePath Output file path
     * @return Export result
     */
    virtual ExportResult exportMesh(const QList<QVector3D>& mesh, 
                                    const QString& filePath) = 0;

    /**
     * @brief Get the exporter name
     */
    virtual QString name() const = 0;

    /**
     * @brief Get file extension (without dot)
     */
    virtual QString extension() const = 0;

    /**
     * @brief Get file filter for file dialogs
     */
    virtual QString fileFilter() const = 0;
};

} // namespace LithoMaker
