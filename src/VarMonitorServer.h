#pragma once

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <esp_system.h>
#include "VariableRegistry.h"
#include "ProfileManager.h"
#include "WebUI.h"

typedef std::function<void(int)> FreqCallback;

// Rate limiting structure per WebSocket client
struct ClientRateLimit {
    unsigned long lastResetTime;
    int messageCount;
};

/**
 * @class VarMonitorServer
 * @brief Manages the web server, REST API paths, and WebSockets for real-time syncing.
 */
class VarMonitorServer {
private:
    AsyncWebServer _server;               /**< Underlying AsyncWebServer */
    AsyncWebSocket _ws;                   /**< Underlying WebSocket handler */
    VariableRegistry& _registry;          /**< Reference to variable registry */
    ProfileManager& _profileManager;      /**< Reference to profile manager */
    
    unsigned long _lastBroadcastTime;     /**< Millis timestamp of the last broadcast */
    bool _broadcastPending;               /**< Flag indicating a broadcast is needed */
    FreqCallback _freqCallback = nullptr;

    String _adminPassword;
    String _authToken;
    bool _authEnabled = false;
    
    // Simple array to track rate limits for WS clients
    ClientRateLimit _clientRateLimits[16]; // Max typical WS connections 

    String generateToken();
    String extractCookieValue(const String& cookieHeader, const String& key);
    String getTokenFromRequest(AsyncWebServerRequest* request);
    bool isAuthorized(AsyncWebServerRequest* request);
    void sendUnauthorized(AsyncWebServerRequest* request);

public:
    void setFreqCallback(FreqCallback cb) { _freqCallback = cb; }
    void setAdminPassword(const char* adminPassword);

    /**
     * @brief Setup HTTP server routes
     */
    void setupRoutes();

    /**
     * @brief Setup WebSocket handlers
     */
    void setupWebSockets();

    /**
     * @brief Process an incoming WebSocket frame/message
     * @param client The WebSocket client
     * @param data The JSON data buffer parsed
     */
    void processWebSocketMessage(AsyncWebSocketClient* client, void* arg, uint8_t* data, size_t len);

    /**
     * @brief Add CORS headers to API responses
     * @param response The web response object
     */
    void setCORSHeaders(AsyncWebServerResponse* response);

    /**
     * @brief Helper to parse basic JSON {"key":"value"} from a string payload
     * @param json Payload
     * @param key Key to find
     * @return String Value found, or empty string
     */
    String extractJsonValue(const String& json, const String& key);

public:
    /**
     * @brief Construct a new VarMonitorServer
     * 
     * @param port HTTP Port to run on (e.g. 80)
     * @param registry Reference to the initialized VariableRegistry
     * @param profileManager Reference to initialized ProfileManager
     */
    VarMonitorServer(uint16_t port, VariableRegistry& registry, ProfileManager& profileManager);

    /**
     * @brief Initialize server paths, websockets and start listening
     */
    void begin(const char* adminPassword = nullptr);

    /**
     * @brief Must be called rapidly in loop() to handle delayed scheduled broadcasts
     */
    void handle();
    
    /**
     * @brief Forces a broadcast to all clients
     */
    void forceBroadcast() { _broadcastPending = true; }

    /**
     * @brief Broadcast current state of all variables to all connected WS clients
     */
    void broadcastAll();
};