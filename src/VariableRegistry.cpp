#include "VariableRegistry.h"

VariableRegistry::VariableRegistry() : _count(0) {
    for (int i = 0; i < MAX_VARIABLES; i++) {
        _variables[i] = nullptr;
    }
}

VariableRegistry& VariableRegistry::getInstance() {
    static VariableRegistry instance;
    return instance;
}

bool VariableRegistry::registerVariable(VariableBase* var) {
    if (var == nullptr) {
        return false;
    }

    if (_count >= MAX_VARIABLES) {
        return false;
    }

    // Check for duplicate name
    for (int i = 0; i < _count; i++) {
        if (_variables[i] != nullptr && _variables[i]->getName() == var->getName()) {
            return false;
        }
    }

    _variables[_count++] = var;
    return true;
}

VariableBase* VariableRegistry::getVariable(const String& name) const {
    for (int i = 0; i < _count; i++) {
        if (_variables[i] != nullptr && _variables[i]->getName() == name) {
            return _variables[i];
        }
    }
    return nullptr;
}

String VariableRegistry::getAllVariablesAsJSON() const {
    String json = "[";
    for (int i = 0; i < _count; i++) {
        if (_variables[i] != nullptr) {
            json += _variables[i]->toJSON();
            if (i < _count - 1) {
                json += ",";
            }
        }
    }
    json += "]";
    return json;
}

bool VariableRegistry::setVariableValue(const String& name, const String& value) {
    VariableBase* var = getVariable(name);
    if (var != nullptr) {
        var->setValueFromString(value);
        return true;
    }
    return false;
}

int VariableRegistry::getVariableCount() const {
    return _count;
}