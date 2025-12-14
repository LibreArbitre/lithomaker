/**
 * @file aboutbox.h
 * @brief About dialog
 *
 * Copyright 2021-2024 Lars Muldjord / Contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <QDialog>

namespace LithoMaker {

class AboutBox : public QDialog {
    Q_OBJECT

public:
    explicit AboutBox(QWidget* parent = nullptr);
};

} // namespace LithoMaker
