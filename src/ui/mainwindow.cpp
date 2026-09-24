/**
 * @file mainwindow.cpp
 * @brief Main application window implementation
 *
 * Copyright 2021-2024 Lars Muldjord / Contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "mainwindow.h"
#ifndef BUILD_WASM
#include "previewwidget.h"
#endif
#include "widgets/slider.h"
#include "aboutbox.h"
#include "configdialog.h"

#include "core/settings.h"
#include "core/imageloader.h"
#include "export/stlexporter.h"
#include "export/objexporter.h"
#include "export/threemfexporter.h"
#include "version.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QSplitter>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QProgressBar>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QDebug>
#include <QApplication>
#include <QStatusBar>
#include <QUuid>
#include <utility>

namespace LithoMaker {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_meshGenerator(std::make_unique<MeshGenerator>())
{
    setWindowTitle(QString("LithoMaker v%1").arg(LITHOMAKER_VERSION));
    setAcceptDrops(true);
    setMinimumSize(900, 600);

    createWidgets();
    createMenus();
    loadSettings();

    // Show preferences on first run
    auto& settings = Settings::instance();
    if (settings.allKeys().isEmpty()) {
#ifndef BUILD_WASM
        showPreferences();
#endif
    }
}

MainWindow::~MainWindow() {
    saveSettings();
#ifdef BUILD_WASM
    if (!m_wasmInputFilePath.isEmpty()) QFile::remove(m_wasmInputFilePath);
#endif
}

void MainWindow::createWidgets() {
    auto* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // Main layout with splitter
    auto* mainLayout = new QHBoxLayout(centralWidget);
    auto* splitter = new QSplitter(Qt::Horizontal, this);

    // Left panel - controls
    auto* controlsWidget = new QWidget();
    auto* controlsLayout = new QVBoxLayout(controlsWidget);
    controlsLayout->setContentsMargins(10, 10, 10, 10);

    // Thickness controls
    auto* thicknessGroup = new QWidget();
    auto* thicknessLayout = new QGridLayout(thicknessGroup);
    thicknessLayout->setContentsMargins(0, 0, 0, 0);

    thicknessLayout->addWidget(new QLabel(tr("Minimum thickness (mm):")), 0, 0);
    m_minThicknessSlider = new Slider("render", "minThickness", 8, 100, 8, 10);
    connect(m_minThicknessSlider, &Slider::valueChanged, this, [this] {
        invalidateMesh(tr("Render settings changed. Click Preview to regenerate."));
    });
    thicknessLayout->addWidget(m_minThicknessSlider, 0, 1);

    thicknessLayout->addWidget(new QLabel(tr("Total thickness (mm):")), 1, 0);
    m_totalThicknessSlider = new Slider("render", "totalThickness", 20, 150, 40, 10);
    connect(m_totalThicknessSlider, &Slider::valueChanged, this, [this] {
        invalidateMesh(tr("Render settings changed. Click Preview to regenerate."));
    });
    thicknessLayout->addWidget(m_totalThicknessSlider, 1, 1);

    thicknessLayout->addWidget(new QLabel(tr("Frame border (mm):")), 2, 0);
    m_borderSlider = new Slider("render", "frameBorder", 20, 500, 30, 10);
    connect(m_borderSlider, &Slider::valueChanged, this, [this] {
        invalidateMesh(tr("Render settings changed. Click Preview to regenerate."));
    });
    thicknessLayout->addWidget(m_borderSlider, 2, 1);

    thicknessLayout->addWidget(new QLabel(tr("Width (mm):")), 3, 0);
    m_widthSlider = new Slider("render", "width", 200, 4000, 2000, 10);
    connect(m_widthSlider, &Slider::valueChanged, this, [this] {
        invalidateMesh(tr("Render settings changed. Click Preview to regenerate."));
    });
    thicknessLayout->addWidget(m_widthSlider, 3, 1);

    controlsLayout->addWidget(thicknessGroup);

    // Input file
    controlsLayout->addWidget(new QLabel(tr("Input image file:")));
    auto* inputLayout = new QHBoxLayout();
    m_inputLineEdit = new QLineEdit();
    m_inputLineEdit->setPlaceholderText(tr("Drag & drop or click to select..."));
#ifdef BUILD_WASM
    m_inputLineEdit->setReadOnly(true);
#endif
    connect(m_inputLineEdit, &QLineEdit::textChanged, this, [this] {
        invalidateMesh(tr("Input image changed. Click Preview to regenerate."));
    });
    m_inputButton = new QPushButton(tr("..."));
    m_inputButton->setMaximumWidth(40);
    connect(m_inputButton, &QPushButton::clicked, this, &MainWindow::onInputFileSelect);
    inputLayout->addWidget(m_inputLineEdit);
    inputLayout->addWidget(m_inputButton);
    controlsLayout->addLayout(inputLayout);

    // Output file
    controlsLayout->addWidget(new QLabel(tr("Output file:")));
    auto* outputLayout = new QHBoxLayout();
    m_outputLineEdit = new QLineEdit("lithophane.stl");
    m_outputButton = new QPushButton(tr("..."));
    m_outputButton->setMaximumWidth(40);
    connect(m_outputButton, &QPushButton::clicked, this, &MainWindow::onOutputFileSelect);
#ifdef BUILD_WASM
    m_outputButton->hide();
    m_outputLineEdit->setPlaceholderText(tr("Downloaded file name"));
#endif
    outputLayout->addWidget(m_outputLineEdit);
    outputLayout->addWidget(m_outputButton);
    controlsLayout->addLayout(outputLayout);

    // Export format
    auto* formatLayout = new QHBoxLayout();
    formatLayout->addWidget(new QLabel(tr("Export format:")));
    m_exportFormatCombo = new QComboBox();
    m_exportFormatCombo->addItem("STL (Binary)", "stl_bin");
    m_exportFormatCombo->addItem("STL (ASCII)", "stl_ascii");
    m_exportFormatCombo->addItem("OBJ", "obj");
#ifndef BUILD_WASM
    m_exportFormatCombo->addItem("3MF", "3mf");
#endif
    connect(m_exportFormatCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &MainWindow::onExportFormatChanged);
    formatLayout->addWidget(m_exportFormatCombo);
    formatLayout->addStretch();
    controlsLayout->addLayout(formatLayout);

    // Flip vertical checkbox
    controlsLayout->addSpacing(10);
    m_flipVerticalCheckbox = new QCheckBox(tr("Flip image vertically"));
    connect(m_flipVerticalCheckbox, &QCheckBox::toggled, 
            this, &MainWindow::onFlipChanged);
    controlsLayout->addWidget(m_flipVerticalCheckbox);

    // Preview button
    m_previewButton = new QPushButton(tr("Preview"));
    m_previewButton->setMinimumHeight(40);
    connect(m_previewButton, &QPushButton::clicked, this, &MainWindow::onPreviewClicked);
    controlsLayout->addWidget(m_previewButton);

    // Export button (disabled until mesh is ready)
    m_exportButton = new QPushButton(tr("Export"));
    m_exportButton->setMinimumHeight(40);
    m_exportButton->setEnabled(false);
    m_exportButton->setStyleSheet("QPushButton { font-weight: bold; }");
    connect(m_exportButton, &QPushButton::clicked, this, &MainWindow::onExportClicked);
    controlsLayout->addWidget(m_exportButton);

    // Progress bar
    m_progressBar = new QProgressBar();
    m_progressBar->setMinimum(0);
    m_progressBar->setMaximum(100);
    m_progressBar->setValue(0);
    m_progressBar->setVisible(false);
    controlsLayout->addWidget(m_progressBar);

    controlsLayout->addStretch();

#ifndef BUILD_WASM
    // Right panel - 3D preview
    m_previewWidget = new PreviewWidget();

    splitter->addWidget(controlsWidget);
    splitter->addWidget(m_previewWidget);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 2);
#else
    // WASM: No 3D preview, just use controls
    splitter->addWidget(controlsWidget);
    auto* placeholder = new QLabel(tr("3D Preview not available in browser version"));
    placeholder->setAlignment(Qt::AlignCenter);
    splitter->addWidget(placeholder);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 1);
#endif

    mainLayout->addWidget(splitter);

    // Status bar
    m_statusLabel = new QLabel(tr("Ready"));
    statusBar()->addWidget(m_statusLabel);
}

void MainWindow::createMenus() {
    // File menu
    auto* fileMenu = menuBar()->addMenu(tr("&File"));
    
    auto* quitAction = fileMenu->addAction(tr("&Quit"));
    quitAction->setShortcut(QKeySequence::Quit);
    connect(quitAction, &QAction::triggered, this, &QMainWindow::close);

    // Options menu
    auto* optionsMenu = menuBar()->addMenu(tr("&Options"));
    
    auto* prefsAction = optionsMenu->addAction(tr("Edit &Preferences..."));
    prefsAction->setShortcut(QKeySequence::Preferences);
    connect(prefsAction, &QAction::triggered, this, &MainWindow::showPreferences);

    // Help menu
    auto* helpMenu = menuBar()->addMenu(tr("&Help"));
    
    auto* aboutAction = helpMenu->addAction(tr("&About..."));
    connect(aboutAction, &QAction::triggered, this, &MainWindow::showAbout);
}

void MainWindow::loadSettings() {
    auto& settings = Settings::instance();
    
    restoreGeometry(settings.value("main/geometry").toByteArray());
#ifdef BUILD_WASM
    // Browser file selections live in the temporary WASM filesystem only.
    m_inputLineEdit->clear();
#else
    m_inputLineEdit->setText(settings.value("main/inputFile", "examples/hummingbird.png").toString());
#endif
    m_outputLineEdit->setText(settings.value("main/outputFile", "lithophane.stl").toString());
    m_exportFormatCombo->setCurrentIndex(settings.value("main/exportFormat", 0).toInt());
}

void MainWindow::saveSettings() {
    auto& settings = Settings::instance();
    
    settings.setValue("main/geometry", saveGeometry());
    settings.setValue("main/inputFile", m_inputLineEdit->text());
    settings.setValue("main/outputFile", m_outputLineEdit->text());
    settings.setValue("main/exportFormat", m_exportFormatCombo->currentIndex());
    settings.sync();
}

void MainWindow::dragEnterEvent(QDragEnterEvent* event) {
    if (event->mimeData()->hasUrls()) {
        for (const QUrl& url : event->mimeData()->urls()) {
            QString path = url.toLocalFile();
            QString ext = QFileInfo(path).suffix().toLower();
            if (ImageLoader::isFormatSupported(ext)) {
                event->acceptProposedAction();
                return;
            }
        }
    }
}

void MainWindow::dropEvent(QDropEvent* event) {
    for (const QUrl& url : event->mimeData()->urls()) {
        QString path = url.toLocalFile();
        QString ext = QFileInfo(path).suffix().toLower();
        if (ImageLoader::isFormatSupported(ext)) {
            setInputFile(path);
            return;
        }
    }
}

void MainWindow::closeEvent(QCloseEvent* event) {
    saveSettings();
    event->accept();
}

void MainWindow::setInputFile(const QString& path) {
    m_inputLineEdit->setText(path);
    m_statusLabel->setText(tr("Loaded: %1. Click Preview to generate the mesh.")
        .arg(QFileInfo(path).fileName()));
}

void MainWindow::onInputFileSelect() {
    QString filter = ImageLoader::supportedFormatsFilter();
#ifdef BUILD_WASM
    QFileDialog::getOpenFileContent(filter, [this](const QString& fileName, const QByteArray& fileContent) {
        if (fileName.isEmpty()) return;
        if (fileContent.isEmpty()) {
            QMessageBox::warning(this, tr("Load failed"), tr("The selected image is empty."));
            return;
        }

        const QString extension = QFileInfo(fileName).suffix().toLower();
        const QString temporaryPath = QDir::temp().filePath(
            "lithomaker-input-" + QUuid::createUuid().toString(QUuid::WithoutBraces) + "." + extension);
        QFile input(temporaryPath);
        if (!input.open(QIODevice::WriteOnly)) {
            QMessageBox::warning(this, tr("Load failed"), tr("Could not read the selected image."));
            return;
        }
        const bool saved = input.write(fileContent) == fileContent.size() && input.flush() &&
                           input.error() == QFileDevice::NoError;
        input.close();
        if (!saved) {
            QFile::remove(temporaryPath);
            QMessageBox::warning(this, tr("Load failed"), tr("Could not read the selected image."));
            return;
        }

        if (!m_wasmInputFilePath.isEmpty()) QFile::remove(m_wasmInputFilePath);
        m_wasmInputFilePath = temporaryPath;
        m_inputLineEdit->setText(QFileInfo(fileName).fileName());
        m_statusLabel->setText(tr("Loaded: %1. Click Preview to generate the mesh.")
            .arg(QFileInfo(fileName).fileName()));
    });
#else
    QString startDir = QFileInfo(m_inputLineEdit->text()).absolutePath();
    
    QString file = QFileDialog::getOpenFileName(this, tr("Select input image"), startDir, filter);
    if (!file.isEmpty()) {
        setInputFile(file);
    }
#endif
}

void MainWindow::onOutputFileSelect() {
#ifdef BUILD_WASM
    // The browser download prompt is shown after a successful export.
    return;
#else
    QString formats = "STL Files (*.stl);;OBJ Files (*.obj);;3MF Files (*.3mf);;All Files (*)";
    QString startDir = QFileInfo(m_outputLineEdit->text()).absolutePath();
    
    QString file = QFileDialog::getSaveFileName(this, tr("Save output file"), startDir, formats);
    if (!file.isEmpty()) {
        m_outputLineEdit->setText(file);
    }
#endif
}

void MainWindow::onExportFormatChanged(int index) {
    // Update output file extension based on format
    QString format = m_exportFormatCombo->itemData(index).toString();
    QString currentFile = m_outputLineEdit->text();
    QString baseName = QFileInfo(currentFile).completeBaseName();
    QString dir = QFileInfo(currentFile).absolutePath();
    
    QString ext = "stl";
    if (format == "obj") ext = "obj";
    else if (format == "3mf") ext = "3mf";
    
    QString newPath = QDir(dir).filePath(baseName + "." + ext);
    m_outputLineEdit->setText(newPath);
}

void MainWindow::onPreviewClicked() {
#ifdef BUILD_WASM
    QString inputFile = m_wasmInputFilePath;
#else
    QString inputFile = m_inputLineEdit->text();
#endif
    const quint64 inputRevision = m_meshInputRevision;
    
    if (!QFileInfo::exists(inputFile)) {
        QMessageBox::warning(this, tr("File not found"), 
            tr("Input file doesn't exist. Please check the filename."));
        return;
    }

    m_previewButton->setEnabled(false);
    m_exportButton->setEnabled(false);
    m_meshReady = false;
    m_currentMesh.clear();
#ifndef BUILD_WASM
    m_previewWidget->clear();
#endif
    m_progressBar->setVisible(true);
    m_progressBar->setValue(0);
    m_statusLabel->setText(tr("Loading image..."));
    QApplication::processEvents();

    // Load image
    auto result = ImageLoader::load(inputFile, 2000, false);
    if (!result) {
        QMessageBox::warning(this, tr("Load failed"), 
            tr("Failed to load the image file."));
        m_previewButton->setEnabled(true);
        m_progressBar->setVisible(false);
        return;
    }

    if (result->hasQualityWarning) {
        auto reply = QMessageBox::question(this, tr("JPEG Quality Warning"),
            tr("This JPEG image may have visible compression artifacts. "
               "For best results, use a high-quality PNG image.\n\n"
               "Continue anyway?"));
        if (reply != QMessageBox::Yes) {
            m_previewButton->setEnabled(true);
            m_progressBar->setVisible(false);
            return;
        }
    }

    if (result->image.width() > 2000 || result->image.height() > 2000) {
        auto reply = QMessageBox::question(this, tr("Large Image"),
            tr("The image is quite large (%1x%2). Resize to 2000px max for faster processing?")
                .arg(result->image.width()).arg(result->image.height()));
        if (reply == QMessageBox::Yes) {
            result = ImageLoader::load(inputFile, 2000, true);
        }
    }

    m_statusLabel->setText(tr("Generating mesh..."));
    m_progressBar->setValue(10);
    QApplication::processEvents();

    // Configure mesh generator
    auto& settings = Settings::instance();
    MeshConfig config;
    config.minThickness = settings.value("render/minThickness", 0.8).toFloat();
    config.totalThickness = settings.value("render/totalThickness", 4.0).toFloat();
    config.frameBorder = settings.value("render/frameBorder", 3.0).toFloat();
    config.width = settings.value("render/width", 200.0).toFloat();
    config.enableStabilizers = settings.value("render/enableStabilizers", true).toBool();
    config.permanentStabilizers = settings.value("render/permanentStabilizers", false).toBool();
    config.stabilizerThreshold = settings.value("render/stabilizerThreshold", 60.0).toFloat();
    config.stabilizerHeightFactor = settings.value("render/stabilizerHeightFactor", 0.15).toFloat();
    config.frameSlopeFactor = settings.value("render/frameSlopeFactor", 0.75).toFloat();
    config.enableHangers = settings.value("render/enableHangers", true).toBool();
    config.hangerCount = settings.value("render/hangers", 2).toInt();

    m_meshGenerator->setConfig(config);

    // Apply flip based on user preference, then invert for lithophane
    QImage image = result->image;
    if (m_flipVerticalCheckbox->isChecked()) {
        image = image.mirrored(false, true);  // Flip vertically
    }
    image.invertPixels();

    // Generate mesh
    auto generatedMesh = m_meshGenerator->generate(image, [this](int current, int total) {
        m_progressBar->setValue(10 + (current * 80) / total);
        QApplication::processEvents();
    });

    if (inputRevision != m_meshInputRevision) {
        m_progressBar->setVisible(false);
        m_previewButton->setEnabled(true);
        m_exportButton->setEnabled(false);
        m_statusLabel->setText(tr("Inputs changed during generation. Click Preview to regenerate."));
        return;
    }

    m_currentMesh = generatedMesh;
    m_meshReady = true;

    m_progressBar->setValue(95);
    m_statusLabel->setText(tr("Updating preview..."));
    QApplication::processEvents();

#ifndef BUILD_WASM
    // Update preview
    m_previewWidget->setMesh(std::move(generatedMesh));
#endif

    m_progressBar->setValue(100);
    m_progressBar->setVisible(false);
    m_previewButton->setEnabled(true);
    m_exportButton->setEnabled(true);
    m_statusLabel->setText(tr("Preview ready: %1 triangles. Click Export when satisfied.")
        .arg(m_currentMesh.size() / 3));
}

void MainWindow::onExportClicked() {
    if (!m_meshReady || m_currentMesh.isEmpty()) {
        QMessageBox::warning(this, tr("No mesh"),
            tr("Please generate a preview first."));
        return;
    }

    m_statusLabel->setText(tr("Exporting..."));
    QApplication::processEvents();

    if (doExport()) {
        m_statusLabel->setText(tr("Export completed: %1 triangles").arg(m_currentMesh.size() / 3));
    } else {
        m_statusLabel->setText(tr("Export was not completed."));
    }
}

void MainWindow::onFlipChanged(bool /*checked*/) {
    invalidateMesh(tr("Image orientation changed. Click Preview to regenerate."));
}

