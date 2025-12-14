/**
 * @file previewwidget.h
 * @brief OpenGL 3D preview widget for lithophane visualization
 *
 * Copyright 2021-2024 Lars Muldjord / Contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QVector3D>
#include <QMatrix4x4>
#include <QList>
#include <QMouseEvent>
#include <QWheelEvent>

class QLabel;

namespace LithoMaker {

/**
 * @brief OpenGL-based 3D preview widget
 *
 * Displays the lithophane mesh with mouse-controlled rotation and zoom.
 */
class PreviewWidget : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT

public:
    explicit PreviewWidget(QWidget* parent = nullptr);
    ~PreviewWidget() override;

    /**
     * @brief Update the displayed mesh
     * @param mesh List of vertices (triangles, 3 per triangle)
     */
    void setMesh(QList<QVector3D> mesh);

    /**
     * @brief Clear the mesh display
     */
    void clear();

    /**
     * @brief Reset camera to default position
     */
    void resetCamera();

    /**
     * @brief Set dark theme mode
     */
    void setDarkTheme(bool dark);

signals:
    void meshUpdated(int triangleCount);

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    void setupShaders();
    void updateMeshBuffer();
    void calculateNormals();

    // Mesh data
    QList<QVector3D> m_mesh;
    QList<QVector3D> m_normals;
    QVector3D m_meshCenter;
    float m_meshRadius{100.0f};

    // OpenGL objects
    QOpenGLShaderProgram* m_program{nullptr};
    QOpenGLBuffer m_vertexBuffer{QOpenGLBuffer::VertexBuffer};
    QOpenGLBuffer m_normalBuffer{QOpenGLBuffer::VertexBuffer};
    QOpenGLVertexArrayObject m_vao;
    bool m_meshDirty{false};

    // Camera/View
    float m_rotationX{-20.0f};   // Slight tilt forward
    float m_rotationY{0.0f};     // Face forward
    float m_zoom{1.0f};
    QPointF m_lastMousePos;

    // Appearance
    bool m_darkTheme{true};
    QVector3D m_lightColor{1.0f, 1.0f, 0.95f};
    QVector3D m_meshColor{0.95f, 0.93f, 0.88f}; // Warm white like PLA
};

} // namespace LithoMaker
