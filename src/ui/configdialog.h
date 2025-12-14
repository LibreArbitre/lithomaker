/**
 * @file configdialog.h
 * @brief Preferences dialog
 *
 * Copyright 2021-2024 Lars Muldjord / Contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <QDialog>

class QListWidget;
class QStackedWidget;

namespace LithoMaker {

class ConfigDialog : public QDialog {
    Q_OBJECT

public:
    explicit ConfigDialog(QWidget* parent = nullptr);

private slots:
    void changePage(int index);

private:
    QListWidget* m_pageList;
    QStackedWidget* m_pages;
};

} // namespace LithoMaker
