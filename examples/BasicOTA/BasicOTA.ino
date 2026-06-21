#include <WiFi.h>
#include <otahub.h>

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Your Project API Key from the OtaHub Dashboard
String apiKey = "YOUR_API_KEY";

// CRITICAL: This version MUST match the exact text version you enter on the OtaHub website when uploading this file.
// If you upload "1.0.1" to the website but leave this as "1.0.0", your device will endlessly update in a loop!
String currentVersion = "1.0.0";

OtaHub otaHub;

void setup() {
  Serial.begin(115200);
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi!");

  // Initialize the OTA Hub
  // Parameters: API Key, Current Version, Check Interval in Minutes (e.g. 60 minutes)
  otaHub.begin(apiKey, currentVersion, 60);
}

void loop() {
  // Your main code runs here undisturbed on Core 1!
  // The OTA Hub automatically checks for updates in the background on Core 0.
  delay(1000);
}
