#pragma once

#include <Arduino.h>
#include "VariableBase.h"

/**
 * @class BoolVariable
 * @brief Concrete variable class for boolean types.
 */
class BoolVariable : public VariableBase {
private:
    bool* _ptr; /**< Pointer to the actual bool variable */

public:
    /**
     * @brief Construct a new BoolVariable
     * 
     * @param ptr Pointer to the bool variable
     * @param name Name of the variable
     * @param group Group name for UI placement
     * @param readOnly Read-only mode
     */
    BoolVariable(bool* ptr, const String& name, const String& group = "", bool readOnly = false)
        : VariableBase(name, group, 0.0f, 1.0f, 1.0f, readOnly), _ptr(ptr) {}

    /**
     * @brief Get the name of the variable
     * @return String Variable name
     */
    String getName() const override {
        return _name;
    }

    /**
     * @brief Get the type of the variable
     * @return String "bool"
     */
    String getType() const override {
        return "bool";
    }

    /**
     * @brief Get the current value as a string
     * @return String "true" or "false"
     */
    String getValueAsString() const override {
        if (!_ptr) return "false";
        return (*_ptr) ? "true" : "false";
    }

    /**
     * @brief Set the value from a string
     * @param value "true", "false", "1", or "0"
     */
    void setValueFromString(const String& value) override {
        if (!_ptr) return;
        String valCopy = value;
        valCopy.trim();
        valCopy.toLowerCase();
        
        if (valCopy == "true" || valCopy == "1") {
            *_ptr = true;
        } else if (valCopy == "false" || valCopy == "0") {
            *_ptr = false;
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
        json += "\"checked\":" + getValueAsString() + ",";
        json += "\"group\":\"" + _group + "\",";
        json += "\"readonly\":" + String(isReadOnly() ? "true" : "false");
        json += "}";
        return json;
    }
};