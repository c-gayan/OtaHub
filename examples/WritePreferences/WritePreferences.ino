#include <Arduino.h>
#include <Preferences.h>

Preferences preferences;

void setup() {
  Serial.begin(115200);
  
  // Wait a moment for serial monitor to connect
  delay(1000);
  Serial.println();
  Serial.println("--- OtaHub Preferences Writer ---");

  // Open the "otahub" namespace in read/write mode (false)
  preferences.begin("otahub", false);

  // Write the device name
  // This name will be pushed to the OTA Hub when checking for updates.
  String deviceName = "My-Living-Room-ESP";
  preferences.putString("name", deviceName);
  Serial.println("Set device name to: " + deviceName);

  // Write the update tag (e.g., "production", "beta", "testing")
  // This helps you target specific devices with specific firmware versions.
  String deviceTag = "beta";
  preferences.putString("tag", deviceTag);
  Serial.println("Set device tag to: " + deviceTag);

  // Close the preferences to save
  preferences.end();

  Serial.println("Preferences successfully saved!");
  Serial.println("You can now flash your main OtaHub code, and it will use these values.");
}

void loop() {
  // Nothing to do here
}
