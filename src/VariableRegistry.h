#pragma once

#include <Arduino.h>
#include "VariableBase.h"

#define MAX_VARIABLES 180

/**
 * @class VariableRegistry
 * @brief Singleton registry for storing and managing variables.
 */
class VariableRegistry {
private:
    VariableBase* _variables[MAX_VARIABLES]; /**< Array of variable pointers */
    int _count;                              /**< Current number of registered variables */

    /**
     * @brief Private constructor for Singleton pattern
     */
    VariableRegistry();

    // Prevent copying and assignment
    VariableRegistry(const VariableRegistry&) = delete;
    VariableRegistry& operator=(const VariableRegistry&) = delete;

public:
    /**
     * @brief Get the Singleton instance
     * @return VariableRegistry& Reference to the instance
     */
    static VariableRegistry& getInstance();

    /**
     * @brief Register a new variable
     * 
     * @param var Pointer to the variable to register
     * @return true If registration was successful
     * @return false If registration failed (array full or duplicate name)
     */
    bool registerVariable(VariableBase* var);

    /**
     * @brief Get a variable by name
     * 
     * @param name Name of the variable to find
     * @return VariableBase* Pointer to the variable, or nullptr if not found
     */
    VariableBase* getVariable(const String& name) const;

    /**
     * @brief Get all registered variables as a JSON array string
     * 
     * @return String JSON array string
     */
    String getAllVariablesAsJSON() const;

    /**
     * @brief Set the value of a variable by name
     * 
     * @param name Name of the variable
     * @param value String representation of the new value
     * @return true If variable was found and value was set
     * @return false If variable was not found
     */
    bool setVariableValue(const String& name, const String& value);

    /**
     * @brief Get the number of registered variables
     * 
     * @return int Number of variables
     */
    int getVariableCount() const;
};