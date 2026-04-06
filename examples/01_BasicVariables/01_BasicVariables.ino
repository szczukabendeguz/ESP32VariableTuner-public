#include <ESP32VariableTuner.h>

// Main library object: handles variable registration,
// the web server, and WebSocket synchronization.
VarMonitor monitor;

// --- Example variables ---
// All of these appear on the web UI and can be changed remotely.
float setpoint = 24.0f;
float roomTemp = 22.5f;
int fanPercent = 35;
bool autoMode = true;
String deviceName = "LAB-ESP32";

// Read-only telemetry: uptime is calculated by the firmware.
float uptimeSec = 0.0f;

void setup() {
  // Start serial output so we can see status and IP in the monitor.
  Serial.begin(115200);
  delay(200);

  // --- Register variables for the UI ---
  // Format: pointer, name, group, min, max, step, [readOnly]
  // Variables are grouped on the UI by the group name.
  monitor.registerFloat(&setpoint, "Setpoint", "Control", 15.0f, 35.0f, 0.1f);
  monitor.registerFloat(&roomTemp, "RoomTemp", "Telemetry", -20.0f, 80.0f, 0.1f, true);
  monitor.registerInt(&fanPercent, "FanPercent", "Control", 0, 100);
  monitor.registerBool(&autoMode, "AutoMode", "Control");
  monitor.registerString(&deviceName, "DeviceName", "System", 24);
  monitor.registerFloat(&uptimeSec, "UptimeSec", "Telemetry", 0.0f, 1e9f, 1.0f, true);

  // AP mode: ESP32 starts its own Wi-Fi network.
  // SSID: VarTuner-Basic, password: 12345678
  monitor.begin("VarTuner-Basic", "12345678", true);

  // Print access information.
  Serial.println("Basic example started.");
  Serial.print("Web UI: http://");
  Serial.println(monitor.getIPAddress());
}

void loop() {
  // Mandatory periodic call: HTTP/WS handling and UI synchronization.
  monitor.handle();

  // 1-second time base for the simulated process.
  static unsigned long lastTick = 0;
  if (millis() - lastTick >= 1000) {
    lastTick = millis();

    // Continuous telemetry update.
    uptimeSec += 1.0f;

    // Simple non-PID automatic control demo:
    // when higher setpoint is needed, fan increases and temperature approaches it.
    if (autoMode) {
      float error = setpoint - roomTemp;
      fanPercent += (int)(error * 6.0f);
      fanPercent = constrain(fanPercent, 0, 100);
      roomTemp += error * 0.12f;
    }
  }
}
