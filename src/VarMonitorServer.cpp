#include "VarMonitorServer.h"

#ifndef VM_EMBED_WEBUI
#define VM_EMBED_WEBUI 1
#endif

#if VM_EMBED_WEBUI
#include "BootstrapData.h"
#endif

VarMonitorServer::VarMonitorServer(uint16_t port, VariableRegistry& registry, ProfileManager& profileManager)
    : _server(port), _ws("/ws"), _registry(registry), _profileManager(profileManager),
      _lastBroadcastTime(0), _broadcastPending(false) 
{
    for (int i = 0; i < 16; i++) {
        _clientRateLimits[i].lastResetTime = 0;
        _clientRateLimits[i].messageCount = 0;
    }
}

void VarMonitorServer::setCORSHeaders(AsyncWebServerResponse* response) {
    response->addHeader("Access-Control-Allow-Origin", "*");
    response->addHeader("Access-Control-Allow-Methods", "GET, POST, DELETE, OPTIONS");
    response->addHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
}

void VarMonitorServer::setAdminPassword(const char* adminPassword) {
    if (adminPassword && adminPassword[0] != '\0') {
        _adminPassword = adminPassword;
        _authEnabled = true;
    } else {
        _adminPassword = "";
        _authEnabled = false;
    }
    _authToken = "";
}

String VarMonitorServer::generateToken() {
    char buf[33];
    uint32_t r1 = esp_random();
    uint32_t r2 = esp_random();
    uint32_t r3 = esp_random();
    uint32_t r4 = esp_random();
    snprintf(buf, sizeof(buf), "%08lx%08lx%08lx%08lx", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3, (unsigned long)r4);
    return String(buf);
}

String VarMonitorServer::extractCookieValue(const String& cookieHeader, const String& key) {
    int start = 0;
    while (start < cookieHeader.length()) {
        int end = cookieHeader.indexOf(';', start);
        if (end == -1) end = cookieHeader.length();
        String part = cookieHeader.substring(start, end);
        part.trim();
        String prefix = key + "=";
        if (part.startsWith(prefix)) {
            return part.substring(prefix.length());
        }
        start = end + 1;
    }
    return "";
}

String VarMonitorServer::getTokenFromRequest(AsyncWebServerRequest* request) {
    if (!request) return "";

    String token = "";
    if (request->hasHeader("Authorization")) {
        String auth = request->getHeader("Authorization")->value();
        auth.trim();
        if (auth.startsWith("Bearer ")) {
            token = auth.substring(7);
        } else {
            token = auth;
        }
    }

    if (token.length() == 0 && request->hasHeader("Cookie")) {
        token = extractCookieValue(request->getHeader("Cookie")->value(), "vm_token");
    }

    if (token.length() == 0 && request->hasParam("token")) {
        token = request->getParam("token")->value();
    }

    return token;
}

bool VarMonitorServer::isAuthorized(AsyncWebServerRequest* request) {
    if (!_authEnabled) return true;
    String token = getTokenFromRequest(request);
    return token.length() > 0 && token == _authToken;
}

void VarMonitorServer::sendUnauthorized(AsyncWebServerRequest* request) {
    AsyncWebServerResponse* response = request->beginResponse(401, "application/json", "{\"ok\":false,\"error\":\"unauthorized\"}");
    response->addHeader("WWW-Authenticate", "Bearer");
    setCORSHeaders(response);
    request->send(response);
}

String VarMonitorServer::extractJsonValue(const String& json, const String& key) {
    // Very simple parser for strings like {"name":"myvar", "value":"..."}
    String searchKey = "\"" + key + "\":";
    int start = json.indexOf(searchKey);
    if (start == -1) return "";
    
    start += searchKey.length();
    
    // Handle spaces
    while (start < json.length() && isspace(json.charAt(start))) start++;
    
    if (json.charAt(start) == '"') {
        int end = json.indexOf('"', start + 1);
        if (end != -1) return json.substring(start + 1, end);
    } else {
        int end1 = json.indexOf(',', start);
        int end2 = json.indexOf('}', start);
        int end = end1;
        if (end == -1 || (end2 != -1 && end2 < end)) end = end2;
        if (end != -1) return json.substring(start, end);
    }
    return "";
}

