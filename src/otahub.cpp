#include "otahub.h"
#include <WiFiClientSecure.h>
#include "certs.h"

const String OTA_HUB_URL = "https://api.otahub.dev/v1/devices/ota";
const String OTA_PROVISION_URL = "https://api.otahub.dev/v1/devices/provision";

OtaHub::OtaHub() {}

void OtaHub::begin(String apiKey, String currentVersion, unsigned int checkIntervalMinutes) {
    apiKey.trim();
    currentVersion.trim();
    _apiKey = apiKey;
    _currentVersion = currentVersion;
    
    if (checkIntervalMinutes > 0) {
        _checkInterval = checkIntervalMinutes * 60 * 1000;
        // Create the FreeRTOS background task pinned to Core 0
        xTaskCreatePinnedToCore(
            _otaTask,         // Task function
            "OTA_Task",       // Task name
            8192,             // Stack size (8KB)
            this,             // Pass the current instance
            0,                // Priority
            &_taskHandle,     // Task handle
            0                 // Pin to Core 0
        );
    }
    
    loadCredentials();
}

void OtaHub::loadCredentials() {
    Preferences preferences;
    preferences.begin("otahub_sec", true);
    _deviceId = preferences.getString("deviceId", "");
    _secretKey = preferences.getString("secretKey", "");
    preferences.end();
}

bool OtaHub::provision() {
    String mac = WiFi.macAddress();
    mac.trim();
    
    

    WiFiClientSecure *client = new WiFiClientSecure;
#if defined(ESP8266)
    client->setTrustAnchors(new X509List(ISRG_ROOT_X1));
#else
    client->setCACert(ISRG_ROOT_X1);
#endif
    
    HTTPClient http;
    http.begin(*client, OTA_PROVISION_URL);
    http.addHeader("x-api-key", _apiKey);
    http.addHeader("Content-Type", "application/json");
    
    String payload = "{\"macAddress\":\"" + mac + "\"}";
    int httpCode = http.POST(payload);
    
    if (httpCode == 200 || httpCode == 201) {
        String response = http.getString();
        String successVal = getJsonValue(response, "success");
        
        if (successVal == "true") {
            _deviceId = getJsonValue(response, "deviceId");
            _secretKey = getJsonValue(response, "secretKey");
            
            if (_deviceId != "" && _secretKey != "") {
                Preferences preferences;
                preferences.begin("otahub_sec", false);
                preferences.putString("deviceId", _deviceId);
                preferences.putString("secretKey", _secretKey);
                preferences.end();
                
                http.end();
                delete client;
                return true;
            }
        }
    }
    
    Serial.printf("[OTAHub] Provisioning failed. HTTP Code: %d\n", httpCode);
    if (httpCode > 0) {
        String response = http.getString();
        String errorMsg = getJsonValue(response, "message");
        String errorCode = getJsonValue(response, "code");
        if (errorMsg != "") {
            if (errorCode != "") {
                Serial.printf("[OTAHub] Reason: %s (%s)\n", errorMsg.c_str(), errorCode.c_str());
            } else {
                Serial.printf("[OTAHub] Reason: %s\n", errorMsg.c_str());
            }
        } else if (response.length() > 0) {
            Serial.printf("[OTAHub] Raw Response: %s\n", response.c_str());
        }
    } else {
        Serial.println("[OTAHub] Connection failed (HTTP Client error).");
    }
    http.end();
    delete client;
    return false;
}



void OtaHub::getDeviceInfo(String &tag, String &name) {
    Preferences preferences;
    preferences.begin("otahub", true);
    tag = preferences.getString("tag", "production");
    name = preferences.getString("name", "");
    preferences.end();
    
    if (tag == "") {
        tag = "production";
    }
    // if name is empty, we leave it empty so the system will assign one
}

String OtaHub::urlEncode(String str) {
    String encodedString = "";
    for (int i = 0; i < str.length(); i++) {
        char c = str.charAt(i);
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            encodedString += c;
        } else if (c == ' ') {
            encodedString += "%20";
        } else {
            char buf[4];
            sprintf(buf, "%%%02X", (unsigned char)c);
            encodedString += buf;
        }
    }
    return encodedString;
}

void OtaHub::_otaTask(void *parameter) {
    OtaHub *hub = (OtaHub *)parameter;
    
    // Wait for WiFi to connect before the first check
    while(WiFi.status() != WL_CONNECTED) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    
    // Perform initial check
    hub->checkForUpdates();
    
    while(true) {
        // Sleep the task for the interval period without blocking the main CPU
        vTaskDelay(pdMS_TO_TICKS(hub->_checkInterval));
        
        if (WiFi.status() == WL_CONNECTED) {
            hub->checkForUpdates();
        }
    }
}

