/**
 * @file aboutbox.cpp
 * @brief About dialog implementation
 *
 * Copyright 2021-2024 Lars Muldjord / Contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "aboutbox.h"
#include "version.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

namespace LithoMaker {

AboutBox::AboutBox(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("About LithoMaker"));
    setFixedSize(400, 300);

    auto* layout = new QVBoxLayout(this);

    auto* titleLabel = new QLabel(QString("<h1>LithoMaker</h1><h3>Version %1</h3>")
                                  .arg(LITHOMAKER_VERSION));
    titleLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(titleLabel);

    auto* descLabel = new QLabel(
        tr("<p>Creates 3D lithophanes from images and exports them to STL, OBJ, or 3MF files.</p>"
           "<p>Originally by Lars Muldjord, modernized for Qt6.</p>"
           "<p><a href='https://github.com/muldjord/lithomaker'>GitHub Repository</a></p>"));
    descLabel->setWordWrap(true);
    descLabel->setOpenExternalLinks(true);
    descLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(descLabel);

    layout->addStretch();

    auto* licenseLabel = new QLabel(
        tr("<small>LithoMaker is free software, licensed under GPL v2 or later.</small>"));
    licenseLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(licenseLabel);

    auto* closeButton = new QPushButton(tr("Close"));
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
    layout->addWidget(closeButton);
}

} // namespace LithoMaker
