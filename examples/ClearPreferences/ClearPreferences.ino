#include <Arduino.h>
#include <Preferences.h>

Preferences preferences;

void setup() {
  Serial.begin(115200);
  
  // Wait a moment for serial monitor to connect
  delay(1000);
  Serial.println();
  Serial.println("--- OtaHub Preferences Cleaner ---");

  // Open the "otahub" namespace in read/write mode
  preferences.begin("otahub", false);

  // Clear all keys saved under this namespace (e.g. name, tag)
  preferences.clear();

  // Close the preferences
  preferences.end();

  Serial.println("The 'otahub' preferences namespace has been completely cleared.");
  Serial.println("The library will now fallback to default values (tag='production', name='ESP-Device').");
}

void loop() {
  // Nothing to do here
}
