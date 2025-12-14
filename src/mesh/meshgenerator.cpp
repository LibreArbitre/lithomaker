/**
 * @file meshgenerator.cpp
 * @brief Mesh generator implementation with OpenMP parallelization
 *
 * Copyright 2021-2024 Lars Muldjord / Contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "meshgenerator.h"

#include <QDebug>

#ifdef USE_OPENMP
#include <omp.h>
#endif

namespace LithoMaker {

MeshGenerator::MeshGenerator(const MeshConfig& config)
    : m_config(config)
{
}

void MeshGenerator::setConfig(const MeshConfig& config) {
    m_config = config;
}

QList<QVector3D> MeshGenerator::generate(const QImage& image,
                                          ProgressCallback progressCallback) {
    m_mesh.clear();

    QImage grayscaleImage = image.convertToFormat(QImage::Format_Grayscale8);

    // Pre-calculate factors
    m_border = m_config.frameBorder;
    m_depthFactor = (m_config.totalThickness - m_config.minThickness) / 255.0f;
    m_widthFactor = (m_config.width - (m_border * 2)) / grayscaleImage.width();

    const float totalHeight = (m_border * 2) + (grayscaleImage.height() * m_widthFactor);
    m_meshDimensions = QSizeF(m_config.width, totalHeight);
    
    // Estimate polygon count for memory reservation
    const int estimatedVertices = 
        (image.width() - 1) * (image.height() - 1) * 6 * 3 + // Lithophane
        12 +  // Backside
        500 + // Frame estimate
        (m_config.enableStabilizers ? 1000 : 0) +
        (m_config.enableHangers ? m_config.hangerCount * 300 : 0);
    
    m_mesh.reserve(estimatedVertices);
    
    qInfo() << "Generating mesh for image" << grayscaleImage.size()
            << "-> final size" << m_meshDimensions << "mm";

    // Generate lithophane heightmap (parallelized)
    generateLithophane(grayscaleImage);

    if (progressCallback) progressCallback(50, 100);
    
    // Generate backside
    if (m_config.enableSegmentation && m_config.backsideSegments > 1) {
        generateSegmentedBackside(grayscaleImage);
    } else {
        generateBackside(grayscaleImage);
    }
    
    if (progressCallback) progressCallback(60, 100);
    
    // Generate frame
    generateFrame(m_config.width, totalHeight);
    
    if (progressCallback) progressCallback(80, 100);
    
    // Generate stabilizers if needed
    if (m_config.enableStabilizers && 
        totalHeight > m_config.stabilizerThreshold) {
        generateStabilizers(m_config.width, totalHeight);
    }
    
    // Generate hangers
    if (m_config.enableHangers) {
        generateHangers(m_config.width, totalHeight);
    }
    
    if (progressCallback) progressCallback(100, 100);
    
    qInfo() << "Mesh generated:" << (m_mesh.size() / 3) << "triangles";
    
    return m_mesh;
}

void MeshGenerator::generateLithophane(const QImage& image) {
    const float minThickness = -m_config.minThickness;
    const int height = image.height();
    const int width = image.width();

    QVector<float> depthBuffer(width * height);
    for (int y = 0; y < height; ++y) {
        const uchar* sourceRow = image.constScanLine(height - 1 - y);
        float* targetRow = depthBuffer.data() + y * width;
        for (int x = 0; x < width; ++x) {
            targetRow[x] = static_cast<float>(sourceRow[x]) * m_depthFactor;
        }
    }

    const float* const buffer = depthBuffer.constData();
    const float* const topRow = buffer;
    const float* const bottomRow = buffer + (height - 1) * width;

    // Thread-local mesh storage for parallel generation
    #ifdef USE_OPENMP
    const int numThreads = omp_get_max_threads();
    QVector<QList<QVector3D>> threadMeshes(numThreads);

    #pragma omp parallel
    {
        const int threadId = omp_get_thread_num();
        auto& localMesh = threadMeshes[threadId];
        localMesh.reserve((height / numThreads) * width * 18); // Rough estimate

        #pragma omp for schedule(dynamic, 32)
        for (int y = 0; y < height - 1; ++y) {
    #else
        auto& localMesh = m_mesh;
        for (int y = 0; y < height - 1; ++y) {
    #endif
            const float* row = buffer + y * width;
            const float* nextRow = row + width;
            
            const float leftDepth = row[0];
            const float leftNextDepth = nextRow[0];
            // Close left side
            localMesh.append(scaleVertex(0, y, minThickness));
            localMesh.append(scaleVertex(0, y, leftDepth));
            localMesh.append(scaleVertex(0, y + 1, leftNextDepth));

            localMesh.append(scaleVertex(0, y + 1, leftNextDepth));
            localMesh.append(scaleVertex(0, y + 1, minThickness));
            localMesh.append(scaleVertex(0, y, minThickness));

            for (int x = 0; x < width - 1; ++x) {
                const float topRightDepth = row[x + 1];
                const float topDepth = row[x];
                const float bottomDepth = nextRow[x];
                const float bottomRightDepth = nextRow[x + 1];
                if (y == 0) {
                    // Close top
                    localMesh.append(scaleVertex(x + 1, 0, topRow[x + 1]));
                    localMesh.append(scaleVertex(x, 0, topRow[x]));
                    localMesh.append(scaleVertex(x, 0, minThickness));

                    localMesh.append(scaleVertex(x, 0, minThickness));
                    localMesh.append(scaleVertex(x + 1, 0, minThickness));
                    localMesh.append(scaleVertex(x + 1, 0, topRow[x + 1]));

                    // Close bottom
                    localMesh.append(scaleVertex(x, height - 1, minThickness));
                    localMesh.append(scaleVertex(x, height - 1, bottomRow[x]));
                    localMesh.append(scaleVertex(x + 1, height - 1, bottomRow[x + 1]));

                    localMesh.append(scaleVertex(x + 1, height - 1, bottomRow[x + 1]));
                    localMesh.append(scaleVertex(x + 1, height - 1, minThickness));
                    localMesh.append(scaleVertex(x, height - 1, minThickness));
                }

                // The lithophane heightmap - two triangles per pixel
                localMesh.append(scaleVertex(x, y, topDepth));
                localMesh.append(scaleVertex(x + 1, y + 1, bottomRightDepth));
                localMesh.append(scaleVertex(x, y + 1, bottomDepth));

                localMesh.append(scaleVertex(x, y, topDepth));
                localMesh.append(scaleVertex(x + 1, y, topRightDepth));
                localMesh.append(scaleVertex(x + 1, y + 1, bottomRightDepth));
            }

            const float rightNextDepth = nextRow[width - 1];
            const float rightDepth = row[width - 1];

            // Close right side
            localMesh.append(scaleVertex(width - 1, y + 1, rightNextDepth));
            localMesh.append(scaleVertex(width - 1, y, rightDepth));
            localMesh.append(scaleVertex(width - 1, y, minThickness));

            localMesh.append(scaleVertex(width - 1, y, minThickness));
            localMesh.append(scaleVertex(width - 1, y + 1, minThickness));
            localMesh.append(scaleVertex(width - 1, y + 1, rightNextDepth));
        }
    #ifdef USE_OPENMP
    }
    
    // Merge thread-local meshes
    for (auto& localMesh : threadMeshes) {
        m_mesh.append(localMesh);
    }
    #endif
}

void MeshGenerator::generateBackside(const QImage& image) {
    const float minThickness = -m_config.minThickness;
    const int height = image.height();
    const int width = image.width();
    
    // Simple flat backside (two triangles)
    m_mesh.append(scaleVertex(0, height - 1, minThickness));
    m_mesh.append(scaleVertex(width - 1, height - 1, minThickness));
    m_mesh.append(scaleVertex(0, 0, minThickness));
    
    m_mesh.append(scaleVertex(width - 1, height - 1, minThickness));
    m_mesh.append(scaleVertex(width - 1, 0, minThickness));
    m_mesh.append(scaleVertex(0, 0, minThickness));
}

void MeshGenerator::generateSegmentedBackside(const QImage& image) {
    // TODO: Implement segmented backside for bending
    // For now, fall back to flat backside
    generateBackside(image);
}

void MeshGenerator::generateFrame(float width, float height) {
    const float minThickness = m_config.minThickness;
    const float depth = m_config.totalThickness - minThickness;
    const float frameSlope = depth * m_config.frameSlopeFactor;
    const float border = m_config.frameBorder;
    
    // This is extracted from the original mainwindow.cpp addFrame function
    // Bottom face
    m_mesh.append(QVector3D(width, height, -minThickness));
    m_mesh.append(QVector3D(0, height, -minThickness));
    m_mesh.append(QVector3D(0, height, depth));
    
    m_mesh.append(QVector3D(width, height, -minThickness));
    m_mesh.append(QVector3D(0, height, depth));
    m_mesh.append(QVector3D(width, height, depth));
    
    // Inner slope
    m_mesh.append(QVector3D(width - border - frameSlope, border + frameSlope, 0));
    m_mesh.append(QVector3D(width - border - frameSlope, height - border - frameSlope, 0));
    m_mesh.append(QVector3D(border + frameSlope, height - border - frameSlope, 0));
    
    m_mesh.append(QVector3D(width - border - frameSlope, border + frameSlope, 0));
    m_mesh.append(QVector3D(border + frameSlope, height - border - frameSlope, 0));
    m_mesh.append(QVector3D(border + frameSlope, border + frameSlope, 0));
    
    // Left face
    m_mesh.append(QVector3D(0, 0, depth));
    m_mesh.append(QVector3D(0, height, depth));
    m_mesh.append(QVector3D(0, height, -minThickness));
    
    m_mesh.append(QVector3D(0, 0, depth));
    m_mesh.append(QVector3D(0, height, -minThickness));
    m_mesh.append(QVector3D(0, 0, -minThickness));
    
    // Top face
    m_mesh.append(QVector3D(0, 0, -minThickness));
    m_mesh.append(QVector3D(width, 0, -minThickness));
    m_mesh.append(QVector3D(width, 0, depth));
    
    m_mesh.append(QVector3D(0, 0, -minThickness));
    m_mesh.append(QVector3D(width, 0, depth));
    m_mesh.append(QVector3D(0, 0, depth));
    
    // Right face
    m_mesh.append(QVector3D(width, 0, -minThickness));
    m_mesh.append(QVector3D(width, height, -minThickness));
    m_mesh.append(QVector3D(width, height, depth));
    
    m_mesh.append(QVector3D(width, 0, -minThickness));
    m_mesh.append(QVector3D(width, height, depth));
    m_mesh.append(QVector3D(width, 0, depth));
    
    // Back face (outer)
    m_mesh.append(QVector3D(0, 0, -minThickness));
    m_mesh.append(QVector3D(0, height, -minThickness));
    m_mesh.append(QVector3D(width, height, -minThickness));
    
    m_mesh.append(QVector3D(0, 0, -minThickness));
    m_mesh.append(QVector3D(width, height, -minThickness));
    m_mesh.append(QVector3D(width, 0, -minThickness));
    
    // Frame inner surfaces
    // Left inner
    m_mesh.append(QVector3D(border, border, depth));
    m_mesh.append(QVector3D(border, height - border, depth));
    m_mesh.append(QVector3D(0, height, depth));
    
    m_mesh.append(QVector3D(border, border, depth));
    m_mesh.append(QVector3D(0, height, depth));
    m_mesh.append(QVector3D(0, 0, depth));
    
    // Right inner
    m_mesh.append(QVector3D(width - border, height - border, depth));
    m_mesh.append(QVector3D(width - border, border, depth));
    m_mesh.append(QVector3D(width, 0, depth));
    
    m_mesh.append(QVector3D(width - border, height - border, depth));
    m_mesh.append(QVector3D(width, 0, depth));
    m_mesh.append(QVector3D(width, height, depth));
    
    // Bottom inner
    m_mesh.append(QVector3D(border, height - border, depth));
    m_mesh.append(QVector3D(width - border, height - border, depth));
    m_mesh.append(QVector3D(width, height, depth));
    
    m_mesh.append(QVector3D(border, height - border, depth));
    m_mesh.append(QVector3D(width, height, depth));
    m_mesh.append(QVector3D(0, height, depth));
    
    // Top inner
    m_mesh.append(QVector3D(width - border, border, depth));
    m_mesh.append(QVector3D(border, border, depth));
    m_mesh.append(QVector3D(0, 0, depth));
    
    m_mesh.append(QVector3D(width - border, border, depth));
    m_mesh.append(QVector3D(0, 0, depth));
    m_mesh.append(QVector3D(width, 0, depth));
    
    // Slope surfaces
    // Left slope
    m_mesh.append(QVector3D(border + frameSlope, border + frameSlope, 0));
    m_mesh.append(QVector3D(border + frameSlope, height - border - frameSlope, 0));
    m_mesh.append(QVector3D(border, height - border, depth));
    
    m_mesh.append(QVector3D(border + frameSlope, border + frameSlope, 0));
    m_mesh.append(QVector3D(border, height - border, depth));
    m_mesh.append(QVector3D(border, border, depth));
    
    // Right slope
    m_mesh.append(QVector3D(width - border - frameSlope, height - border - frameSlope, 0));
    m_mesh.append(QVector3D(width - border - frameSlope, border + frameSlope, 0));
    m_mesh.append(QVector3D(width - border, border, depth));
    
    m_mesh.append(QVector3D(width - border - frameSlope, height - border - frameSlope, 0));
    m_mesh.append(QVector3D(width - border, border, depth));
    m_mesh.append(QVector3D(width - border, height - border, depth));
    
    // Bottom slope
    m_mesh.append(QVector3D(border + frameSlope, height - border - frameSlope, 0));
    m_mesh.append(QVector3D(width - border - frameSlope, height - border - frameSlope, 0));
    m_mesh.append(QVector3D(width - border, height - border, depth));
    
    m_mesh.append(QVector3D(border + frameSlope, height - border - frameSlope, 0));
    m_mesh.append(QVector3D(width - border, height - border, depth));
    m_mesh.append(QVector3D(border, height - border, depth));
    
    // Top slope
    m_mesh.append(QVector3D(width - border - frameSlope, border + frameSlope, 0));
    m_mesh.append(QVector3D(border + frameSlope, border + frameSlope, 0));
    m_mesh.append(QVector3D(border, border, depth));
    
    m_mesh.append(QVector3D(width - border - frameSlope, border + frameSlope, 0));
    m_mesh.append(QVector3D(border, border, depth));
    m_mesh.append(QVector3D(width - border, border, depth));
}

void MeshGenerator::generateStabilizers(float width, float height) {
    const float stabHeight = height * m_config.stabilizerHeightFactor;
    const float stabWidth = std::min(m_border, 4.0f);
    const float depth = stabHeight * 0.5f;
    const float minThickness = m_config.minThickness;
    const float totalThickness = m_config.totalThickness;
    const float zDelta = m_config.permanentStabilizers ? 1.0f : 0.0f;
    
    // Generate left stabilizer
    addSingleStabilizer(0.0f, stabHeight, depth, minThickness, totalThickness, zDelta);
    
    // Generate right stabilizer
    addSingleStabilizer(width - stabWidth, stabHeight, depth, minThickness, totalThickness, zDelta);
    
    qInfo() << "Stabilizers generated: height=" << stabHeight << "mm, width=" << stabWidth << "mm";
}

void MeshGenerator::addSingleStabilizer(float x, float stabHeight, float depth, 
                                        float minThickness, float totalThickness, float zDelta) {
    const float stabWidth = std::min(m_border, 4.0f);
    const float h = stabHeight;  // height of the stabilizer
    
    // Front stabilizer (positive Z direction)
    float z = totalThickness - minThickness;
    
    // Front face - left side
    m_mesh.append(QVector3D(x, 0, z + 1 - zDelta));
    m_mesh.append(QVector3D(x, 0, z + depth));
    m_mesh.append(QVector3D(x, h, z + 3));
    
    m_mesh.append(QVector3D(x, h, z + 3));
    m_mesh.append(QVector3D(x, h, z + 1 - zDelta));
    m_mesh.append(QVector3D(x, h - 1, z + 1 - zDelta));
    
    m_mesh.append(QVector3D(x, h, z + 3));
    m_mesh.append(QVector3D(x, h - 1, z + 1 - zDelta));
    m_mesh.append(QVector3D(x, 0, z + 1 - zDelta));
    
    // Front face - right side
    m_mesh.append(QVector3D(x + stabWidth, h, z + 3));
    m_mesh.append(QVector3D(x + stabWidth, 0, z + depth));
    m_mesh.append(QVector3D(x + stabWidth, 0, z + 1 - zDelta));
    
    m_mesh.append(QVector3D(x + stabWidth, h - 1, z + 1 - zDelta));
    m_mesh.append(QVector3D(x + stabWidth, h, z + 1 - zDelta));
    m_mesh.append(QVector3D(x + stabWidth, h, z + 3));
    
    m_mesh.append(QVector3D(x + stabWidth, 0, z + 1 - zDelta));
    m_mesh.append(QVector3D(x + stabWidth, h - 1, z + 1 - zDelta));
    m_mesh.append(QVector3D(x + stabWidth, h, z + 3));
    
    // Top faces
    m_mesh.append(QVector3D(x + 1, h, z + 1 - zDelta));
    m_mesh.append(QVector3D(x, h, z + 1 - zDelta));
    m_mesh.append(QVector3D(x, h, z + 3));
    
    m_mesh.append(QVector3D(x, h, z + 3));
    m_mesh.append(QVector3D(x + stabWidth, h, z + 3));
    m_mesh.append(QVector3D(x + stabWidth, h, z + 1 - zDelta));
    
    m_mesh.append(QVector3D(x + stabWidth - 1, h, z + 1 - zDelta));
    m_mesh.append(QVector3D(x + 1, h, z + 1 - zDelta));
    m_mesh.append(QVector3D(x, h, z + 3));
    
    m_mesh.append(QVector3D(x, h, z + 3));
    m_mesh.append(QVector3D(x + stabWidth, h, z + 1 - zDelta));
    m_mesh.append(QVector3D(x + stabWidth - 1, h, z + 1 - zDelta));
    
    // Bottom face
    m_mesh.append(QVector3D(x, 0, z + depth));
    m_mesh.append(QVector3D(x, 0, z + 1 - zDelta));
    m_mesh.append(QVector3D(x + stabWidth, 0, z + 1 - zDelta));
    
    m_mesh.append(QVector3D(x, 0, z + depth));
    m_mesh.append(QVector3D(x + stabWidth, 0, z + 1 - zDelta));
    m_mesh.append(QVector3D(x + stabWidth, 0, z + depth));
    
    // Sloped front face (triangular)
    m_mesh.append(QVector3D(x, h, z + 3));
    m_mesh.append(QVector3D(x, 0, z + depth));
    m_mesh.append(QVector3D(x + stabWidth, 0, z + depth));
    
    m_mesh.append(QVector3D(x, h, z + 3));
    m_mesh.append(QVector3D(x + stabWidth, 0, z + depth));
    m_mesh.append(QVector3D(x + stabWidth, h, z + 3));
    
    // Inner connection faces
    m_mesh.append(QVector3D(x + 1, h - 1, z + 1 - zDelta));
    m_mesh.append(QVector3D(x + 1, h, z + 1 - zDelta));
    m_mesh.append(QVector3D(x + stabWidth - 1, h, z + 1 - zDelta));
    
    m_mesh.append(QVector3D(x + 1, h - 1, z + 1 - zDelta));
    m_mesh.append(QVector3D(x + stabWidth - 1, h, z + 1 - zDelta));
    m_mesh.append(QVector3D(x + stabWidth - 1, h - 1, z + 1 - zDelta));
    
    // Back stabilizer (negative Z direction)
    z = -minThickness;
    
    // Back face - right side
    m_mesh.append(QVector3D(x + stabWidth, 0, z - 1 + zDelta));
    m_mesh.append(QVector3D(x + stabWidth, 0, z - depth));
    m_mesh.append(QVector3D(x + stabWidth, h, z - 3));
    
    m_mesh.append(QVector3D(x + stabWidth, h, z - 3));
    m_mesh.append(QVector3D(x + stabWidth, h, z - 1 + zDelta));
    m_mesh.append(QVector3D(x + stabWidth, h - 1, z - 1 + zDelta));
    
    m_mesh.append(QVector3D(x + stabWidth, h, z - 3));
    m_mesh.append(QVector3D(x + stabWidth, h - 1, z - 1 + zDelta));
    m_mesh.append(QVector3D(x + stabWidth, 0, z - 1 + zDelta));
    
    // Back face - left side
    m_mesh.append(QVector3D(x, h, z - 3));
    m_mesh.append(QVector3D(x, 0, z - depth));
    m_mesh.append(QVector3D(x, 0, z - 1 + zDelta));
    
    m_mesh.append(QVector3D(x, h - 1, z - 1 + zDelta));
    m_mesh.append(QVector3D(x, h, z - 1 + zDelta));
    m_mesh.append(QVector3D(x, h, z - 3));
    
    m_mesh.append(QVector3D(x, 0, z - 1 + zDelta));
    m_mesh.append(QVector3D(x, h - 1, z - 1 + zDelta));
    m_mesh.append(QVector3D(x, h, z - 3));
    
    // Back top faces
    m_mesh.append(QVector3D(x + stabWidth - 1, h, z - 1 + zDelta));
    m_mesh.append(QVector3D(x + stabWidth, h, z - 1 + zDelta));
    m_mesh.append(QVector3D(x + stabWidth, h, z - 3));
    
    m_mesh.append(QVector3D(x + stabWidth, h, z - 3));
    m_mesh.append(QVector3D(x, h, z - 3));
    m_mesh.append(QVector3D(x, h, z - 1 + zDelta));
    
    m_mesh.append(QVector3D(x + 1, h, z - 1 + zDelta));
    m_mesh.append(QVector3D(x + stabWidth - 1, h, z - 1 + zDelta));
    m_mesh.append(QVector3D(x + stabWidth, h, z - 3));
    
    m_mesh.append(QVector3D(x + stabWidth, h, z - 3));
    m_mesh.append(QVector3D(x, h, z - 1 + zDelta));
    m_mesh.append(QVector3D(x + 1, h, z - 1 + zDelta));
    
    // Back bottom face
    m_mesh.append(QVector3D(x + stabWidth, 0, z - depth));
    m_mesh.append(QVector3D(x + stabWidth, 0, z - 1 + zDelta));
    m_mesh.append(QVector3D(x, 0, z - 1 + zDelta));
    
    m_mesh.append(QVector3D(x + stabWidth, 0, z - depth));
    m_mesh.append(QVector3D(x, 0, z - 1 + zDelta));
    m_mesh.append(QVector3D(x, 0, z - depth));
    
    // Back sloped face
    m_mesh.append(QVector3D(x + stabWidth, h, z - 3));
    m_mesh.append(QVector3D(x + stabWidth, 0, z - depth));
    m_mesh.append(QVector3D(x, 0, z - depth));
    
    m_mesh.append(QVector3D(x + stabWidth, h, z - 3));
    m_mesh.append(QVector3D(x, 0, z - depth));
    m_mesh.append(QVector3D(x, h, z - 3));
    
    // Back inner faces
    m_mesh.append(QVector3D(x + stabWidth - 1, h - 1, z - 1 + zDelta));
    m_mesh.append(QVector3D(x + stabWidth - 1, h, z - 1 + zDelta));
    m_mesh.append(QVector3D(x + 1, h, z - 1 + zDelta));
    
    m_mesh.append(QVector3D(x + stabWidth - 1, h - 1, z - 1 + zDelta));
    m_mesh.append(QVector3D(x + 1, h, z - 1 + zDelta));
    m_mesh.append(QVector3D(x + 1, h - 1, z - 1 + zDelta));
}

void MeshGenerator::generateHangers(float width, float height) {
    const int noOfHangers = m_config.hangerCount;
    const float xDelta = (width / noOfHangers) / 2.0f;
    float x = xDelta - 4.5f; // 4.5 is half the width of a hanger
    
    for (int i = 0; i < noOfHangers; ++i) {
        // Front face of hanger
        m_mesh.append(QVector3D(x + 3, height, 0));
        m_mesh.append(QVector3D(x, height, 0));
        m_mesh.append(QVector3D(x + 3, height + 3, 0));
        
        m_mesh.append(QVector3D(x + 3, height + 3, 0));
        m_mesh.append(QVector3D(x + 6, height + 3, 0));
        m_mesh.append(QVector3D(x + 9, height, 0));
        
        // Loop hole
        m_mesh.append(QVector3D(x + 9, height, 0));
        m_mesh.append(QVector3D(x + 6, height, 0));
        m_mesh.append(QVector3D(x + 5, height + 1, 0));
        
        m_mesh.append(QVector3D(x + 4, height + 1, 0));
        m_mesh.append(QVector3D(x + 3, height, 0));
        m_mesh.append(QVector3D(x + 3, height + 3, 0));
        
        m_mesh.append(QVector3D(x + 3, height + 3, 0));
        m_mesh.append(QVector3D(x + 9, height, 0));
        m_mesh.append(QVector3D(x + 5, height + 1, 0));
        
        m_mesh.append(QVector3D(x + 3, height + 3, 0));
        m_mesh.append(QVector3D(x + 5, height + 1, 0));
        m_mesh.append(QVector3D(x + 4, height + 1, 0));
        
        // Back face of hanger (z = 2)
        m_mesh.append(QVector3D(x + 3, height + 3, 2));
        m_mesh.append(QVector3D(x, height, 2));
        m_mesh.append(QVector3D(x + 3, height, 2));
        
        m_mesh.append(QVector3D(x + 3, height + 3, 2));
        m_mesh.append(QVector3D(x + 3, height, 2));
        m_mesh.append(QVector3D(x + 4, height + 1, 2));
        
        m_mesh.append(QVector3D(x + 9, height, 2));
        m_mesh.append(QVector3D(x + 6, height + 3, 2));
        m_mesh.append(QVector3D(x + 3, height + 3, 2));
        
        m_mesh.append(QVector3D(x + 5, height + 1, 2));
        m_mesh.append(QVector3D(x + 6, height, 2));
        m_mesh.append(QVector3D(x + 9, height, 2));
        
        m_mesh.append(QVector3D(x + 3, height + 3, 2));
        m_mesh.append(QVector3D(x + 4, height + 1, 2));
        m_mesh.append(QVector3D(x + 5, height + 1, 2));
        
        m_mesh.append(QVector3D(x + 5, height + 1, 2));
        m_mesh.append(QVector3D(x + 9, height, 2));
        m_mesh.append(QVector3D(x + 3, height + 3, 2));
        
        // Side faces connecting front and back
        // Inner loop sides
        m_mesh.append(QVector3D(x + 5, height + 1, 0));
        m_mesh.append(QVector3D(x + 6, height, 0));
        m_mesh.append(QVector3D(x + 6, height, 2));
        
        m_mesh.append(QVector3D(x + 5, height + 1, 0));
        m_mesh.append(QVector3D(x + 6, height, 2));
        m_mesh.append(QVector3D(x + 5, height + 1, 2));
        
        // Top arch
        m_mesh.append(QVector3D(x + 6, height + 3, 0));
        m_mesh.append(QVector3D(x + 3, height + 3, 0));
        m_mesh.append(QVector3D(x + 3, height + 3, 2));
        
        m_mesh.append(QVector3D(x + 6, height + 3, 0));
        m_mesh.append(QVector3D(x + 3, height + 3, 2));
        m_mesh.append(QVector3D(x + 6, height + 3, 2));
        
        x += xDelta * 2;
    }
}

QVector3D MeshGenerator::scaleVertex(float x, float y, float z) const {
    return QVector3D(
        x * m_widthFactor + m_border,
        y * m_widthFactor + m_border,
        z
    );
}
} // namespace LithoMaker
