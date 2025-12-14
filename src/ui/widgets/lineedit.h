/**
 * @file lineedit.h
 * @brief Custom line edit widget with settings persistence
 *
 * Copyright 2021-2024 Lars Muldjord / Contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <QLineEdit>
#include <QString>

namespace LithoMaker {

class LineEdit : public QLineEdit {
    Q_OBJECT

public:
    LineEdit(const QString& group, const QString& name,
             const QString& defaultValue, QWidget* parent = nullptr);

public slots:
    void resetToDefault();

private slots:
    void saveToConfig();

private:
    QString m_key;
    QString m_defaultValue;
};

} // namespace LithoMaker
