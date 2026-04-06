#pragma once

#include <Arduino.h>
#include "VariableBase.h"

/**
 * @class DoubleVariable
 * @brief Concrete variable class for double types.
 */
class DoubleVariable : public VariableBase {
private:
    double* _ptr; /**< Pointer to the actual double variable */

public:
    /**
     * @brief Construct a new DoubleVariable
     * 
     * @param ptr Pointer to the double variable
     * @param name Name of the variable
     * @param group Group name for UI placement
     * @param min Minimum allowed value
     * @param max Maximum allowed value
     * @param step Step size for modifications
     * @param readOnly Read-only mode
     */
    DoubleVariable(double* ptr, const String& name, const String& group, double min, double max, double step, bool readOnly = false)
        : VariableBase(name, group, static_cast<float>(min), static_cast<float>(max), static_cast<float>(step), readOnly), _ptr(ptr) {}

    /**
     * @brief Get the name of the variable
     * @return String Variable name
     */
    String getName() const override {
        return _name;
    }

    /**
     * @brief Get the type of the variable
     * @return String "double"
     */
    String getType() const override {
        return "double";
    }

    /**
     * @brief Get the current value formatted to 6 decimal places
     * @return String Current value
     */
    String getValueAsString() const override {
        if (!_ptr) return "0.000000";
        return String(*_ptr, 6);
    }

    /**
     * @brief Set the value from a string, clamping between min and max
     * @param value String representation of the new value
     */
    void setValueFromString(const String& value) override {
        if (!_ptr) return;
        double val = value.toDouble();
        if (val < _minValue) val = _minValue;
        if (val > _maxValue) val = _maxValue;
        *_ptr = val;
    }

    /**
     * @brief Generate JSON representation
     * @return String JSON string
     */
    String toJSON() const override {
        String json = "{";
        json += "\"name\":\"" + getName() + "\",";
        json += "\"type\":\"" + getType() + "\",";
        json += "\"value\":\"" + getValueAsString() + "\",";
        json += "\"min\":" + String(_minValue) + ",";
        json += "\"max\":" + String(_maxValue) + ",";
        json += "\"step\":" + String(_step) + ",";
        json += "\"group\":\"" + _group + "\",";
        json += "\"readonly\":" + String(isReadOnly() ? "true" : "false");
        json += "}";
        return json;
    }
};
