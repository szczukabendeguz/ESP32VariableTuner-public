#include "VarMonitor.h"

VarMonitor::VarMonitor(uint16_t port)
    : _registry(VariableRegistry::getInstance()),
      _profileManager(),
      _server(port, _registry, _profileManager),
      _uiUpdateRate(250)
{
    _server.setFreqCallback([this](int freq) {
        _uiUpdateRate = freq;
    });
}

void VarMonitor::registerFloat(float* ptr, const String& name, const String& group, float min, float max, float step, bool readOnly) {
    FloatVariable* var = new FloatVariable(ptr, name, group, min, max, step, readOnly);
    _registry.registerVariable(var);
}

void VarMonitor::registerDouble(double* ptr, const String& name, const String& group, double min, double max, double step, bool readOnly) {
    DoubleVariable* var = new DoubleVariable(ptr, name, group, min, max, step, readOnly);
    _registry.registerVariable(var);
}

void VarMonitor::registerInt(int* ptr, const String& name, const String& group, int min, int max, bool readOnly) {
    IntVariable* var = new IntVariable(ptr, name, group, min, max, readOnly);
    _registry.registerVariable(var);
}

void VarMonitor::registerBool(bool* ptr, const String& name, const String& group, bool readOnly) {
    BoolVariable* var = new BoolVariable(ptr, name, group, readOnly);
    _registry.registerVariable(var);
}

void VarMonitor::registerString(String* ptr, const String& name, const String& group, int maxLength, bool readOnly) {
    StringVariable* var = new StringVariable(ptr, name, group, maxLength, readOnly);
    _registry.registerVariable(var);
}

void VarMonitor::begin(const char* ssid, const char* password, bool apMode, uint16_t port, const char* adminPassword) {
    if (apMode) {
        Serial.println("Starting Wi-Fi in AP mode...");
        WiFi.mode(WIFI_AP);
        WiFi.softAP(ssid, password);
        Serial.println("AP Mode started. IP: 192.168.4.1");
    } else {
        Serial.println("Connecting to Wi-Fi...");
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid, password);
        
        unsigned long startAttemptTime = millis();
        // Wait up to 10 seconds for connection
        while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
            Serial.print(".");
            delay(500);
        }
        Serial.println();
        
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("Failed to connect to Wi-Fi.");
            // Decide what to do here (fall back to AP could be nice, but stick to requirements)
        } else {
            Serial.println("Connected!");
            Serial.print("IP Address: ");
            Serial.println(WiFi.localIP());
        }
    }

    _server.begin(adminPassword);

    if (_profileManager.getAutoLoad()) {
        String lastProfile = _profileManager.loadLastUsedProfile();
        if (!lastProfile.isEmpty()) {
            Serial.println("Auto-loading profile: " + lastProfile);
            _profileManager.loadProfile(lastProfile, _registry);
        }
    }

    Serial.println("VarMonitor UI: http://" + getIPAddress());
}

void VarMonitor::handle() {
    _server.handle();

    // Check if variables have been changed natively, to sync UI.
    // To do this fully optimized, variables should throw events.
    // Simple way is let the client fetch, but for real time we need to broadcast.
    // _server syncs when setting from web, but when ESP changes values natively:
    static unsigned long lastCheck = 0;
    if (millis() - lastCheck > _uiUpdateRate) { // Send update to UI if values changed server-side
        lastCheck = millis();
        // Force broadcast is a bit heavy, ideally only if changed, but we can trigger a sync:
        _server.forceBroadcast();
    }
}

void VarMonitor::enableAutoLoad(bool enable) {
    _profileManager.setAutoLoad(enable);
}

String VarMonitor::getIPAddress() {
    if (WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA) {
        return WiFi.softAPIP().toString();
    } else {
        return WiFi.localIP().toString();
    }
}