void VarMonitorServer::setupRoutes() {
    // Login route
    _server.on("/api/login", HTTP_POST,
        [](AsyncWebServerRequest *request) {}, NULL,
        [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
            String body = "";
            for (size_t i = 0; i < len; i++) body += (char)data[i];

            if (!_authEnabled) {
                AsyncWebServerResponse* response = request->beginResponse(200, "application/json", "{\"ok\":true,\"auth\":false}");
                setCORSHeaders(response);
                request->send(response);
                return;
            }

            String password = extractJsonValue(body, "password");
            if (password == _adminPassword) {
                _authToken = generateToken();
                String json = "{\"ok\":true,\"auth\":true,\"token\":\"" + _authToken + "\"}";
                AsyncWebServerResponse* response = request->beginResponse(200, "application/json", json);
                response->addHeader("Set-Cookie", "vm_token=" + _authToken + "; Path=/; SameSite=Lax");
                setCORSHeaders(response);
                request->send(response);
            } else {
                sendUnauthorized(request);
            }
        }
    );

#if VM_EMBED_WEBUI
    // Bootstrap CSS Server
    _server.on("/css/bootstrap.css", HTTP_GET, [](AsyncWebServerRequest *request){
        AsyncWebServerResponse *response = request->beginResponse_P(200, "text/css", BOOTSTRAP_CSS_GZ, BOOTSTRAP_CSS_GZ_len);
        response->addHeader("Content-Encoding", "gzip");
        response->addHeader("Cache-Control", "max-age=31536000"); // Cache it aggressively
        request->send(response);
    });

    // Bootstrap JS Server
    _server.on("/js/bootstrap.js", HTTP_GET, [](AsyncWebServerRequest *request){
        AsyncWebServerResponse *response = request->beginResponse_P(200, "application/javascript", BOOTSTRAP_JS_GZ, BOOTSTRAP_JS_GZ_len);
        response->addHeader("Content-Encoding", "gzip");
        response->addHeader("Cache-Control", "max-age=31536000");
        request->send(response);
    });

    // Index Route - Serve UI
    _server.on("/", HTTP_GET, [this](AsyncWebServerRequest *request){
        AsyncWebServerResponse* response = request->beginResponse_P(200, "text/html", WEBUI_HTML);
        setCORSHeaders(response);
        request->send(response);
    });
#else
    _server.on("/", HTTP_GET, [this](AsyncWebServerRequest *request){
        AsyncWebServerResponse* response = request->beginResponse(200, "text/plain", "ESP32VariableTuner API mode (UI embedded disabled)");
        setCORSHeaders(response);
        request->send(response);
    });
#endif

    // OPTIONS preflight for CORS
    _server.onNotFound([](AsyncWebServerRequest *request) {
        if (request->method() == HTTP_OPTIONS) {
            AsyncWebServerResponse* response = request->beginResponse(200);
            response->addHeader("Access-Control-Allow-Origin", "*");
            response->addHeader("Access-Control-Allow-Methods", "GET, POST, DELETE, OPTIONS");
            response->addHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
            request->send(response);
        } else {
            request->send(404);
        }
    });

    // GET /api/variables
    _server.on("/api/variables", HTTP_GET, [this](AsyncWebServerRequest *request){
        if (!isAuthorized(request)) { sendUnauthorized(request); return; }
        String json = "{\"variables\":" + _registry.getAllVariablesAsJSON() + "}";
        AsyncWebServerResponse* response = request->beginResponse(200, "application/json", json);
        setCORSHeaders(response);
        request->send(response);
    });

    // GET /api/profiles
    _server.on("/api/profiles", HTTP_GET, [this](AsyncWebServerRequest *request){
        if (!isAuthorized(request)) { sendUnauthorized(request); return; }
        String json = _profileManager.listProfilesAsJSON();
        AsyncWebServerResponse* response = request->beginResponse(200, "application/json", json);
        setCORSHeaders(response);
        request->send(response);
    });

    // Get autoload
    _server.on("/api/profile/autoload", HTTP_GET, [this](AsyncWebServerRequest *request){
        if (!isAuthorized(request)) { sendUnauthorized(request); return; }
        String json = "{\"enabled\":" + String(_profileManager.getAutoLoad() ? "true" : "false") + "}";
        AsyncWebServerResponse* response = request->beginResponse(200, "application/json", json);
        setCORSHeaders(response);
        request->send(response);
    });

    // Need access to body payload for POST/DELETE
    // NOTE: ESPAsyncWebServer handles body payloads natively via .onBody() but simple requests might be query params
    // Let's support both body JSON and query parameters for profiles
    
    // Instead we do a unified handler for simple bodies by storing them
    _server.on("/api/variable", HTTP_POST, 
        [](AsyncWebServerRequest *request) {}, NULL,
        [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
            if (!isAuthorized(request)) { sendUnauthorized(request); return; }
            String body = "";
            for (size_t i = 0; i < len; i++) body += (char)data[i];
            
            String name = extractJsonValue(body, "name");
            String value = extractJsonValue(body, "value");
            
            AsyncWebServerResponse* response;
            if (_registry.setVariableValue(name, value)) {
                _broadcastPending = true;
                response = request->beginResponse(200, "application/json", "{\"ok\":true}");
            } else {
                response = request->beginResponse(400, "application/json", "{\"ok\":false,\"error\":\"Not found\"}");
            }
            setCORSHeaders(response);
            request->send(response);
        }
    );

    // POST /api/profile/save
    _server.on("/api/profile/save", HTTP_POST, [this](AsyncWebServerRequest *request){
        if (!isAuthorized(request)) { sendUnauthorized(request); return; }
        String name = "";
        if (request->hasParam("name")) name = request->getParam("name")->value();
        else { request->send(400, "application/json", "{\"ok\":false}"); return; }

        bool res = _profileManager.saveProfile(name, _registry);
        AsyncWebServerResponse* response = request->beginResponse(res ? 200 : 500, "application/json", res ? "{\"ok\":true}" : "{\"ok\":false}");
        setCORSHeaders(response);
        request->send(response);
    }, NULL, [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        if (!isAuthorized(request)) { sendUnauthorized(request); return; }
        // Parse JSON body if param wasn't used
        if(request->hasParam("name")) return;
        String body = ""; for(size_t i=0; i<len; i++) body += (char)data[i];
        String name = extractJsonValue(body, "name");
        bool res = _profileManager.saveProfile(name, _registry);
        request->send(res ? 200 : 500, "application/json", res ? "{\"ok\":true}" : "{\"ok\":false}");
    });

    // POST /api/profile/load
    _server.on("/api/profile/load", HTTP_POST, [this](AsyncWebServerRequest *request){
        if (!isAuthorized(request)) { sendUnauthorized(request); return; }
        String name = "";
        if (request->hasParam("name")) name = request->getParam("name")->value();
        bool res = _profileManager.loadProfile(name, _registry);
        if (res) _broadcastPending = true;
        AsyncWebServerResponse* response = request->beginResponse(res ? 200 : 500, "application/json", res ? "{\"ok\":true}" : "{\"ok\":false}");
        setCORSHeaders(response);
        request->send(response);
    });

    // POST /api/profile/delete (Alias for DELETE since standard forms dont send it nicely)
    _server.on("/api/profile/delete", HTTP_POST, [this](AsyncWebServerRequest *request){
        if (!isAuthorized(request)) { sendUnauthorized(request); return; }
        String name = "";
        if (request->hasParam("name")) name = request->getParam("name")->value();
        bool res = _profileManager.deleteProfile(name);
        AsyncWebServerResponse* response = request->beginResponse(res ? 200 : 500, "application/json", res ? "{\"ok\":true}" : "{\"ok\":false}");
        setCORSHeaders(response);
        request->send(response);
    });
    
    // DELETE /api/profile
    _server.on("/api/profile", HTTP_DELETE, [this](AsyncWebServerRequest *request){
        if (!isAuthorized(request)) { sendUnauthorized(request); return; }
        String name = "";
        if (request->hasParam("name")) name = request->getParam("name")->value();
        bool res = _profileManager.deleteProfile(name);
        AsyncWebServerResponse* response = request->beginResponse(res ? 200 : 500, "application/json", res ? "{\"ok\":true}" : "{\"ok\":false}");
        setCORSHeaders(response);
        request->send(response);
    }, NULL, [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        if (!isAuthorized(request)) { sendUnauthorized(request); return; }
        if(request->hasParam("name")) return;
        String body = ""; for(size_t i=0; i<len; i++) body += (char)data[i];
        String name = extractJsonValue(body, "name");
        bool res = _profileManager.deleteProfile(name);
        request->send(res ? 200 : 500, "application/json", res ? "{\"ok\":true}" : "{\"ok\":false}");
    });

    // Autoload POST
    _server.on("/api/profile/autoload", HTTP_POST, [this](AsyncWebServerRequest *request){
        if (!isAuthorized(request)) { sendUnauthorized(request); return; }
        if (request->hasParam("enable")) {
            bool enable = request->getParam("enable")->value() == "true";
            _profileManager.setAutoLoad(enable);
        }
        AsyncWebServerResponse* response = request->beginResponse(200, "application/json", "{\"ok\":true}");
        setCORSHeaders(response);
        request->send(response);
    });
}

