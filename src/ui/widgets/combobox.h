/**
 * @file combobox.h
 * @brief Custom combobox widget with settings persistence
 *
 * Copyright 2021-2024 Lars Muldjord / Contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <QComboBox>
#include <QString>

namespace LithoMaker {

class ComboBox : public QComboBox {
    Q_OBJECT

public:
    ComboBox(const QString& group, const QString& name,
             const QString& defaultValue, QWidget* parent = nullptr);

    void addConfigItem(const QString& text, const QString& value);
    void setFromConfig();

public slots:
    void resetToDefault();

private slots:
    void saveToConfig();

private:
    QString m_key;
    QString m_defaultValue;
};

} // namespace LithoMaker
