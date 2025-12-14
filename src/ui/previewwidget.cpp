/**
 * @file previewwidget.cpp
 * @brief OpenGL 3D preview widget implementation
 *
 * Copyright 2021-2024 Lars Muldjord / Contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "previewwidget.h"

#include <QDebug>
#include <QtMath>
#include <utility>

namespace LithoMaker {

// Vertex shader
static const char* vertexShaderSource = R"(
    #version 330 core
    layout(location = 0) in vec3 position;
    layout(location = 1) in vec3 normal;
    
    uniform mat4 mvp;
    uniform mat4 model;
    uniform mat3 normalMatrix;
    
    out vec3 fragNormal;
    out vec3 fragPos;
    
    void main() {
        gl_Position = mvp * vec4(position, 1.0);
        fragPos = vec3(model * vec4(position, 1.0));
        fragNormal = normalMatrix * normal;
    }
)";

// Fragment shader with Phong lighting
static const char* fragmentShaderSource = R"(
    #version 330 core
    in vec3 fragNormal;
    in vec3 fragPos;
    
    uniform vec3 lightPos;
    uniform vec3 lightColor;
    uniform vec3 objectColor;
    uniform vec3 viewPos;
    
    out vec4 fragColor;
    
    void main() {
        // Ambient
        float ambientStrength = 0.3;
        vec3 ambient = ambientStrength * lightColor;
        
        // Diffuse
        vec3 norm = normalize(fragNormal);
        vec3 lightDir = normalize(lightPos - fragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * lightColor;
        
        // Specular
        float specularStrength = 0.3;
        vec3 viewDir = normalize(viewPos - fragPos);
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
        vec3 specular = specularStrength * spec * lightColor;
        
        vec3 result = (ambient + diffuse + specular) * objectColor;
        fragColor = vec4(result, 1.0);
    }
)";

PreviewWidget::PreviewWidget(QWidget* parent)
    : QOpenGLWidget(parent)
{
    setMinimumSize(300, 300);
    setFocusPolicy(Qt::StrongFocus);
    
    // Enable multisampling for smoother edges
    QSurfaceFormat format;
    format.setSamples(4);
    format.setDepthBufferSize(24);
    setFormat(format);
}

PreviewWidget::~PreviewWidget() {
    makeCurrent();
    m_vertexBuffer.destroy();
    m_normalBuffer.destroy();
    m_vao.destroy();
    delete m_program;
    doneCurrent();
}

void PreviewWidget::initializeGL() {
    initializeOpenGLFunctions();
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    
    // Background color
    if (m_darkTheme) {
        glClearColor(0.12f, 0.12f, 0.14f, 1.0f);
    } else {
        glClearColor(0.94f, 0.94f, 0.94f, 1.0f);
    }
    
    setupShaders();
    
    m_vao.create();
    m_vertexBuffer.create();
    m_normalBuffer.create();
}

void PreviewWidget::setupShaders() {
    m_program = new QOpenGLShaderProgram(this);
    
    if (!m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource)) {
        qWarning() << "Vertex shader compilation failed:" << m_program->log();
    }
    
    if (!m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource)) {
        qWarning() << "Fragment shader compilation failed:" << m_program->log();
    }
    
    if (!m_program->link()) {
        qWarning() << "Shader linking failed:" << m_program->log();
    }
}

void PreviewWidget::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);
}

void PreviewWidget::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    if (m_mesh.isEmpty() || !m_program->isLinked()) {
        return;
    }
    
    if (m_meshDirty) {
        updateMeshBuffer();
        m_meshDirty = false;
    }
    
    m_program->bind();
    
    // Calculate matrices
    QMatrix4x4 model;
    model.translate(-m_meshCenter);
    model.rotate(m_rotationX, 1.0f, 0.0f, 0.0f);
    model.rotate(m_rotationY, 0.0f, 1.0f, 0.0f);
    
    QMatrix4x4 view;
    float distance = m_meshRadius * 2.5f / m_zoom;
    view.lookAt(QVector3D(0, 0, distance), QVector3D(0, 0, 0), QVector3D(0, 1, 0));
    
    QMatrix4x4 projection;
    float aspect = float(width()) / float(height());
    projection.perspective(45.0f, aspect, 0.1f, m_meshRadius * 10.0f);
    
    QMatrix4x4 mvp = projection * view * model;
    QMatrix3x3 normalMatrix = model.normalMatrix();
    
    m_program->setUniformValue("mvp", mvp);
    m_program->setUniformValue("model", model);
    m_program->setUniformValue("normalMatrix", normalMatrix);
    m_program->setUniformValue("lightPos", QVector3D(m_meshRadius * 2, m_meshRadius * 2, m_meshRadius * 3));
    m_program->setUniformValue("lightColor", m_lightColor);
    m_program->setUniformValue("objectColor", m_meshColor);
    m_program->setUniformValue("viewPos", QVector3D(0, 0, distance));
    
    m_vao.bind();
    glDrawArrays(GL_TRIANGLES, 0, m_mesh.size());
    m_vao.release();
    
    m_program->release();
}

void PreviewWidget::setMesh(QList<QVector3D> mesh) {
    m_mesh = std::move(mesh);
    m_meshDirty = true;

    // Calculate bounding box and center
    if (!m_mesh.isEmpty()) {
        QVector3D minBound(std::numeric_limits<float>::max(),
                          std::numeric_limits<float>::max(),
                          std::numeric_limits<float>::max());
        QVector3D maxBound(std::numeric_limits<float>::lowest(),
                          std::numeric_limits<float>::lowest(),
                          std::numeric_limits<float>::lowest());

        for (const auto& v : m_mesh) {
            minBound.setX(std::min(minBound.x(), v.x()));
            minBound.setY(std::min(minBound.y(), v.y()));
            minBound.setZ(std::min(minBound.z(), v.z()));
            maxBound.setX(std::max(maxBound.x(), v.x()));
            maxBound.setY(std::max(maxBound.y(), v.y()));
            maxBound.setZ(std::max(maxBound.z(), v.z()));
        }

        m_meshCenter = (minBound + maxBound) / 2.0f;
        m_meshRadius = (maxBound - minBound).length() / 2.0f;

        calculateNormals();
    }

    emit meshUpdated(m_mesh.size() / 3);
    update();  // Trigger repaint

    qInfo() << "Preview updated:" << (m_mesh.size() / 3) << "triangles";
}

void PreviewWidget::calculateNormals() {
    m_normals.clear();
    m_normals.reserve(m_mesh.size());
    
    // Calculate per-face normals (flat shading)
    for (int i = 0; i < m_mesh.size(); i += 3) {
        QVector3D v0 = m_mesh[i];
        QVector3D v1 = m_mesh[i + 1];
        QVector3D v2 = m_mesh[i + 2];
        
        QVector3D edge1 = v1 - v0;
        QVector3D edge2 = v2 - v0;
        QVector3D normal = QVector3D::crossProduct(edge1, edge2).normalized();
        
        // Same normal for all 3 vertices of the triangle
        m_normals.append(normal);
        m_normals.append(normal);
        m_normals.append(normal);
    }
}

void PreviewWidget::updateMeshBuffer() {
    if (m_mesh.isEmpty()) return;
    
    m_vao.bind();
    
    // Upload vertex positions
    m_vertexBuffer.bind();
    m_vertexBuffer.allocate(m_mesh.constData(), m_mesh.size() * sizeof(QVector3D));
    m_program->enableAttributeArray(0);
    m_program->setAttributeBuffer(0, GL_FLOAT, 0, 3, sizeof(QVector3D));
    
    // Upload normals
    m_normalBuffer.bind();
    m_normalBuffer.allocate(m_normals.constData(), m_normals.size() * sizeof(QVector3D));
    m_program->enableAttributeArray(1);
    m_program->setAttributeBuffer(1, GL_FLOAT, 0, 3, sizeof(QVector3D));
    
    m_vao.release();
}

void PreviewWidget::clear() {
    m_mesh.clear();
    m_normals.clear();
    m_meshDirty = true;
    update();
}

void PreviewWidget::resetCamera() {
    m_rotationX = 30.0f;
    m_rotationY = -45.0f;
    m_zoom = 1.0f;
    update();
}

void PreviewWidget::setDarkTheme(bool dark) {
    m_darkTheme = dark;
    if (context()) {
        makeCurrent();
        if (dark) {
            glClearColor(0.12f, 0.12f, 0.14f, 1.0f);
        } else {
            glClearColor(0.94f, 0.94f, 0.94f, 1.0f);
        }
        doneCurrent();
    }
    update();
}

void PreviewWidget::mousePressEvent(QMouseEvent* event) {
    m_lastMousePos = event->position();
}

void PreviewWidget::mouseMoveEvent(QMouseEvent* event) {
    QPointF delta = event->position() - m_lastMousePos;
    m_lastMousePos = event->position();
    
    if (event->buttons() & Qt::LeftButton) {
        // Rotation
        m_rotationY += delta.x() * 0.5f;
        m_rotationX += delta.y() * 0.5f;
        
        // Clamp vertical rotation
        m_rotationX = qBound(-90.0f, m_rotationX, 90.0f);
        
        update();
    }
}

void PreviewWidget::wheelEvent(QWheelEvent* event) {
    float delta = event->angleDelta().y() / 120.0f;
    m_zoom *= (1.0f + delta * 0.1f);
    m_zoom = qBound(0.1f, m_zoom, 10.0f);
    update();
}

} // namespace LithoMaker
