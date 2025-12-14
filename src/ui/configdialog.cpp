/**
 * @file configdialog.cpp
 * @brief Preferences dialog implementation
 *
 * Copyright 2021-2024 Lars Muldjord / Contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "configdialog.h"
#include "configpages.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QListWidget>
#include <QStackedWidget>
#include <QPushButton>

namespace LithoMaker {

ConfigDialog::ConfigDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Preferences"));
    setMinimumSize(600, 400);

    m_pageList = new QListWidget();
    m_pageList->setFixedWidth(120);
    m_pageList->addItem(tr("Render"));
    m_pageList->addItem(tr("Export"));
    m_pageList->addItem(tr("Appearance"));
    m_pageList->setCurrentRow(0);

    m_pages = new QStackedWidget();
    m_pages->addWidget(new RenderPage());
    m_pages->addWidget(new ExportPage());
    m_pages->addWidget(new AppearancePage());

    connect(m_pageList, &QListWidget::currentRowChanged, this, &ConfigDialog::changePage);

    auto* mainLayout = new QHBoxLayout();
    mainLayout->addWidget(m_pageList);
    mainLayout->addWidget(m_pages, 1);

    auto* closeButton = new QPushButton(tr("Close"));
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);

    auto* layout = new QVBoxLayout(this);
    layout->addLayout(mainLayout);
    layout->addWidget(closeButton);
}

void ConfigDialog::changePage(int index) {
    m_pages->setCurrentIndex(index);
}

} // namespace LithoMaker
