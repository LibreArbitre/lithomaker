/**
 * @file mainwindow.h
 * @brief Main application window (UI only, no mesh logic)
 *
 * Copyright 2021-2024 Lars Muldjord / Contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <QMainWindow>
#include <QProgressBar>
#include <QLabel>
#include <memory>

#include "mesh/meshgenerator.h"

// Forward declarations
class QLineEdit;
class QPushButton;
class QComboBox;

namespace LithoMaker {

#ifndef BUILD_WASM
class PreviewWidget;
#endif
class Slider;

/**
 * @brief Main application window
 *
 * Handles UI and coordinates mesh generation and export.
 * All mesh logic is delegated to MeshGenerator and Exporter classes.
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private slots:
    void onRenderClicked();
    void onInputFileSelect();
    void onOutputFileSelect();
    void onExportFormatChanged(int index);
    void showPreferences();
    void showAbout();
    void updatePreview();

private:
    void createWidgets();
    void createMenus();
    void loadSettings();
    void saveSettings();
    void setInputFile(const QString& path);
    void doExport();

    // UI widgets
    Slider* m_minThicknessSlider{nullptr};
    Slider* m_totalThicknessSlider{nullptr};
    Slider* m_borderSlider{nullptr};
    Slider* m_widthSlider{nullptr};

    QLineEdit* m_inputLineEdit{nullptr};
    QPushButton* m_inputButton{nullptr};
    QLineEdit* m_outputLineEdit{nullptr};
    QPushButton* m_outputButton{nullptr};
    QComboBox* m_exportFormatCombo{nullptr};
    QPushButton* m_renderButton{nullptr};
    QProgressBar* m_progressBar{nullptr};
    QLabel* m_statusLabel{nullptr};
#ifndef BUILD_WASM
    PreviewWidget* m_previewWidget{nullptr};
#endif

    // Mesh generation
    std::unique_ptr<MeshGenerator> m_meshGenerator;
    QList<QVector3D> m_currentMesh;
};

} // namespace LithoMaker
