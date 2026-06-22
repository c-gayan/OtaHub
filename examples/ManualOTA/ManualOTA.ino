#include <WiFi.h>
#include <OtaHub.h>

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Your Project API Key from the OtaHub Dashboard
String apiKey = "YOUR_API_KEY";

// CRITICAL: This version MUST match the exact text version you enter on the OtaHub website when uploading this file.
// If you upload "1.0.1" to the website but leave this as "1.0.0", your device will endlessly update in a loop!
String currentVersion = "1.0.0";

OtaHub otaHub;

// Keep track of the last time we checked for updates
unsigned long lastCheck = 0;
const unsigned long checkInterval = 3600000; // Check every 60 minutes (3,600,000 ms)

void setup() {
  Serial.begin(115200);
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi!");

  // Initialize the OTA Hub in manual mode
  // Passing 0 as the third parameter disables background thread checks entirely.
  otaHub.begin(apiKey, currentVersion, 0);

  // Perform an initial manual check on boot
  Serial.println("Performing initial firmware check...");
  otaHub.checkForUpdates();
}

void loop() {
  // Check for updates manually at defined interval
  if (millis() - lastCheck >= checkInterval) {
    lastCheck = millis();
    if (WiFi.status() == WL_CONNECTED) {
      otaHub.checkForUpdates();
    }
  }

  // Your main code runs here
  delay(1000);
}
