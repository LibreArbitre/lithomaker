/**
 * @file threemfexporter.h
 * @brief 3MF mesh exporter
 *
 * Copyright 2021-2024 Lars Muldjord / Contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include "exporter.h"

namespace LithoMaker {

/**
 * @brief 3MF mesh exporter
 * 
 * Creates a valid 3MF package (ZIP with XML content)
 * compatible with modern 3D printing slicers.
 */
class ThreeMfExporter : public Exporter {
public:
    ThreeMfExporter() = default;

    ExportResult exportMesh(const QList<QVector3D>& mesh, 
                           const QString& filePath) override;

    QString name() const override { return QStringLiteral("3MF"); }
    QString extension() const override { return QStringLiteral("3mf"); }
    QString fileFilter() const override { return QStringLiteral("3MF Files (*.3mf)"); }

private:
    QString generateModelXml(const QList<QVector3D>& mesh);
    QString generateContentTypesXml();
    QString generateRelsXml();
};

} // namespace LithoMaker
