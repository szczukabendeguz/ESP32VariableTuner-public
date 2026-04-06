#include <ESP32VariableTuner.h>

int myInt = 10;
float myFloat = 3.14;
bool myBool = true;
String myString = "test";

void setup() {
  Serial.begin(115200);

  // Initialize the registry
  VarRegistry.begin(10);

  // Register variables
  VarRegistry.registerInt("IntegerVal", &myInt, 0, 100);
  VarRegistry.registerFloat("FloatVal", &myFloat, 0.0, 10.0, 0.1);
  VarRegistry.registerBool("BoolVal", &myBool);
  VarRegistry.registerString("StringVal", &myString, 20);

  Serial.println("Variables registered.");
  Serial.print("Total variables: ");
  Serial.println(VarRegistry.getVariableCount());
}

void loop() {
  // Normally WebServer handling goes here
}
