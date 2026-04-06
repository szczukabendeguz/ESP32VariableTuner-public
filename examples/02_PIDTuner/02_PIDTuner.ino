#include <ESP32VariableTuner.h>

// Central monitor object: UI, API, WebSocket, and variable management.
VarMonitor monitor;

// --- Process (plant) state ---
// targetTemp: desired temperature
// measuredTemp: current measured/simulated temperature
// ambientTemp: ambient temperature (used for heat loss)
float targetTemp = 60.0f;
float measuredTemp = 26.0f;
float ambientTemp = 24.0f;

// --- PID parameters ---
float kp = 2.20f;
float ki = 0.35f;
float kd = 0.08f;

// --- Diagnostics and output ---
// heaterPower: controller output (0..100%)
// controlError/integralTerm/derivativeTerm: PID components visible on UI
float heaterPower = 0.0f;
float controlError = 0.0f;
float integralTerm = 0.0f;
float derivativeTerm = 0.0f;
bool controllerEnabled = true;

// System-level parameters
int pwmCeiling = 100;
String activeRecipe = "reflow_v1";

void setup() {
	// Serial initialization for debug and status output.
	Serial.begin(115200);
	delay(200);

	// --- Publish variables to the web UI ---
	// The "PID", "Telemetry", "Plant", "Output", "System" groups
	// organize controls into separate sections on the interface.
	monitor.registerFloat(&targetTemp, "TargetTemp", "PID", 20.0f, 90.0f, 0.1f);
	monitor.registerFloat(&measuredTemp, "MeasuredTemp", "Telemetry", -20.0f, 200.0f, 0.1f, true);
	monitor.registerFloat(&ambientTemp, "AmbientTemp", "Plant", -20.0f, 60.0f, 0.1f);

	monitor.registerFloat(&kp, "Kp", "PID", 0.0f, 20.0f, 0.01f);
	monitor.registerFloat(&ki, "Ki", "PID", 0.0f, 5.0f, 0.01f);
	monitor.registerFloat(&kd, "Kd", "PID", 0.0f, 5.0f, 0.01f);

	monitor.registerFloat(&heaterPower, "HeaterPower", "Output", 0.0f, 100.0f, 0.1f, true);
	monitor.registerFloat(&controlError, "Error", "Telemetry", -100.0f, 100.0f, 0.01f, true);
	monitor.registerFloat(&integralTerm, "Integral", "Telemetry", -500.0f, 500.0f, 0.01f, true);
	monitor.registerFloat(&derivativeTerm, "Derivative", "Telemetry", -100.0f, 100.0f, 0.01f, true);

	monitor.registerBool(&controllerEnabled, "ControllerEnabled", "System");
	monitor.registerInt(&pwmCeiling, "PwmCeiling", "System", 20, 100);
	monitor.registerString(&activeRecipe, "ActiveRecipe", "System", 20);

	// If a profile was saved earlier, it will auto-load on boot.
	monitor.enableAutoLoad(true);

	// AP mode + admin password:
	// SSID: VarTuner-PID, pass: 12345678, admin API/WS password: admin123
	monitor.begin("VarTuner-PID", "12345678", true, 80, "admin123");

	// Print access info.
	Serial.println("PID tuner example started.");
	Serial.print("Web UI: http://");
	Serial.println(monitor.getIPAddress());
}

void loop() {
	// Mandatory periodic call for web server and synchronization.
	monitor.handle();

	// 100 ms control cycle (dt = 0.1 s).
	static unsigned long lastControlMs = 0;
	if (millis() - lastControlMs < 100) {
		return;
	}
	lastControlMs = millis();

	const float dt = 0.1f;

	// Control error: positive means heating is still required.
	controlError = targetTemp - measuredTemp;

	if (controllerEnabled) {
		// Integrator accumulation + anti-windup clamping.
		integralTerm += controlError * dt;
		integralTerm = constrain(integralTerm, -250.0f, 250.0f);

		// Derivative term: error rate of change.
		static float previousError = 0.0f;
		derivativeTerm = (controlError - previousError) / dt;
		previousError = controlError;

		// PID sum and output limit (based on pwmCeiling).
		float raw = kp * controlError + ki * integralTerm + kd * derivativeTerm;
		heaterPower = constrain(raw, 0.0f, (float)pwmCeiling);
	} else {
		// When manually disabled, force output to zero
		// and slowly decay the integrator state.
		heaterPower = 0.0f;
		integralTerm *= 0.95f;
		derivativeTerm = 0.0f;
	}

	// Simple thermal model:
	// - heating: positive contribution from heater power
	// - cooling: proportional cooling toward ambient
	float heating = (heaterPower / 100.0f) * 2.2f;
	float cooling = (measuredTemp - ambientTemp) * 0.06f;
	measuredTemp += heating - cooling;
}
