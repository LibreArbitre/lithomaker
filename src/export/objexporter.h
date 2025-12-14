/**
 * @file objexporter.h
 * @brief Wavefront OBJ mesh exporter
 *
 * Copyright 2021-2024 Lars Muldjord / Contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include "exporter.h"

namespace LithoMaker {

/**
 * @brief Wavefront OBJ mesh exporter
 */
class ObjExporter : public Exporter {
public:
    ObjExporter() = default;

    ExportResult exportMesh(const QList<QVector3D>& mesh, 
                           const QString& filePath) override;

    QString name() const override { return QStringLiteral("OBJ"); }
    QString extension() const override { return QStringLiteral("obj"); }
    QString fileFilter() const override { return QStringLiteral("Wavefront OBJ (*.obj)"); }
};

} // namespace LithoMaker
