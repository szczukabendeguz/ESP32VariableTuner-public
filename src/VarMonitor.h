#pragma once

#include <Arduino.h>
#include <WiFi.h>

// Forward declarations or includes for dependencies
#include "VariableRegistry.h"
#include "ProfileManager.h"
#include "VarMonitorServer.h"

#include "FloatVariable.h"
#include "DoubleVariable.h"
#include "IntVariable.h"
#include "BoolVariable.h"
#include "StringVariable.h"

/**
 * @class VarMonitor
 * @brief Main API for ESP32VariableTuner library. Integrates variable registration and web server management.
 */
class VarMonitor {
private:
    VariableRegistry& _registry;      /**< Singleton reference to variable registry */
    ProfileManager _profileManager;   /**< Manages storage to flash */
    VarMonitorServer _server;         /**< Web server and WebSocket manager */
    int _uiUpdateRate;                /**< UI Update frequency in ms */

public:
    /**
     * @brief Construct a new VarMonitor
     * @param port Web interface port (default: 80)
     */
    VarMonitor(uint16_t port = 80);

    /**
     * @brief Register a floating point variable
     * @param ptr Pointer to the actual float variable
     * @param name Variable name string
     * @param group Group name string
     * @param min Minimum bounds
     * @param max Maximum bounds
     * @param step Incremental step for sliders
     * @param readOnly If true, variable will only be displayed
     */
    void registerFloat(float* ptr, const String& name, const String& group, float min, float max, float step = 0.01f, bool readOnly = false);

    /**
     * @brief Register a double precision variable
     * @param ptr Pointer to the actual double variable
     * @param name Variable name string
     * @param group Group name string
     * @param min Minimum bounds
     * @param max Maximum bounds
     * @param step Incremental step for sliders
     * @param readOnly If true, variable will only be displayed
     */
    void registerDouble(double* ptr, const String& name, const String& group, double min, double max, double step = 0.01, bool readOnly = false);

    /**
     * @brief Register an integer variable
     * @param ptr Pointer to the actual integer variable
     * @param name Variable name string
     * @param group Group name string
     * @param min Minimum bounds
     * @param max Maximum bounds
     * @param readOnly If true, variable will only be displayed
     */
    void registerInt(int* ptr, const String& name, const String& group, int min, int max, bool readOnly = false);

    /**
     * @brief Register a boolean variable
     * @param ptr Pointer to the actual boolean variable
     * @param name Variable name string
     * @param group Group name string
     * @param readOnly If true, variable will only be displayed
     */
    void registerBool(bool* ptr, const String& name, const String& group, bool readOnly = false);

    /**
     * @brief Register a string variable
     * @param ptr Pointer to the actual string variable
     * @param name Variable name string
     * @param group Group name string
     * @param maxLength Character limit for string length
     * @param readOnly If true, variable will only be displayed
     */
    void registerString(String* ptr, const String& name, const String& group, int maxLength = 64, bool readOnly = false);

    /**
     * @brief Start Wi-Fi and Web Interface
     * @param ssid Wi-Fi network name
     * @param password Wi-Fi network password
     * @param apMode If true, sets up ESP32 as an Access Point
     * @param port Optional port parameter
     * @param adminPassword Optional admin password for API/WebSocket protection
     */
    void begin(const char* ssid, const char* password, bool apMode = false, uint16_t port = 80, const char* adminPassword = nullptr);

    /**
     * @brief Task loop. Should be placed in main loop().
     */
    void handle();

    /**
     * @brief Configure automatic load logic of persistent profiles
     * @param enable If true, will auto bind on boot to settings last saved
     */
    void enableAutoLoad(bool enable);

    /**
     * @brief Return the hosted IP address
     * @return String IP Address 
     */
    String getIPAddress();
};