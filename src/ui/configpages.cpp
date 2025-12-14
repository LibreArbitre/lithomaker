/**
 * @file configpages.cpp
 * @brief Preferences pages implementation
 *
 * Copyright 2021-2024 Lars Muldjord / Contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "configpages.h"
#include "widgets/slider.h"
#include "widgets/checkbox.h"
#include "widgets/combobox.h"
#include "widgets/lineedit.h"
#include "core/settings.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

namespace LithoMaker {

RenderPage::RenderPage(QWidget* parent)
    : QWidget(parent)
{
    auto* resetButton = new QPushButton(tr("Reset all to defaults"));

    auto* enableStabilizers = new CheckBox("render", "enableStabilizers", 
                                           tr("Enable stabilizers"), true);
    connect(resetButton, &QPushButton::clicked, enableStabilizers, &CheckBox::resetToDefault);

    auto* permanentStabilizers = new CheckBox("render", "permanentStabilizers",
                                              tr("Make stabilizers permanent"), false);
    connect(resetButton, &QPushButton::clicked, permanentStabilizers, &CheckBox::resetToDefault);

    auto* stabThresholdLabel = new QLabel(tr("Min height before stabilizers (mm):"));
    auto* stabThreshold = new LineEdit("render", "stabilizerThreshold", "60.0");
    connect(resetButton, &QPushButton::clicked, stabThreshold, &LineEdit::resetToDefault);

    auto* stabFactorLabel = new QLabel(tr("Stabilizer height factor:"));
    auto* stabFactor = new LineEdit("render", "stabilizerHeightFactor", "0.15");
    connect(resetButton, &QPushButton::clicked, stabFactor, &LineEdit::resetToDefault);

    auto* slopeLabel = new QLabel(tr("Frame slope factor:"));
    auto* slopeFactor = new LineEdit("render", "frameSlopeFactor", "0.75");
    connect(resetButton, &QPushButton::clicked, slopeFactor, &LineEdit::resetToDefault);

    auto* enableHangers = new CheckBox("render", "enableHangers",
                                       tr("Enable hangers"), true);
    connect(resetButton, &QPushButton::clicked, enableHangers, &CheckBox::resetToDefault);

    auto* hangersLabel = new QLabel(tr("Number of hangers:"));
    auto* hangersSlider = new Slider("render", "hangers", 1, 4, 2, 1);
    connect(resetButton, &QPushButton::clicked, hangersSlider, &Slider::resetToDefault);

    auto* layout = new QVBoxLayout(this);
    layout->addWidget(resetButton);
    layout->addWidget(enableStabilizers);
    layout->addWidget(permanentStabilizers);
    layout->addWidget(stabThresholdLabel);
    layout->addWidget(stabThreshold);
    layout->addWidget(stabFactorLabel);
    layout->addWidget(stabFactor);
    layout->addWidget(slopeLabel);
    layout->addWidget(slopeFactor);
    layout->addWidget(enableHangers);
    layout->addWidget(hangersLabel);
    layout->addWidget(hangersSlider);
    layout->addStretch();
}

ExportPage::ExportPage(QWidget* parent)
    : QWidget(parent)
{
    auto* resetButton = new QPushButton(tr("Reset all to defaults"));

    auto* formatLabel = new QLabel(tr("Default STL format:"));
    auto* formatCombo = new ComboBox("export", "stlFormat", "binary");
    formatCombo->addConfigItem(tr("Binary"), "binary");
    formatCombo->addConfigItem(tr("ASCII"), "ascii");
    formatCombo->setFromConfig();
    connect(resetButton, &QPushButton::clicked, formatCombo, &ComboBox::resetToDefault);

    auto* overwriteCheck = new CheckBox("export", "alwaysOverwrite",
                                        tr("Always overwrite existing file"), false);
    connect(resetButton, &QPushButton::clicked, overwriteCheck, &CheckBox::resetToDefault);

    auto* layout = new QVBoxLayout(this);
    layout->addWidget(resetButton);
    layout->addWidget(formatLabel);
    layout->addWidget(formatCombo);
    layout->addWidget(overwriteCheck);
    layout->addStretch();
}

AppearancePage::AppearancePage(QWidget* parent)
    : QWidget(parent)
{
    auto* resetButton = new QPushButton(tr("Reset all to defaults"));

    auto* darkTheme = new CheckBox("ui", "darkTheme",
                                   tr("Enable dark theme (requires restart)"), false);
    connect(resetButton, &QPushButton::clicked, darkTheme, &CheckBox::resetToDefault);

    auto* layout = new QVBoxLayout(this);
    layout->addWidget(resetButton);
    layout->addWidget(darkTheme);
    layout->addStretch();
}

} // namespace LithoMaker
