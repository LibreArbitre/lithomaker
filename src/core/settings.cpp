/**
 * @file settings.cpp
 * @brief Singleton settings manager implementation
 *
 * Copyright 2021-2024 Lars Muldjord / Contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "settings.h"

namespace LithoMaker {

Settings& Settings::instance() {
    static Settings instance;
    return instance;
}

Settings::Settings()
    : m_settings(std::make_unique<QSettings>("LithoMaker", "LithoMaker"))
{
}

QVariant Settings::value(const QString& key, const QVariant& defaultValue) const {
    return m_settings->value(key, defaultValue);
}

void Settings::setValue(const QString& key, const QVariant& value) {
    m_settings->setValue(key, value);
}

bool Settings::contains(const QString& key) const {
    return m_settings->contains(key);
}

QStringList Settings::allKeys() const {
    return m_settings->allKeys();
}

void Settings::sync() {
    m_settings->sync();
}

} // namespace LithoMaker
