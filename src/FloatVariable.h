#pragma once

#include <Arduino.h>
#include "VariableBase.h"

/**
 * @class FloatVariable
 * @brief Concrete variable class for float types.
 */
class FloatVariable : public VariableBase {
private:
    float* _ptr; /**< Pointer to the actual float variable */

public:
    /**
     * @brief Construct a new FloatVariable
     * 
     * @param ptr Pointer to the float variable
     * @param name Name of the variable
     * @param group Group name for UI placement
     * @param min Minimum allowed value
     * @param max Maximum allowed value
     * @param step Step size for modifications
     * @param readOnly Read-only mode
     */
    FloatVariable(float* ptr, const String& name, const String& group, float min, float max, float step, bool readOnly = false)
        : VariableBase(name, group, min, max, step, readOnly), _ptr(ptr) {}

    /**
     * @brief Get the name of the variable
     * @return String Variable name
     */
    String getName() const override {
        return _name;
    }

    /**
     * @brief Get the type of the variable
     * @return String "float"
     */
    String getType() const override {
        return "float";
    }

    /**
     * @brief Get the current value formatted to 4 decimal places
     * @return String Current value
     */
    String getValueAsString() const override {
        if (!_ptr) return "0.0000";
        return String(*_ptr, 4);
    }

    /**
     * @brief Set the value from a string, clamping between min and max
     * @param value String representation of the new value
     */
    void setValueFromString(const String& value) override {
        if (!_ptr) return;
        float val = value.toFloat();
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