void MainWindow::invalidateMesh(const QString& reason) {
    ++m_meshInputRevision;
    m_meshReady = false;
    m_currentMesh.clear();
    if (m_exportButton) m_exportButton->setEnabled(false);
#ifndef BUILD_WASM
    if (m_previewWidget) m_previewWidget->clear();
#endif
    if (m_statusLabel) m_statusLabel->setText(reason);
}

bool MainWindow::doExport() {
    QString outputFile = m_outputLineEdit->text();
    QString format = m_exportFormatCombo->currentData().toString();
#ifdef BUILD_WASM
    QString extension = format == "obj" ? "obj" : "stl";
    QString downloadName = QFileInfo(outputFile).completeBaseName();
    if (downloadName.isEmpty()) downloadName = "lithophane";
    downloadName += "." + extension;
    outputFile = QDir::temp().filePath(
        "lithomaker-export-" + QUuid::createUuid().toString(QUuid::WithoutBraces) + "." + extension);
#endif

    std::unique_ptr<Exporter> exporter;
    if (format == "stl_bin") {
        exporter = std::make_unique<StlExporter>(StlFormat::Binary);
    } else if (format == "stl_ascii") {
        exporter = std::make_unique<StlExporter>(StlFormat::Ascii);
    } else if (format == "obj") {
        exporter = std::make_unique<ObjExporter>();
    } else if (format == "3mf") {
        exporter = std::make_unique<ThreeMfExporter>();
    } else {
        exporter = std::make_unique<StlExporter>(StlFormat::Binary);
    }

    auto& settings = Settings::instance();
    if (QFileInfo::exists(outputFile) && !settings.value("export/alwaysOverwrite", false).toBool()) {
        auto reply = QMessageBox::question(this, tr("Overwrite?"),
            tr("Output file already exists. Overwrite?"));
        if (reply != QMessageBox::Yes) {
            return false;
        }
    }

    auto result = exporter->exportMesh(m_currentMesh, outputFile);
    if (!result.success) {
        QMessageBox::warning(this, tr("Export failed"), result.errorMessage);
        return false;
    } else {
#ifdef BUILD_WASM
        QFile exportedFile(outputFile);
        if (!exportedFile.open(QIODevice::ReadOnly)) {
            QMessageBox::warning(this, tr("Export failed"), tr("Could not prepare the file download."));
            return false;
        }
        const QByteArray fileContent = exportedFile.readAll();
        if (exportedFile.error() != QFileDevice::NoError || fileContent.isEmpty()) {
            QMessageBox::warning(this, tr("Export failed"), tr("Could not read the exported file."));
            return false;
        }
        QFileDialog::saveFileContent(fileContent, downloadName);
#endif
        QMessageBox::information(this, tr("Export succeeded"),
            tr("Successfully exported to %1\n\nFile size: %2 KB\nTriangles: %3")
#ifdef BUILD_WASM
                .arg(downloadName)
#else
                .arg(outputFile)
#endif
                .arg(result.bytesWritten / 1024)
                .arg(m_currentMesh.size() / 3));
    }
    return true;
}

void MainWindow::showPreferences() {
    auto& settings = Settings::instance();
    const QStringList meshSettingKeys = {
        "render/enableStabilizers", "render/permanentStabilizers",
        "render/stabilizerThreshold", "render/stabilizerHeightFactor",
        "render/frameSlopeFactor", "render/enableHangers", "render/hangers"
    };
    QStringList previousValues;
    for (const QString& key : meshSettingKeys) previousValues.append(settings.value(key).toString());

    ConfigDialog dialog(this);
    dialog.exec();

    QStringList currentValues;
    for (const QString& key : meshSettingKeys) currentValues.append(settings.value(key).toString());
    if (previousValues != currentValues) {
        invalidateMesh(tr("Render settings changed. Click Preview to regenerate."));
    }
}

void MainWindow::showAbout() {
    AboutBox about(this);
    about.exec();
}

} // namespace LithoMaker
