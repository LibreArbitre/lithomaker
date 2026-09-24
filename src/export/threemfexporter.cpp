/**
 * @file threemfexporter.cpp
 * @brief 3MF mesh exporter implementation
 *
 * Copyright 2021-2024 Lars Muldjord / Contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "threemfexporter.h"

#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QMap>
#include <QDir>
#include <QSaveFile>
#include <QTemporaryDir>
#ifndef BUILD_WASM
#include <QProcess>
#include <QProcessEnvironment>
#endif

namespace LithoMaker {

ExportResult ThreeMfExporter::exportMesh(const QList<QVector3D>& mesh, 
                                          const QString& filePath) {
#ifdef BUILD_WASM
    // 3MF export requires QProcess which is not available in browser
    Q_UNUSED(mesh);
    Q_UNUSED(filePath);
    return {false, QObject::tr("3MF export is not available in browser version. Please use STL or OBJ format."), 0};
#else
    if (mesh.isEmpty()) {
        return {false, QObject::tr("Empty mesh"), 0};
    }

    if (mesh.size() % 3 != 0) {
        return {false, QObject::tr("Invalid mesh: vertex count not divisible by 3"), 0};
    }

    QTemporaryDir packageDir(QDir::temp().filePath("lithomaker_3mf_XXXXXX"));
    QTemporaryDir archiveDir(QDir::temp().filePath("lithomaker_3mf_archive_XXXXXX"));
    if (!packageDir.isValid() || !archiveDir.isValid()) {
        return {false, QObject::tr("Cannot create temporary files for 3MF export"), 0};
    }

    const QString tempDir = packageDir.path();
    if (!QDir().mkpath(tempDir + "/3D") || !QDir().mkpath(tempDir + "/_rels")) {
        return {false, QObject::tr("Cannot create temporary files for 3MF export"), 0};
    }

    // Write content files
    auto writePackageFile = [](const QString& path, const QString& contents) {
        QFile file(path);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;
        const QByteArray bytes = contents.toUtf8();
        return file.write(bytes) == bytes.size() && file.flush() && file.error() == QFileDevice::NoError;
    };

    if (!writePackageFile(tempDir + "/[Content_Types].xml", generateContentTypesXml()) ||
        !writePackageFile(tempDir + "/_rels/.rels", generateRelsXml()) ||
        !writePackageFile(tempDir + "/3D/3dmodel.model", generateModelXml(mesh))) {
        return {false, QObject::tr("Failed to write 3MF package contents"), 0};
    }

    const QString archivePath = archiveDir.filePath("LithoMaker.3mf");

    // Create ZIP using PowerShell (Windows) or zip command (Linux/Mac).
    // Keep user-controlled paths out of PowerShell source code.
    bool success = false;
    
#ifdef Q_OS_WIN
    // Use PowerShell Compress-Archive
    QProcess process;
    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    environment.insert("LITHOMAKER_3MF_SOURCE", QDir::toNativeSeparators(tempDir));
    environment.insert("LITHOMAKER_3MF_ARCHIVE", QDir::toNativeSeparators(archivePath));
    process.setProcessEnvironment(environment);
    const QString script =
        "$ErrorActionPreference = 'Stop'; "
        "Compress-Archive -Path (Join-Path -Path $env:LITHOMAKER_3MF_SOURCE -ChildPath '*') "
        "-DestinationPath $env:LITHOMAKER_3MF_ARCHIVE -Force";
    process.start("powershell", QStringList() << "-NoProfile" << "-NonInteractive" << "-Command" << script);
    success = process.waitForStarted() && process.waitForFinished(30000) &&
              process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0;
    if (process.state() != QProcess::NotRunning) {
        process.kill();
        process.waitForFinished();
    }
#else
    // Use zip command on Linux/Mac
    QProcess process;
    process.setWorkingDirectory(tempDir);
    process.start("zip", QStringList() << "-q" << "-r" << archivePath << ".");
    success = process.waitForStarted() && process.waitForFinished(30000) &&
              process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0;
    if (process.state() != QProcess::NotRunning) {
        process.kill();
        process.waitForFinished();
    }
#endif

    if (!success || !QFileInfo(archivePath).isFile() || QFileInfo(archivePath).size() == 0) {
        return {false, QObject::tr("Failed to create 3MF archive"), 0};
    }

    QFile archive(archivePath);
    if (!archive.open(QIODevice::ReadOnly)) {
        return {false, QObject::tr("Failed to read temporary 3MF archive"), 0};
    }

    QSaveFile output(filePath);
    if (!output.open(QIODevice::WriteOnly)) {
        return {false, QObject::tr("Cannot open file for writing: ") + output.errorString(), 0};
    }

    QByteArray buffer(1024 * 1024, '\0');
    while (true) {
        const qint64 bytesRead = archive.read(buffer.data(), buffer.size());
        if (bytesRead < 0) {
            output.cancelWriting();
            return {false, QObject::tr("Failed to read temporary 3MF archive"), 0};
        }
        if (bytesRead == 0) break;
        if (output.write(buffer.constData(), bytesRead) != bytesRead) {
            output.cancelWriting();
            return {false, QObject::tr("Failed while writing output file: ") + output.errorString(), 0};
        }
    }

    if (!output.commit()) {
        return {false, QObject::tr("Failed while writing output file: ") + output.errorString(), 0};
    }

    const qint64 size = QFileInfo(filePath).size();

    qInfo() << "Exported 3MF:" << filePath << "(" << size << "bytes)";

    return {true, QString(), size};
#endif // BUILD_WASM
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