void VarMonitorServer::setupWebSockets() {
    _ws.onEvent([this](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
        if (type == WS_EVT_CONNECT) {
            AsyncWebServerRequest* request = (AsyncWebServerRequest*)arg;
            if (!isAuthorized(request)) {
                client->close(1008, "Unauthorized");
                return;
            }
            // Trigger a broadcast so the new client gets UI sync immediately
            _broadcastPending = true;
        } else if (type == WS_EVT_DATA) {
            processWebSocketMessage(client, arg, data, len);
        }
    });

    _server.addHandler(&_ws);
}

void VarMonitorServer::processWebSocketMessage(AsyncWebSocketClient* client, void* arg, uint8_t* data, size_t len) {
    AwsFrameInfo *info = (AwsFrameInfo*)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
        // Implement Rate Limiting (max 20 per second per client id)
        unsigned long now = millis();
        int cid = client->id() % 16;
        if (now - _clientRateLimits[cid].lastResetTime > 1000) {
            _clientRateLimits[cid].lastResetTime = now;
            _clientRateLimits[cid].messageCount = 0;
        }
        _clientRateLimits[cid].messageCount++;
        
        if (_clientRateLimits[cid].messageCount > 20) {
            return; // Ignore message (rate limit breached)
        }

        data[len] = 0;
        String msg = (char*)data;
        
        // Very simple JSON parsing for speed {"action":"set","name":"varX","value":"123"}
        String action = extractJsonValue(msg, "action");
        if (action == "set") {
            String name = extractJsonValue(msg, "name");
            String value = extractJsonValue(msg, "value");
            
            if (_registry.setVariableValue(name, value)) {
                _broadcastPending = true;
            }
        } else if (action == "set_freq") {
            String value = extractJsonValue(msg, "value");
            int freq = value.toInt();
            if (freq >= 50 && freq <= 5000 && _freqCallback) {
                _freqCallback(freq);
            }
        }
    }
}

void VarMonitorServer::begin(const char* adminPassword) {
    setAdminPassword(adminPassword);
    setupRoutes();
    setupWebSockets();
    _server.begin();
}

void VarMonitorServer::broadcastAll() {
    if (_ws.count() > 0) {
        String payload = "{\"variables\":" + _registry.getAllVariablesAsJSON() + "}";
        _ws.textAll(payload);
    }
    _broadcastPending = false;
    _lastBroadcastTime = millis();
}

void VarMonitorServer::handle() {
    if (_broadcastPending) {
        if (millis() - _lastBroadcastTime > 100) {
            broadcastAll();
        }
    }
    _ws.cleanupClients();
}