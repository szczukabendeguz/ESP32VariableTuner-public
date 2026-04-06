#pragma once

#include <Arduino.h>
#include "VariableBase.h"

/**
 * @class IntVariable
 * @brief Concrete variable class for integer types.
 */
class IntVariable : public VariableBase {
private:
    int* _ptr; /**< Pointer to the actual int variable */

public:
    /**
     * @brief Construct a new IntVariable
     * 
     * @param ptr Pointer to the int variable
     * @param name Name of the variable
     * @param group Group name for UI placement
     * @param min Minimum allowed value
     * @param max Maximum allowed value
     * @param readOnly Read-only mode
     */
    IntVariable(int* ptr, const String& name, const String& group, int min, int max, bool readOnly = false)
        : VariableBase(name, group, min, max, 1.0f, readOnly), _ptr(ptr) {}

    /**
     * @brief Get the name of the variable
     * @return String Variable name
     */
    String getName() const override {
        return _name;
    }

    /**
     * @brief Get the type of the variable
     * @return String "int"
     */
    String getType() const override {
        return "int";
    }

    /**
     * @brief Get the current value as a string
     * @return String Current value
     */
    String getValueAsString() const override {
        if (!_ptr) return "0";
        return String(*_ptr);
    }

    /**
     * @brief Set the value from a string, clamping between min and max
     * @param value String representation of the new value
     */
    void setValueFromString(const String& value) override {
        if (!_ptr) return;
        int val = value.toInt();
        if (val < (int)_minValue) val = (int)_minValue;
        if (val > (int)_maxValue) val = (int)_maxValue;
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
        json += "\"min\":" + String((int)_minValue) + ",";
        json += "\"max\":" + String((int)_maxValue) + ",";
        json += "\"step\":1,";
        json += "\"group\":\"" + _group + "\",";
        json += "\"readonly\":" + String(isReadOnly() ? "true" : "false");
        json += "}";
        return json;
    }
};