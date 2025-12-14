/**
 * @file threemfexporter.cpp
 * @brief 3MF mesh exporter implementation
 *
 * Copyright 2021-2024 Lars Muldjord / Contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "threemfexporter.h"

#include <QFile>
#include <QDebug>
#include <QMap>
#include <QDir>
#include <QProcess>

namespace LithoMaker {

ExportResult ThreeMfExporter::exportMesh(const QList<QVector3D>& mesh, 
                                          const QString& filePath) {
    if (mesh.isEmpty()) {
        return {false, QObject::tr("Empty mesh"), 0};
    }

    if (mesh.size() % 3 != 0) {
        return {false, QObject::tr("Invalid mesh: vertex count not divisible by 3"), 0};
    }

    // Create temporary directory for 3MF contents
    QString tempDir = QDir::temp().filePath("lithomaker_3mf_" + QString::number(QDateTime::currentMSecsSinceEpoch()));
    QDir().mkpath(tempDir);
    QDir().mkpath(tempDir + "/3D");
    QDir().mkpath(tempDir + "/_rels");

    // Write content files
    QFile contentTypes(tempDir + "/[Content_Types].xml");
    if (contentTypes.open(QIODevice::WriteOnly | QIODevice::Text)) {
        contentTypes.write(generateContentTypesXml().toUtf8());
        contentTypes.close();
    }

    QFile rels(tempDir + "/_rels/.rels");
    if (rels.open(QIODevice::WriteOnly | QIODevice::Text)) {
        rels.write(generateRelsXml().toUtf8());
        rels.close();
    }

    QFile model(tempDir + "/3D/3dmodel.model");
    if (model.open(QIODevice::WriteOnly | QIODevice::Text)) {
        model.write(generateModelXml(mesh).toUtf8());
        model.close();
    }

    // Create ZIP using PowerShell (Windows) or zip command (Linux/Mac)
    bool success = false;
    
#ifdef Q_OS_WIN
    // Use PowerShell Compress-Archive
    QProcess process;
    QString tempDirWin = tempDir;
    QString filePathWin = filePath;
    tempDirWin.replace("/", "\\");
    filePathWin.replace("/", "\\");
    QString script = QString("Compress-Archive -Path '%1\\*' -DestinationPath '%2' -Force")
        .arg(tempDirWin)
        .arg(filePathWin);
    process.start("powershell", QStringList() << "-Command" << script);
    process.waitForFinished(30000);
    success = (process.exitCode() == 0);
#else
    // Use zip command on Linux/Mac
    QProcess process;
    process.setWorkingDirectory(tempDir);
    process.start("zip", QStringList() << "-r" << filePath << ".");
    process.waitForFinished(30000);
    success = (process.exitCode() == 0);
#endif

    // Cleanup temp directory
    QDir(tempDir).removeRecursively();

    if (!success) {
        return {false, QObject::tr("Failed to create 3MF archive"), 0};
    }

    QFile file(filePath);
    qint64 size = file.exists() ? file.size() : 0;

    qInfo() << "Exported 3MF:" << filePath << "(" << size << "bytes)";

    return {true, QString(), size};
}

QString ThreeMfExporter::generateContentTypesXml() {
    return R"(<?xml version="1.0" encoding="UTF-8"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
  <Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
  <Default Extension="model" ContentType="application/vnd.ms-package.3dmanufacturing-3dmodel+xml"/>
</Types>
)";
}

QString ThreeMfExporter::generateRelsXml() {
    return R"(<?xml version="1.0" encoding="UTF-8"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
  <Relationship Target="/3D/3dmodel.model" Id="rel0" Type="http://schemas.microsoft.com/3dmanufacturing/2013/01/3dmodel"/>
</Relationships>
)";
}

QString ThreeMfExporter::generateModelXml(const QList<QVector3D>& mesh) {
    // Deduplicate vertices
    QMap<QString, int> vertexMap;
    QList<QVector3D> uniqueVertices;
    QList<int> triangleIndices;

    auto getVertexKey = [](const QVector3D& v) {
        return QString("%1_%2_%3")
            .arg(static_cast<double>(v.x()), 0, 'f', 6)
            .arg(static_cast<double>(v.y()), 0, 'f', 6)
            .arg(static_cast<double>(v.z()), 0, 'f', 6);
    };

    for (const QVector3D& v : mesh) {
        QString key = getVertexKey(v);
        if (!vertexMap.contains(key)) {
            vertexMap[key] = uniqueVertices.size(); // 3MF is 0-indexed
            uniqueVertices.append(v);
        }
        triangleIndices.append(vertexMap[key]);
    }

    QString xml;
    xml += R"(<?xml version="1.0" encoding="UTF-8"?>
<model unit="millimeter" xmlns="http://schemas.microsoft.com/3dmanufacturing/core/2015/02">
  <resources>
    <object id="1" type="model">
      <mesh>
        <vertices>
)";

    // Vertices
    for (const QVector3D& v : uniqueVertices) {
        xml += QString("          <vertex x=\"%1\" y=\"%2\" z=\"%3\"/>\n")
            .arg(static_cast<double>(v.x()), 0, 'f', 6)
            .arg(static_cast<double>(v.y()), 0, 'f', 6)
            .arg(static_cast<double>(v.z()), 0, 'f', 6);
    }

    xml += "        </vertices>\n        <triangles>\n";

    // Triangles
    for (int i = 0; i < triangleIndices.size(); i += 3) {
        xml += QString("          <triangle v1=\"%1\" v2=\"%2\" v3=\"%3\"/>\n")
            .arg(triangleIndices[i])
            .arg(triangleIndices[i + 1])
            .arg(triangleIndices[i + 2]);
    }

    xml += R"(        </triangles>
      </mesh>
    </object>
  </resources>
  <build>
    <item objectid="1"/>
  </build>
</model>
)";

    return xml;
}

} // namespace LithoMaker
