#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include "VariableRegistry.h"

/**
 * @class ProfileManager
 * @brief Manages saving and loading variable profiles to/from ESP32 Flash (Preferences) and JSON serialization.
 */
class ProfileManager {
private:
    Preferences _prefs;                     /**< ESP32 Preferences instance */
    const char* _cfgNamespace = "cfg";      /**< Master configuration namespace */
    const char* _profilesKey = "profiles";  /**< Key for the comma-separated profile list */
    const char* _lastUsedKey = "last_prof"; /**< Key for the last used profile name */
    const char* _autoLoadKey = "autoload";  /**< Key for the autoload flag */

    /**
     * @brief Helper to add a profile to the master list
     * @param profileName Name to add
     */
    void addToProfileList(const String& profileName);

    /**
     * @brief Helper to remove a profile from the master list
     * @param profileName Name to remove
     */
    void removeFromProfileList(const String& profileName);

    /**
     * @brief Limit a string to 15 characters (Preferences constraint for namespace/keys)
     */
    String truncateKey(const String& key);

public:
    /**
     * @brief Default constructor
     */
    ProfileManager();

    /**
     * @brief Save a profile to Flash
     * @param profileName Name of the profile
     * @param registry VariableRegistry containing the current variables
     * @return true if successful, false otherwise
     */
    bool saveProfile(const String& profileName, VariableRegistry& registry);

    /**
     * @brief Load a profile from Flash
     * @param profileName Name of the profile
     * @param registry VariableRegistry to apply loaded values to
     * @return true if successful, false otherwise
     */
    bool loadProfile(const String& profileName, VariableRegistry& registry);

    /**
     * @brief Delete a profile from Flash
     * @param profileName Name of the profile to delete
     * @return true if successful, false otherwise
     */
    bool deleteProfile(const String& profileName);

    /**
     * @brief Return a JSON array of all saved profile names
     * @return String JSON array (e.g., ["default","profile1"])
     */
    String listProfilesAsJSON();

    /**
     * @brief Persist the last used profile name
     * @param name Profile name
     */
    void saveLastUsedProfile(const String& name);

    /**
     * @brief Retrieve the last used profile name
     * @return String Profile name
     */
    String loadLastUsedProfile();

    /**
     * @brief Save the auto-load flag
     * @param enable true to auto-load last profile on boot
     */
    void setAutoLoad(bool enable);

    /**
     * @brief Retrieve the auto-load flag
     * @return true if auto-load is enabled
     */
    bool getAutoLoad();

    /**
     * @brief Export all variable values to a JSON key-value string
     * @param registry VariableRegistry containing the variables
     * @return String JSON string
     */
    String exportAllToJSON(VariableRegistry& registry);

    /**
     * @brief Import variable values from a JSON string without ArduinoJson
     * @param json JSON string
     * @param registry VariableRegistry to update
     * @return true if successful parsing and setting occurred
     */
    bool importFromJSON(const String& json, VariableRegistry& registry);
};