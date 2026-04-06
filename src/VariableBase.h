#pragma once

#include <Arduino.h>

/**
 * @class VariableBase
 * @brief Abstract base class for all variable types.
 */
class VariableBase {
protected:
    String _name;   /**< Name of the variable */
    String _group;  /**< Group name for UI placement */
    float _minValue; /**< Minimum allowed value */
    float _maxValue; /**< Maximum allowed value */
    float _step;    /**< Step size for modifications */
    bool _readOnly; /**< Read-only mode flag */

public:
    /**
     * @brief Construct a new VariableBase object
     * 
     * @param name Name of the variable
     * @param group Group name of the variable
     * @param min Minimum value mapped to the variable
     * @param max Maximum value mapped to the variable
     * @param step Step size
     * @param readOnly Whether the variable is read-only
     */
    VariableBase(const String& name, const String& group = "", float min = 0.0f, float max = 0.0f, float step = 0.0f, bool readOnly = false)
        : _name(name), _group(group), _minValue(min), _maxValue(max), _step(step), _readOnly(readOnly) {}

    /**
     * @brief Destroy the VariableBase object
     */
    virtual ~VariableBase() {}

    /**
     * @brief Get the name of the variable
     * 
     * @return String Variable name
     */
    virtual String getName() const = 0;

    /**
     * @brief Get the type of the variable
     * 
     * @return String Variable type
     */
    virtual String getType() const = 0;

    /**
     * @brief Get the current value as a string representation
     * 
     * @return String Current value
     */
    virtual String getValueAsString() const = 0;

    /**
     * @brief Set the value from a string representation
     * 
     * @param value The string to parse and set
     */
    virtual void setValueFromString(const String& value) = 0;

    /**
     * @brief Get the minimum value for this variable
     * 
     * @return float Minimum value
     */
    virtual float getMin() const {
        return _minValue;
    }

    /**
     * @brief Get the maximum value for this variable
     * 
     * @return float Maximum value
     */
    virtual float getMax() const {
        return _maxValue;
    }

    /**
     * @brief Get the step size for this variable
     * 
     * @return float Step size
     */
    virtual float getStep() const {
        return _step;
    }

    /**
     * @brief Get the UI group of this variable
     * 
     * @return String Group name
     */
    virtual String getGroup() const {
        return _group;
    }

    /**
     * @brief Get the read-only mode of this variable
     * 
     * @return bool Read-only mode
     */
    virtual bool isReadOnly() const {
        return _readOnly;
    }

    /**
     * @brief Generate a JSON representation of the variable
     * 
     * @return String JSON object containing name, type, value, min, max, step, and group fields
     */
    virtual String toJSON() const {
        String json = "{";
        json += "\"name\":\"" + getName() + "\",";
        json += "\"type\":\"" + getType() + "\",";
        json += "\"value\":\"" + getValueAsString() + "\",";
        json += "\"min\":" + String(getMin()) + ",";
        json += "\"max\":" + String(getMax()) + ",";
        json += "\"step\":" + String(getStep()) + ",";
        json += "\"group\":\"" + getGroup() + "\",";
        json += "\"readonly\":" + String(isReadOnly() ? "true" : "false");
        json += "}";
        return json;
    }
};