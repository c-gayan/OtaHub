#ifndef otahub_h
#define otahub_h

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Update.h>
#include <Preferences.h>

class OtaHub {
public:
    OtaHub();
    
    /**
     * @brief Initialize the OTA Hub listener
     * @param apiKey Your project's API key
     * @param currentVersion The current firmware version on this device (e.g. "1.0.0")
     * @param checkIntervalMinutes How often to poll the Hub for updates (default: 1)
     */
    void begin(String apiKey, String currentVersion, unsigned int checkIntervalMinutes = 1);
    


    /**
     * @brief Manually query the OTA Hub server for firmware updates
     */
    void checkForUpdates();

private:
    String _apiKey;
    String _currentVersion;
    unsigned int _checkInterval;
    
    String _deviceId;
    String _secretKey;
    
    bool provision();
    void loadCredentials();
    
    void getDeviceInfo(String &tag, String &name);
    String urlEncode(String str);
    String getJsonValue(const String &json, const String &key);
    
    static void _otaTask(void* parameter);
    TaskHandle_t _taskHandle;
};

#endif
