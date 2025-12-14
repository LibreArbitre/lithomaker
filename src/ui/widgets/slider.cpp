/**
 * @file slider.cpp
 * @brief Custom slider widget implementation
 *
 * Copyright 2021-2024 Lars Muldjord / Contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "slider.h"
#include "core/settings.h"

#include <QHBoxLayout>
#include <QDebug>

namespace LithoMaker {

Slider::Slider(const QString& group, const QString& name,
               int minValue, int maxValue, int defaultValue, int exponent,
               QWidget* parent)
    : QWidget(parent)
    , m_defaultValue(defaultValue)
    , m_exponent(exponent)
{
    m_key = (group != "General" ? group + "/" : "") + name;

    m_lineEdit = new QLineEdit(this);
    m_lineEdit->setMaximumWidth(60);

    m_slider = new QSlider(Qt::Horizontal, this);
    m_slider->setMinimum(minValue);
    m_slider->setMaximum(maxValue);
    m_slider->setTickInterval(1);
    m_slider->setSingleStep(1);
    m_slider->setPageStep(10);

    QString tooltip = tr("Default: %1").arg(static_cast<float>(defaultValue) / exponent);
    m_slider->setToolTip(tooltip);
    m_lineEdit->setToolTip(tooltip);

    // Load from settings
    auto& settings = Settings::instance();
    if (!settings.contains(m_key)) {
        settings.setValue(m_key, static_cast<float>(defaultValue) / exponent);
    }
    m_slider->setValue(static_cast<int>(settings.value(m_key).toFloat() * exponent));
    m_lineEdit->setText(QString::number(static_cast<float>(m_slider->value()) / exponent));

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_lineEdit);
    layout->addWidget(m_slider);

    connect(m_slider, &QSlider::valueChanged, this, &Slider::onSliderChanged);
    connect(m_lineEdit, &QLineEdit::editingFinished, this, &Slider::onLineEditFinished);
}

float Slider::value() const {
    return static_cast<float>(m_slider->value()) / m_exponent;
}

void Slider::setValue(float value) {
    m_slider->setValue(static_cast<int>(value * m_exponent));
}

void Slider::resetToDefault() {
    m_slider->setValue(m_defaultValue);
    m_lineEdit->setText(QString::number(static_cast<float>(m_defaultValue) / m_exponent));
    saveToConfig();
}

void Slider::onSliderChanged(int value) {
    float realValue = static_cast<float>(value) / m_exponent;
    m_lineEdit->setText(QString::number(realValue));
    saveToConfig();
    emit valueChanged(realValue);
}

void Slider::onLineEditFinished() {
    float value = m_lineEdit->text().toFloat();
    m_slider->setValue(static_cast<int>(value * m_exponent));
}

void Slider::saveToConfig() {
    auto& settings = Settings::instance();
    settings.setValue(m_key, m_lineEdit->text());
    qDebug() << "Saved" << m_key << "=" << m_lineEdit->text();
}

} // namespace LithoMaker
