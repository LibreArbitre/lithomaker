/**
 * @file lineedit.cpp
 * @brief Custom line edit widget implementation
 *
 * Copyright 2021-2024 Lars Muldjord / Contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "lineedit.h"
#include "core/settings.h"

namespace LithoMaker {

LineEdit::LineEdit(const QString& group, const QString& name,
                   const QString& defaultValue, QWidget* parent)
    : QLineEdit(parent)
    , m_defaultValue(defaultValue)
{
    m_key = (group != "General" ? group + "/" : "") + name;

    auto& settings = Settings::instance();
    if (!settings.contains(m_key)) {
        settings.setValue(m_key, defaultValue);
    }
    setText(settings.value(m_key).toString());
    setToolTip(tr("Default: %1").arg(defaultValue));

    connect(this, &QLineEdit::editingFinished, this, &LineEdit::saveToConfig);
}

void LineEdit::resetToDefault() {
    setText(m_defaultValue);
    saveToConfig();
}

void LineEdit::saveToConfig() {
    Settings::instance().setValue(m_key, text());
}

} // namespace LithoMaker
