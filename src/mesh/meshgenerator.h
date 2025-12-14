/**
 * @file meshgenerator.h
 * @brief Base mesh generator for lithophanes
 *
 * Copyright 2021-2024 Lars Muldjord / Contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <QVector3D>
#include <QImage>
#include <QList>
#include <functional>

namespace LithoMaker {

/**
 * @brief Configuration for mesh generation
 */
struct MeshConfig {
    float minThickness{0.8f};    ///< Minimum lithophane thickness (mm)
    float totalThickness{4.0f};  ///< Total thickness including min (mm)
    float frameBorder{3.0f};     ///< Frame border width (mm)
    float width{200.0f};         ///< Total width including frame (mm)
    float frameSlopeFactor{0.75f};
    
    // Stabilizers
    bool enableStabilizers{true};
    bool permanentStabilizers{false};
    float stabilizerThreshold{60.0f}; ///< Min height before adding stabilizers
    float stabilizerHeightFactor{0.15f};
    
    // Hangers
    bool enableHangers{true};
    int hangerCount{2};
    
    // Segmentation (for bending)
    bool enableSegmentation{false};
    int backsideSegments{1};
    int frameSegments{1};
};

/**
 * @brief Progress callback type
 * @param current Current progress value
 * @param total Total progress value
 */
using ProgressCallback = std::function<void(int current, int total)>;

/**
 * @brief Complete lithophane mesh generator
 *
 * Generates the full 3D mesh including lithophane surface,
 * frame, stabilizers, and hangers.
 */
class MeshGenerator {
public:
    MeshGenerator() = default;
    explicit MeshGenerator(const MeshConfig& config);

    /**
     * @brief Set the mesh configuration
     */
    void setConfig(const MeshConfig& config);
    
    /**
     * @brief Get current configuration
     */
    const MeshConfig& config() const { return m_config; }

    /**
     * @brief Generate the complete mesh from an image
     * @param image Grayscale image (should already be processed)
     * @param progressCallback Optional callback for progress reporting
     * @return List of vertices (triangles, 3 vertices per triangle)
     */
    QList<QVector3D> generate(const QImage& image, 
                              ProgressCallback progressCallback = nullptr);

    /**
     * @brief Get the last generated mesh
     * @return List of vertices
     */
    const QList<QVector3D>& mesh() const { return m_mesh; }

    /**
     * @brief Get the total dimensions of last generated mesh
     */
    QSizeF meshDimensions() const { return m_meshDimensions; }

private:
    // Mesh generation helpers
    void generateLithophane(const QImage& image);
    void generateBackside(const QImage& image);
    void generateFrame(float width, float height);
    void generateStabilizers(float width, float height);
    void addSingleStabilizer(float x, float stabHeight, float depth,
                              float minThickness, float totalThickness, float zDelta);
    void generateHangers(float width, float height);
    void generateSegmentedBackside(const QImage& image);

    // Vertex helpers
    QVector3D scaleVertex(float x, float y, float z) const;

    MeshConfig m_config;
    QList<QVector3D> m_mesh;
    QSizeF m_meshDimensions;
    
    // Computed values during generation
    float m_widthFactor{1.0f};
    float m_depthFactor{1.0f};
    float m_border{0.0f};
};

} // namespace LithoMaker
