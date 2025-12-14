/**
 * @file stlexporter.h
 * @brief STL mesh exporter (binary and ASCII)
 *
 * Copyright 2021-2024 Lars Muldjord / Contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include "exporter.h"

namespace LithoMaker {

/**
 * @brief STL format type
 */
enum class StlFormat {
    Binary,
    Ascii
};

/**
 * @brief STL mesh exporter
 */
class StlExporter : public Exporter {
public:
    explicit StlExporter(StlFormat format = StlFormat::Binary);

    ExportResult exportMesh(const QList<QVector3D>& mesh, 
                           const QString& filePath) override;

    QString name() const override { return QStringLiteral("STL"); }
    QString extension() const override { return QStringLiteral("stl"); }
    QString fileFilter() const override { return QStringLiteral("STL Files (*.stl)"); }

    void setFormat(StlFormat format) { m_format = format; }
    StlFormat format() const { return m_format; }

private:
    ExportResult exportBinary(const QList<QVector3D>& mesh, const QString& filePath);
    ExportResult exportAscii(const QList<QVector3D>& mesh, const QString& filePath);

    StlFormat m_format;
};

} // namespace LithoMaker