void OtaHub::checkForUpdates() {
    if (_deviceId == "" || _secretKey == "") {
        if (!provision()) return;
    }

    String tag, name;
    getDeviceInfo(tag, name);
    tag.trim();
    
    String requestUrl = OTA_HUB_URL + "?deviceId=" + urlEncode(_deviceId) 
                        + "&secretKey=" + urlEncode(_secretKey)
                        + "&tag=" + urlEncode(tag) 
                        + "&v=" + urlEncode(_currentVersion);

    
    WiFiClientSecure *client = new WiFiClientSecure;
    
#if defined(ESP8266)
    client->setTrustAnchors(new X509List(ISRG_ROOT_X1));
#else
    client->setCACert(ISRG_ROOT_X1);
#endif
    
    HTTPClient http;
    http.begin(*client, requestUrl);
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS); // MUST be set to follow Cloudflare R2 302 Redirects!
    
    const char * headerKeys[] = {"Content-Length"};
    http.collectHeaders(headerKeys, 1);
    
    int httpCode = http.GET();
    
    if (httpCode == 200) {
        int contentLength = http.getSize();
        if (contentLength <= 0) contentLength = http.header("Content-Length").toInt();
        
        Serial.println("[OTAHub] New firmware update found. Downloading and flashing...");
        bool canBegin = Update.begin(contentLength);
        
        if (canBegin) {
            size_t written = Update.writeStream(*client);
            
            if (written == contentLength) {
                if (Update.end()) {
                    if (Update.isFinished()) {
                        Serial.println("[OTAHub] Update successfully completed. Rebooting...");
                        ESP.restart();
                    } else {
                        Serial.println("[OTAHub] Update not finished.");
                    }
                } else {
                    Serial.println("[OTAHub] Error Occurred: " + String(Update.getError()));
                }
            } else {
                Serial.println("[OTAHub] Incomplete download.");
            }
        } else {
            Serial.println("[OTAHub] Not enough space to begin OTA");
        }
    } else if (httpCode == 304) {
        Serial.println("[OTAHub] Firmware is up to date.");
    } else {
        Serial.printf("[OTAHub] Check failed. HTTP Code: %d\n", httpCode);
        if (httpCode > 0) {
            String errResponse = http.getString();
            String errorMsg = getJsonValue(errResponse, "message");
            String errorCode = getJsonValue(errResponse, "code");
            if (errorMsg != "") {
                if (errorCode != "") {
                    Serial.printf("[OTAHub] Reason: %s (%s)\n", errorMsg.c_str(), errorCode.c_str());
                } else {
                    Serial.printf("[OTAHub] Reason: %s\n", errorMsg.c_str());
                }
            } else if (errResponse.length() > 0) {
                Serial.printf("[OTAHub] Raw Response: %s\n", errResponse.c_str());
            }
        } else {
            Serial.println("[OTAHub] Connection failed (HTTP Client error).");
        }
    }
    
    http.end();
    delete client;
}

String OtaHub::getJsonValue(const String &json, const String &key) {
    String searchKey = "\"" + key + "\"";
    int keyIdx = json.indexOf(searchKey);
    if (keyIdx == -1) return "";
    
    int colonIdx = json.indexOf(':', keyIdx + searchKey.length());
    if (colonIdx == -1) return "";
    
    int valStart = colonIdx + 1;
    while (valStart < json.length() && (json[valStart] == ' ' || json[valStart] == '\t' || json[valStart] == '\r' || json[valStart] == '\n')) {
        valStart++;
    }
    
    if (valStart >= json.length()) return "";
    
    if (json[valStart] == '"') {
        int strStart = valStart + 1;
        int strEnd = strStart;
        while (strEnd < json.length()) {
            if (json[strEnd] == '"') {
                int slashes = 0;
                int p = strEnd - 1;
                while (p >= strStart && json[p] == '\\') {
                    slashes++;
                    p--;
                }
                if (slashes % 2 == 0) {
                    break;
                }
            }
            strEnd++;
        }
        if (strEnd >= json.length()) return "";
        return json.substring(strStart, strEnd);
    } else {
        int valEnd = valStart;
        while (valEnd < json.length() && 
               json[valEnd] != ',' && 
               json[valEnd] != '}' && 
               json[valEnd] != ']' && 
               json[valEnd] != ' ' && 
               json[valEnd] != '\t' && 
               json[valEnd] != '\r' && 
               json[valEnd] != '\n') {
            valEnd++;
        }
        return json.substring(valStart, valEnd);
    }
}
