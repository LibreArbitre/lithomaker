/**
 * @file main.cpp
 * @brief LithoMaker application entry point
 *
 * Copyright 2021-2024 Lars Muldjord / Contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QStyleFactory>
#include <QPalette>
#include <QDebug>

#include "core/settings.h"
#include "ui/mainwindow.h"
#include "version.h"

/**
 * @brief Apply dark theme palette to the application
 */
void applyDarkTheme(QApplication& app) {
    QPalette darkPalette;
    
    darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(42, 42, 42));
    darkPalette.setColor(QPalette::AlternateBase, QColor(66, 66, 66));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);
    
    app.setPalette(darkPalette);
}

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    
    // Set application metadata
    app.setApplicationName("LithoMaker");
    app.setOrganizationName("LithoMaker");
    app.setApplicationVersion(LITHOMAKER_VERSION);
    
    // Apply Fusion style for consistent cross-platform look
    app.setStyle(QStyleFactory::create("Fusion"));
    
    // Check for dark theme preference
    auto& settings = LithoMaker::Settings::instance();
    if (settings.value("ui/darkTheme", false).toBool()) {
        applyDarkTheme(app);
    }
    
    // Load translations
    QTranslator translator;
    if (translator.load(QLocale::system(), "lithomaker", "_", ":/translations")) {
        app.installTranslator(&translator);
        qInfo() << "Loaded translation for" << QLocale::system().name();
    }
    
    qInfo() << "LithoMaker" << LITHOMAKER_VERSION << "starting...";
    
    LithoMaker::MainWindow window;
    window.show();
    
    return app.exec();
}
