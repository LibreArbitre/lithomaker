/**
 * @file slider.h
 * @brief Custom slider widget with settings persistence
 *
 * Copyright 2021-2024 Lars Muldjord / Contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <QWidget>
#include <QSlider>
#include <QLineEdit>
#include <QString>

namespace LithoMaker {

/**
 * @brief Slider with value display and settings persistence
 */
class Slider : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief Construct a slider
     * @param group Settings group
     * @param name Settings key name
     * @param minValue Minimum slider value
     * @param maxValue Maximum slider value
     * @param defaultValue Default value
     * @param exponent Scaling factor (real value = slider value / exponent)
     */
    Slider(const QString& group, const QString& name,
           int minValue, int maxValue, int defaultValue, int exponent,
           QWidget* parent = nullptr);

    ~Slider() override = default;

    /**
     * @brief Get the current real value
     */
    float value() const;

    /**
     * @brief Set the value
     */
    void setValue(float value);

public slots:
    /**
     * @brief Reset to default value
     */
    void resetToDefault();

signals:
    void valueChanged(float value);

private slots:
    void onSliderChanged(int value);
    void onLineEditFinished();

private:
    void saveToConfig();

    QSlider* m_slider;
    QLineEdit* m_lineEdit;
    QString m_key;
    int m_defaultValue;
    int m_exponent;
};

} // namespace LithoMaker
