/**
 * @file stlexporter.cpp
 * @brief STL mesh exporter implementation
 *
 * Copyright 2021-2024 Lars Muldjord / Contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "stlexporter.h"

#include <QFile>
#include <QDataStream>
#include <QDebug>
#include <cstring>

namespace LithoMaker {

StlExporter::StlExporter(StlFormat format)
    : m_format(format)
{
}

ExportResult StlExporter::exportMesh(const QList<QVector3D>& mesh, 
                                      const QString& filePath) {
    if (mesh.isEmpty()) {
        return {false, QObject::tr("Empty mesh"), 0};
    }

    if (mesh.size() % 3 != 0) {
        return {false, QObject::tr("Invalid mesh: vertex count not divisible by 3"), 0};
    }

    if (m_format == StlFormat::Binary) {
        return exportBinary(mesh, filePath);
    } else {
        return exportAscii(mesh, filePath);
    }
}

ExportResult StlExporter::exportBinary(const QList<QVector3D>& mesh, 
                                        const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        return {false, QObject::tr("Cannot open file for writing: ") + file.errorString(), 0};
    }

    // 80 byte header
    char header[80];
    std::memset(header, 0, 80);
    std::strncpy(header, "LithoMaker Export", 79);
    file.write(header, 80);

    // Number of triangles (uint32)
    quint32 triangleCount = static_cast<quint32>(mesh.size() / 3);
    file.write(reinterpret_cast<const char*>(&triangleCount), sizeof(quint32));

    // Write triangles
    for (int i = 0; i < mesh.size(); i += 3) {
        // Normal vector (not calculated, set to 0)
        float normal[3] = {0.0f, 0.0f, 0.0f};
        file.write(reinterpret_cast<const char*>(normal), sizeof(float) * 3);

        // Three vertices
        for (int j = 0; j < 3; ++j) {
            const QVector3D& v = mesh.at(i + j);
            float vertex[3] = {v.x(), v.y(), v.z()};
            file.write(reinterpret_cast<const char*>(vertex), sizeof(float) * 3);
        }

        // Attribute byte count
        quint16 attrByteCount = 0;
        file.write(reinterpret_cast<const char*>(&attrByteCount), sizeof(quint16));
    }

    qint64 written = file.size();
    file.close();

    qInfo() << "Exported binary STL:" << filePath << "(" << written << "bytes," 
            << triangleCount << "triangles)";

    return {true, QString(), written};
}

ExportResult StlExporter::exportAscii(const QList<QVector3D>& mesh, 
                                       const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return {false, QObject::tr("Cannot open file for writing: ") + file.errorString(), 0};
    }

    file.write("solid lithophane\n");

    for (int i = 0; i < mesh.size(); i += 3) {
        file.write("facet normal 0.0 0.0 0.0\n");
        file.write("\touter loop\n");
        
        for (int j = 0; j < 3; ++j) {
            const QVector3D& v = mesh.at(i + j);
            QString line = QString("\t\tvertex %1 %2 %3\n")
                .arg(static_cast<double>(v.x()), 0, 'g', 6)
                .arg(static_cast<double>(v.y()), 0, 'g', 6)
                .arg(static_cast<double>(v.z()), 0, 'g', 6);
            file.write(line.toLatin1());
        }
        
        file.write("\tendloop\n");
        file.write("endfacet\n");
    }

    file.write("endsolid\n");

    qint64 written = file.size();
    file.close();

    qInfo() << "Exported ASCII STL:" << filePath << "(" << written << "bytes)";

    return {true, QString(), written};
}

} // namespace LithoMaker
