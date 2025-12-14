/**
 * @file settings.h
 * @brief Singleton settings manager for LithoMaker
 *
 * Copyright 2021-2024 Lars Muldjord / Contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <QSettings>
#include <QString>
#include <QVariant>
#include <memory>

namespace LithoMaker {

/**
 * @brief Singleton class for application settings management
 *
 * Replaces the global QSettings pointer for better encapsulation
 * and thread safety.
 */
class Settings {
public:
    /**
     * @brief Get the singleton instance
     * @return Reference to the Settings instance
     */
    static Settings& instance();

    // Delete copy/move constructors and assignment operators
    Settings(const Settings&) = delete;
    Settings& operator=(const Settings&) = delete;
    Settings(Settings&&) = delete;
    Settings& operator=(Settings&&) = delete;

    /**
     * @brief Get a setting value
     * @param key The setting key (e.g., "render/minThickness")
     * @param defaultValue Default value if key doesn't exist
     * @return The setting value
     */
    QVariant value(const QString& key, const QVariant& defaultValue = QVariant()) const;

    /**
     * @brief Set a setting value
     * @param key The setting key
     * @param value The value to store
     */
    void setValue(const QString& key, const QVariant& value);

    /**
     * @brief Check if a setting exists
     * @param key The setting key
     * @return true if the key exists
     */
    bool contains(const QString& key) const;

    /**
     * @brief Get all setting keys
     * @return List of all keys
     */
    QStringList allKeys() const;

    /**
     * @brief Sync settings to disk
     */
    void sync();

    /**
     * @brief Get the underlying QSettings object for backward compatibility
     * @return Pointer to QSettings
     * @deprecated Use value/setValue instead
     */
    [[deprecated("Use value/setValue instead")]]
    QSettings* qsettings() { return m_settings.get(); }

private:
    Settings();
    ~Settings() = default;

    std::unique_ptr<QSettings> m_settings;
};

} // namespace LithoMaker
