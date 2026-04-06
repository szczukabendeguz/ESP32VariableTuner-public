#include "ProfileManager.h"

ProfileManager::ProfileManager() {
    // Constructor
}

String ProfileManager::truncateKey(const String& key) {
    if (key.length() > 15) {
        return key.substring(0, 15);
    }
    return key;
}

void ProfileManager::addToProfileList(const String& profileName) {
    _prefs.begin(_cfgNamespace, false);
    String profiles = _prefs.getString(_profilesKey, "");
    
    // Check if it already exists
    String searchStr = profileName;
    int idx = profiles.indexOf(searchStr);
    bool exists = false;
    
    if (idx != -1) {
        // Double check it's not a substring of another profile
        if ((idx == 0 || profiles.charAt(idx - 1) == ',') &&
            (idx + searchStr.length() == profiles.length() || profiles.charAt(idx + searchStr.length()) == ',')) {
            exists = true;
        }
    }
    
    if (!exists) {
        if (profiles.length() > 0) {
            profiles += ",";
        }
        profiles += profileName;
        _prefs.putString(_profilesKey, profiles);
    }
    _prefs.end();
}

void ProfileManager::removeFromProfileList(const String& profileName) {
    _prefs.begin(_cfgNamespace, false);
    String profiles = _prefs.getString(_profilesKey, "");
    
    if (profiles.length() > 0) {
        String newProfiles = "";
        int start = 0;
        int end = profiles.indexOf(',');
        
        while (end != -1) {
            String p = profiles.substring(start, end);
            if (p != profileName) {
                if (newProfiles.length() > 0) newProfiles += ",";
                newProfiles += p;
            }
            start = end + 1;
            end = profiles.indexOf(',', start);
        }
        
        // Handle last element
        String p = profiles.substring(start);
        if (p != profileName) {
            if (newProfiles.length() > 0) newProfiles += ",";
            newProfiles += p;
        }
        
        _prefs.putString(_profilesKey, newProfiles);
    }
    _prefs.end();
}

bool ProfileManager::saveProfile(const String& profileName, VariableRegistry& registry) {
    if (profileName.isEmpty()) return false;
    
    String ns = truncateKey(profileName);
    _prefs.begin(ns.c_str(), false);
    
    int count = registry.getVariableCount();
    for (int i = 0; i < count; i++) {
        // We know we can access the inner variables by getting them from a JSON parse if needed,
        // but since we need pointers, we iterate through the registry's JSON for now or add a method.
        // Wait, VariableRegistry has getVariable and getVariableCount, but how do we iterate?
        // Let's use getAllVariablesAsJSON to figure out the names, or wait, we don't have an iterator in VariableRegistry.
        // We can dynamically parse the JSON array or just modify VariableRegistry to allow iteration...
        // Actually, we can just rely on the names being parsed or use a workaround for iterating.
    }
    
    // We can't easily iterate VariableRegistry cleanly. Let's add an iterator or use the name list.
    // Assuming we parse the names from getAllVariablesAsJSON.
    String allJson = registry.getAllVariablesAsJSON();
    // Simple parse: find "name":"... "
    int startIdx = 0;
    while (true) {
        int nameIdx = allJson.indexOf("\"name\":\"", startIdx);
        if (nameIdx == -1) break;
        nameIdx += 8;
        int endIdx = allJson.indexOf("\"", nameIdx);
        if (endIdx == -1) break;
        
        String varName = allJson.substring(nameIdx, endIdx);
        VariableBase* var = registry.getVariable(varName);
        if (var) {
            String key = truncateKey(varName);
            _prefs.putString(key.c_str(), var->getValueAsString());
        }
        startIdx = endIdx + 1;
    }
    
    _prefs.end();
    
    addToProfileList(profileName);
    saveLastUsedProfile(profileName);
    return true;
}

bool ProfileManager::loadProfile(const String& profileName, VariableRegistry& registry) {
    if (profileName.isEmpty()) return false;
    
    String ns = truncateKey(profileName);
    _prefs.begin(ns.c_str(), true); // Read-only
    
    String allJson = registry.getAllVariablesAsJSON();
    int startIdx = 0;
    while (true) {
        int nameIdx = allJson.indexOf("\"name\":\"", startIdx);
        if (nameIdx == -1) break;
        nameIdx += 8;
        int endIdx = allJson.indexOf("\"", nameIdx);
        if (endIdx == -1) break;
        
        String varName = allJson.substring(nameIdx, endIdx);
        String key = truncateKey(varName);
        
        if (_prefs.isKey(key.c_str())) {
            String val = _prefs.getString(key.c_str(), "");
            registry.setVariableValue(varName, val);
        }
        startIdx = endIdx + 1;
    }
    
    _prefs.end();
    
    // Save as last used
    saveLastUsedProfile(profileName);
    
    return true;
}

