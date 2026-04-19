# ESP32VariableTuner

A library for real-time variable tuning and monitoring on ESP32 through an embedded Web UI.

![Web UI](UI.png)

## What it does

- Register firmware variables (float, double, int, bool, string)
- View and edit values live from browser UI
- Real-time synchronization over WebSocket
- Group-based UI layout for cleaner tuning pages
- Built-in profile save/load support
- Works in AP mode and STA mode

## Dependencies

This library depends on:

- ESPAsyncWebServer
- AsyncTCP

## Quick start

```cpp
#include <ESP32VariableTuner.h>

VarMonitor monitor;

float setpoint = 24.0f;
float roomTemp = 22.5f;

void setup() {
	Serial.begin(115200);

	monitor.registerFloat(&setpoint, "Setpoint", "Control", 15.0f, 35.0f, 0.1f);
	monitor.registerFloat(&roomTemp, "RoomTemp", "Telemetry", -20.0f, 80.0f, 0.1f, true);

	monitor.begin("VarTuner", "12345678", true);
}

void loop() {
	monitor.handle();
}
```

## Main API

### `VarMonitor(uint16_t port = 80)`
Constructs a new `VarMonitor` instance.
- **port**: Web interface port (default: 80).

### `void registerFloat(...)`
`void registerFloat(float* ptr, const String& name, const String& group, float min, float max, float step = 0.01f, bool readOnly = false)`
Registers a floating point variable.
- **ptr**: Pointer to the actual float variable
- **name**: Variable name string
- **group**: Group name string
- **min**: Minimum bounds
- **max**: Maximum bounds
- **step**: Incremental step for sliders
- **readOnly**: If true, variable will only be displayed

### `void registerDouble(...)`
`void registerDouble(double* ptr, const String& name, const String& group, double min, double max, double step = 0.01, bool readOnly = false)`
Registers a double precision variable.
- **ptr**: Pointer to the actual double variable
- **name**: Variable name string
- **group**: Group name string
- **min**: Minimum bounds
- **max**: Maximum bounds
- **step**: Incremental step for sliders
- **readOnly**: If true, variable will only be displayed

### `void registerInt(...)`
`void registerInt(int* ptr, const String& name, const String& group, int min, int max, bool readOnly = false)`
Registers an integer variable.
- **ptr**: Pointer to the actual integer variable
- **name**: Variable name string
- **group**: Group name string
- **min**: Minimum bounds
- **max**: Maximum bounds
- **readOnly**: If true, variable will only be displayed

### `void registerBool(...)`
`void registerBool(bool* ptr, const String& name, const String& group, bool readOnly = false)`
Registers a boolean variable.
- **ptr**: Pointer to the actual boolean variable
- **name**: Variable name string
- **group**: Group name string
- **readOnly**: If true, variable will only be displayed

### `void registerString(...)`
`void registerString(String* ptr, const String& name, const String& group, int maxLength = 64, bool readOnly = false)`
Registers a string variable.
- **ptr**: Pointer to the actual string variable
- **name**: Variable name string
- **group**: Group name string
- **maxLength**: Character limit for string length
- **readOnly**: If true, variable will only be displayed

### `void begin(...)`
`void begin(const char* ssid, const char* password, bool apMode = false, uint16_t port = 80, const char* adminPassword = nullptr)`
Starts Wi-Fi and Web Interface.
- **ssid**: Wi-Fi network name
- **password**: Wi-Fi network password
- **apMode**: If true, sets up ESP32 as an Access Point
- **port**: Optional port parameter
- **adminPassword**: Optional admin password for API/WebSocket protection

### `void handle()`
Task loop. Should be placed in main `loop()`.

### `void enableAutoLoad(bool enable)`
Configures automatic load logic of persistent profiles.
- **enable**: If true, will auto bind on boot to settings last saved

### `String getIPAddress()`
Returns the hosted IP address.

## Examples

- `examples/01_BasicVariables` – basic setup with grouped controls and telemetry
- `examples/02_PIDTuner` – advanced PID-style runtime tuning example

## Performance & Resource Usage

Based on latest performance tests (up to 100 watched variables):

- **Flash Footprint**: Linked library contribution is ~124,182 B (~121.27 KiB), roughly 9.5% of the total firmware flash budget.
- **Memory Scaling**: Runtime free-heap decreases by roughly **217 bytes per registered variable**.
- **Latency**: WebSocket round-trip time (`ws_rtt_p95`) is very stable, staying around **30 ms** even under load with 100 variables.
- **CPU Loop Time**: Sub-millisecond impact on the main loop. CPU loop time remains stable around 10 ms due to non-blocking async nature.
- **Robustness**: Validation passed for multi-client consistency, reconnects, and live functional E2E tests (including Safari/iOS compatibility).

## Embedded UI mode

By default, the library serves the embedded UI from the ESP32.

To build API/WebSocket-only mode (without embedded UI), use:

```ini
build_flags =
	-DVM_EMBED_WEBUI=0
```

With `VM_EMBED_WEBUI=0`:

- `/api/*` and `/ws` stay available
- root route `/` returns API-mode message
- embedded UI assets are excluded to reduce footprint
