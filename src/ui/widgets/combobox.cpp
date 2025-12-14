/**
 * @file combobox.cpp
 * @brief Custom combobox widget implementation
 *
 * Copyright 2021-2024 Lars Muldjord / Contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "combobox.h"
#include "core/settings.h"

namespace LithoMaker {

ComboBox::ComboBox(const QString& group, const QString& name,
                   const QString& defaultValue, QWidget* parent)
    : QComboBox(parent)
    , m_defaultValue(defaultValue)
{
    m_key = (group != "General" ? group + "/" : "") + name;

    connect(this, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ComboBox::saveToConfig);
}

void ComboBox::addConfigItem(const QString& text, const QString& value) {
    addItem(text, value);
}

void ComboBox::setFromConfig() {
    auto& settings = Settings::instance();
    QString value = settings.value(m_key, m_defaultValue).toString();

    for (int i = 0; i < count(); ++i) {
        if (itemData(i).toString() == value) {
            setCurrentIndex(i);
            return;
        }
    }
    setCurrentIndex(0);
}

void ComboBox::resetToDefault() {
    for (int i = 0; i < count(); ++i) {
        if (itemData(i).toString() == m_defaultValue) {
            setCurrentIndex(i);
            return;
        }
    }
}

void ComboBox::saveToConfig() {
    Settings::instance().setValue(m_key, currentData().toString());
}

} // namespace LithoMaker
