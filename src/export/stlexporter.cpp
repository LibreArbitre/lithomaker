/**
 * @file stlexporter.cpp
 * @brief STL mesh exporter implementation
 *
 * Copyright 2021-2024 Lars Muldjord / Contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "stlexporter.h"

#include <QSaveFile>
#include <QDebug>
#include <cstring>

namespace LithoMaker {

namespace {

bool writeAll(QIODevice& device, const char* data, qint64 size) {
    return device.write(data, size) == size;
}

ExportResult writeFailure(const QSaveFile& file) {
    return {false, QObject::tr("Failed while writing output file: ") + file.errorString(), 0};
}

} // namespace

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
    QSaveFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        return {false, QObject::tr("Cannot open file for writing: ") + file.errorString(), 0};
    }

    // 80 byte header
    char header[80];
    std::memset(header, 0, 80);
    std::strncpy(header, "LithoMaker Export", 79);
    if (!writeAll(file, header, 80)) return writeFailure(file);

    // Number of triangles (uint32)
    quint32 triangleCount = static_cast<quint32>(mesh.size() / 3);
    if (!writeAll(file, reinterpret_cast<const char*>(&triangleCount), sizeof(quint32)))
        return writeFailure(file);

    // Write triangles
    for (int i = 0; i < mesh.size(); i += 3) {
        // Normal vector (not calculated, set to 0)
        float normal[3] = {0.0f, 0.0f, 0.0f};
        if (!writeAll(file, reinterpret_cast<const char*>(normal), sizeof(float) * 3))
            return writeFailure(file);

        // Three vertices
        for (int j = 0; j < 3; ++j) {
            const QVector3D& v = mesh.at(i + j);
            float vertex[3] = {v.x(), v.y(), v.z()};
            if (!writeAll(file, reinterpret_cast<const char*>(vertex), sizeof(float) * 3))
                return writeFailure(file);
        }

        // Attribute byte count
        quint16 attrByteCount = 0;
        if (!writeAll(file, reinterpret_cast<const char*>(&attrByteCount), sizeof(quint16)))
            return writeFailure(file);
    }

    qint64 written = file.pos();
    if (!file.commit()) return writeFailure(file);

    qInfo() << "Exported binary STL:" << filePath << "(" << written << "bytes," 
            << triangleCount << "triangles)";

    return {true, QString(), written};
}

ExportResult StlExporter::exportAscii(const QList<QVector3D>& mesh, 
                                       const QString& filePath) {
    QSaveFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return {false, QObject::tr("Cannot open file for writing: ") + file.errorString(), 0};
    }

    auto writeText = [&file](const QString& text) {
        const QByteArray bytes = text.toLatin1();
        return writeAll(file, bytes.constData(), bytes.size());
    };

    if (!writeText(QStringLiteral("solid lithophane\n"))) return writeFailure(file);

    for (int i = 0; i < mesh.size(); i += 3) {
        if (!writeText(QStringLiteral("facet normal 0.0 0.0 0.0\n")) ||
            !writeText(QStringLiteral("\touter loop\n"))) return writeFailure(file);
        
        for (int j = 0; j < 3; ++j) {
            const QVector3D& v = mesh.at(i + j);
            QString line = QString("\t\tvertex %1 %2 %3\n")
                .arg(static_cast<double>(v.x()), 0, 'g', 6)
                .arg(static_cast<double>(v.y()), 0, 'g', 6)
                .arg(static_cast<double>(v.z()), 0, 'g', 6);
            if (!writeText(line)) return writeFailure(file);
        }
        
        if (!writeText(QStringLiteral("\tendloop\n")) ||
            !writeText(QStringLiteral("endfacet\n"))) return writeFailure(file);
    }

    if (!writeText(QStringLiteral("endsolid\n"))) return writeFailure(file);

    qint64 written = file.pos();
    if (!file.commit()) return writeFailure(file);

    qInfo() << "Exported ASCII STL:" << filePath << "(" << written << "bytes)";

    return {true, QString(), written};
}

} // namespace LithoMaker
