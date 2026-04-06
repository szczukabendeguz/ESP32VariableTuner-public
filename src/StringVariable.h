#pragma once

#include <Arduino.h>
#include "VariableBase.h"

/**
 * @class StringVariable
 * @brief Concrete variable class for String types.
 */
class StringVariable : public VariableBase {
private:
    String* _ptr;   /**< Pointer to the actual String variable */
    int _maxLength; /**< Maximum allowed length of the string */

public:
    /**
     * @brief Construct a new StringVariable
     * 
     * @param ptr Pointer to the String variable
     * @param name Name of the variable
     * @param group Group name for UI placement
     * @param maxLength Maximum allowed length
     * @param readOnly Read-only mode
     */
    StringVariable(String* ptr, const String& name, const String& group, int maxLength, bool readOnly = false)
        : VariableBase(name, group, 0.0f, 0.0f, 0.0f, readOnly), _ptr(ptr), _maxLength(maxLength) {}

    /**
     * @brief Get the name of the variable
     * @return String Variable name
     */
    String getName() const override {
        return _name;
    }

    /**
     * @brief Get the type of the variable
     * @return String "string"
     */
    String getType() const override {
        return "string";
    }

    /**
     * @brief Get the current value as a string
     * @return String Current value
     */
    String getValueAsString() const override {
        if (!_ptr) return "";
        return *_ptr;
    }

    /**
     * @brief Set the value from a string, truncating if over maxLength
     * @param value The new string value
     */
    void setValueFromString(const String& value) override {
        if (!_ptr) return;
        if (value.length() > (unsigned int)_maxLength) {
            *_ptr = value.substring(0, _maxLength);
        } else {
            *_ptr = value;
        }
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
        json += "\"maxLength\":" + String(_maxLength) + ",";
        json += "\"group\":\"" + _group + "\",";
        json += "\"readonly\":" + String(isReadOnly() ? "true" : "false");
        json += "}";
        return json;
    }
};