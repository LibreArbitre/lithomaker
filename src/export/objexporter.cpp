/**
 * @file objexporter.cpp
 * @brief Wavefront OBJ mesh exporter implementation
 *
 * Copyright 2021-2024 Lars Muldjord / Contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "objexporter.h"

#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QMap>

namespace LithoMaker {

ExportResult ObjExporter::exportMesh(const QList<QVector3D>& mesh, 
                                      const QString& filePath) {
    if (mesh.isEmpty()) {
        return {false, QObject::tr("Empty mesh"), 0};
    }

    if (mesh.size() % 3 != 0) {
        return {false, QObject::tr("Invalid mesh: vertex count not divisible by 3"), 0};
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return {false, QObject::tr("Cannot open file for writing: ") + file.errorString(), 0};
    }

    QTextStream out(&file);
    out.setRealNumberNotation(QTextStream::FixedNotation);
    out.setRealNumberPrecision(6);

    // Header
    out << "# LithoMaker Export\n";
    out << "# Triangles: " << (mesh.size() / 3) << "\n\n";
    out << "o lithophane\n\n";

    // Deduplicate vertices for smaller file size
    QMap<QString, int> vertexMap;
    QList<QVector3D> uniqueVertices;
    QList<int> faceIndices;

    auto getVertexKey = [](const QVector3D& v) {
        return QString("%1_%2_%3")
            .arg(static_cast<double>(v.x()), 0, 'f', 6)
            .arg(static_cast<double>(v.y()), 0, 'f', 6)
            .arg(static_cast<double>(v.z()), 0, 'f', 6);
    };

    for (const QVector3D& v : mesh) {
        QString key = getVertexKey(v);
        if (!vertexMap.contains(key)) {
            vertexMap[key] = uniqueVertices.size() + 1; // OBJ is 1-indexed
            uniqueVertices.append(v);
        }
        faceIndices.append(vertexMap[key]);
    }

    // Write vertices
    for (const QVector3D& v : uniqueVertices) {
        out << "v " << v.x() << " " << v.y() << " " << v.z() << "\n";
    }

    out << "\n# Faces\n";

    // Write faces
    for (int i = 0; i < faceIndices.size(); i += 3) {
        out << "f " << faceIndices[i] 
            << " " << faceIndices[i + 1] 
            << " " << faceIndices[i + 2] << "\n";
    }

    qint64 written = file.size();
    file.close();

    qInfo() << "Exported OBJ:" << filePath << "(" << written << "bytes," 
            << uniqueVertices.size() << "unique vertices)";

    return {true, QString(), written};
}

} // namespace LithoMaker
