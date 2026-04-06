# ESP32VariableTuner

A library for real-time variable tuning and monitoring on ESP32 through an embedded Web UI.

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

- `registerFloat(...)`
- `registerDouble(...)`
- `registerInt(...)`
- `registerBool(...)`
- `registerString(...)`
- `begin(ssid, password, apMode, port, adminPassword)`
- `handle()`
- `enableAutoLoad(bool)`
- `getIPAddress()`

## Examples

- `examples/01_BasicVariables` – basic setup with grouped controls and telemetry
- `examples/02_PIDTuner` – advanced PID-style runtime tuning example

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
