/**
 * @file configpages.h
 * @brief Preferences pages
 *
 * Copyright 2021-2024 Lars Muldjord / Contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <QWidget>

namespace LithoMaker {

class RenderPage : public QWidget {
    Q_OBJECT
public:
    explicit RenderPage(QWidget* parent = nullptr);
};

class ExportPage : public QWidget {
    Q_OBJECT
public:
    explicit ExportPage(QWidget* parent = nullptr);
};

class AppearancePage : public QWidget {
    Q_OBJECT
public:
    explicit AppearancePage(QWidget* parent = nullptr);
};

} // namespace LithoMaker
