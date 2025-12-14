/**
 * @file checkbox.cpp
 * @brief Custom checkbox widget implementation
 *
 * Copyright 2021-2024 Lars Muldjord / Contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "checkbox.h"
#include "core/settings.h"

namespace LithoMaker {

CheckBox::CheckBox(const QString& group, const QString& name,
                   const QString& text, bool defaultValue,
                   QWidget* parent)
    : QCheckBox(text, parent)
    , m_defaultValue(defaultValue)
{
    m_key = (group != "General" ? group + "/" : "") + name;

    auto& settings = Settings::instance();
    if (!settings.contains(m_key)) {
        settings.setValue(m_key, defaultValue);
    }
    setChecked(settings.value(m_key).toBool());

    connect(this, &QCheckBox::toggled, this, &CheckBox::saveToConfig);
}

void CheckBox::resetToDefault() {
    setChecked(m_defaultValue);
}

void CheckBox::saveToConfig() {
    Settings::instance().setValue(m_key, isChecked());
}

} // namespace LithoMaker
