# OtaHub

Official Arduino library for OtaHub.dev

## Description

[OtaHub](https://otahub.dev) is a distributed firmware OTA update platform for IoT. This library seamlessly connects your ESP32 devices to OtaHub to securely host, tag, and distribute firmware updates over-the-air.

### Key Features
- **Binary File Hosting:** Manage compiled `.bin` firmware files via a clean developer console.
- **Release Tagging:** Tag releases (e.g., `production`, `dev`, `test`). Devices automatically pull the correct update.
- **MD5 Integrity Checks:** Built-in validation ensures binary integrity before writing to flash.
- **REST API Access:** Devices request updates with standard HTTP GET requests using a scoped API key.

## Installation

1. Download this repository as a ZIP file.
2. In the Arduino IDE, navigate to **Sketch** > **Include Library** > **Add .ZIP Library...** and select the downloaded file.
3. Alternatively, you can clone or extract this repository directly into your `Arduino/libraries/` folder.

## Examples

Check the `examples/` directory for ready-to-use examples:
- **BasicOTA**: Basic setup and usage with background polling.
- **ManualOTA**: How to manually trigger OTA update checks.
- **ClearPreferences**: Utility to clear stored device credentials.
- **WritePreferences**: Utility to manually write device credentials.

## Usage

```cpp
#include <OtaHub.h>

OtaHub otaHub;

void setup() {
  Serial.begin(115200);
  
  // Connect to WiFi first
  // WiFi.begin("SSID", "PASSWORD");

  // Initialize OTA Hub
  // parameters: apiKey, currentVersion, pollingIntervalMinutes
  otaHub.begin("YOUR_API_KEY", "1.0.0", 1);
}

void loop() {
  // OTA check is handled automatically in the background
}
```