bool ProfileManager::deleteProfile(const String& profileName) {
    if (profileName.isEmpty()) return false;
    
    String ns = truncateKey(profileName);
    _prefs.begin(ns.c_str(), false);
    _prefs.clear(); // Clear all keys in this namespace
    _prefs.end();
    
    removeFromProfileList(profileName);
    
    if (loadLastUsedProfile() == profileName) {
        saveLastUsedProfile("");
    }
    
    return true;
}

String ProfileManager::listProfilesAsJSON() {
    _prefs.begin(_cfgNamespace, true);
    String profiles = _prefs.getString(_profilesKey, "");
    _prefs.end();
    
    String json = "[";
    if (profiles.length() > 0) {
        int start = 0;
        int end = profiles.indexOf(',');
        bool first = true;
        
        while (end != -1) {
            if (!first) json += ",";
            json += "\"" + profiles.substring(start, end) + "\"";
            first = false;
            
            start = end + 1;
            end = profiles.indexOf(',', start);
        }
        
        // Handle last element
        if (!first) json += ",";
        json += "\"" + profiles.substring(start) + "\"";
    }
    json += "]";
    return json;
}

void ProfileManager::saveLastUsedProfile(const String& name) {
    _prefs.begin(_cfgNamespace, false);
    _prefs.putString(_lastUsedKey, name);
    _prefs.end();
}

String ProfileManager::loadLastUsedProfile() {
    _prefs.begin(_cfgNamespace, true);
    String name = _prefs.getString(_lastUsedKey, "");
    _prefs.end();
    return name;
}

void ProfileManager::setAutoLoad(bool enable) {
    _prefs.begin(_cfgNamespace, false);
    _prefs.putBool(_autoLoadKey, enable);
    _prefs.end();
}

bool ProfileManager::getAutoLoad() {
    _prefs.begin(_cfgNamespace, true);
    bool enable = _prefs.getBool(_autoLoadKey, false);
    _prefs.end();
    return enable;
}

String ProfileManager::exportAllToJSON(VariableRegistry& registry) {
    String allJson = registry.getAllVariablesAsJSON();
    String result = "{";
    bool first = true;
    
    int startIdx = 0;
    while (true) {
        int nameIdx = allJson.indexOf("\"name\":\"", startIdx);
        if (nameIdx == -1) break;
        nameIdx += 8;
        int endNameIdx = allJson.indexOf("\"", nameIdx);
        if (endNameIdx == -1) break;
        
        String varName = allJson.substring(nameIdx, endNameIdx);
        VariableBase* var = registry.getVariable(varName);
        if (var) {
            if (!first) result += ",";
            
            // Cleanly append key and value
            result += "\"" + varName + "\":";
            if (var->getType() == "string") {
                result += "\"" + var->getValueAsString() + "\"";
            } else if (var->getType() == "bool") {
                result += var->getValueAsString(); // true/false unquoted
            } else {
                result += var->getValueAsString(); // numbers unquoted
            }
            first = false;
        }
        startIdx = endNameIdx + 1;
    }
    
    result += "}";
    return result;
}

bool ProfileManager::importFromJSON(const String& json, VariableRegistry& registry) {
    if (json.length() < 2 || json.charAt(0) != '{') return false;
    
    bool changed = false;
    int searchIdx = 1; // skip '{'
    
    while (searchIdx < (int)json.length()) {
        int quoteIdx = json.indexOf('"', searchIdx);
        if (quoteIdx == -1) break;
        
        int endQuoteIdx = json.indexOf('"', quoteIdx + 1);
        if (endQuoteIdx == -1) break;
        
        String key = json.substring(quoteIdx + 1, endQuoteIdx);
        
        int colonIdx = json.indexOf(':', endQuoteIdx + 1);
        if (colonIdx == -1) break;
        
        // Find start of value
        int valStart = colonIdx + 1;
        while (valStart < (int)json.length() && isspace(json.charAt(valStart))) {
            valStart++;
        }
        
        String valueStr = "";
        int nextSearch = valStart;
        
        if (json.charAt(valStart) == '"') {
            // String value
            int valEnd = json.indexOf('"', valStart + 1);
            if (valEnd != -1) {
                valueStr = json.substring(valStart + 1, valEnd);
                nextSearch = valEnd + 1;
            } else {
                break;
            }
        } else {
            // Number or boolean
            int commaIdx = json.indexOf(',', valStart);
            int braceIdx = json.indexOf('}', valStart);
            
            int valEnd = commaIdx;
            if (valEnd == -1 || (braceIdx != -1 && braceIdx < commaIdx)) {
                valEnd = braceIdx;
            }
            
            if (valEnd != -1) {
                valueStr = json.substring(valStart, valEnd);
                valueStr.trim();
                nextSearch = valEnd;
            } else {
                valueStr = json.substring(valStart);
                valueStr.trim();
                nextSearch = json.length();
            }
        }
        
        if (registry.setVariableValue(key, valueStr)) {
            changed = true;
        }
        
        // Move to next key-value pair
        searchIdx = json.indexOf(',', nextSearch);
        if (searchIdx == -1) break; // no more commas
    }
    
    return changed;
}