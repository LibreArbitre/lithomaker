/**
 * @file checkbox.h
 * @brief Custom checkbox widget with settings persistence
 *
 * Copyright 2021-2024 Lars Muldjord / Contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <QCheckBox>
#include <QString>

namespace LithoMaker {

class CheckBox : public QCheckBox {
    Q_OBJECT

public:
    CheckBox(const QString& group, const QString& name,
             const QString& text, bool defaultValue,
             QWidget* parent = nullptr);

public slots:
    void resetToDefault();

private slots:
    void saveToConfig();

private:
    QString m_key;
    bool m_defaultValue;
};

} // namespace LithoMaker
