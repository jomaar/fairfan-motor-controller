#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include "Config.h"

/**
 * WiFiManager - Handles WiFi connection and Access Point setup
 */
class WiFiManager {
private:
    bool apMode;
    String ipAddress;
    
public:
    WiFiManager() : apMode(false), ipAddress("") {}
    
    /**
     * Initialize WiFi - tries Station mode first, falls back to AP mode
     */
    void begin() {
        WiFi.mode(WIFI_OFF);
        delay(100);
        
        // Try Station mode if credentials provided
        if (strlen(Config::WiFi::STA_SSID) > 0) {
            if (connectStation()) {
                return;
            }
        }
        
        // Fall back to Access Point mode
        startAccessPoint();
    }
    
    /**
     * Get IP address as string
     */
    String getIPAddress() const {
        return ipAddress;
    }
    
    /**
     * Check if in AP mode
     */
    bool isAPMode() const {
        return apMode;
    }
    
    /**
     * Get WiFi status string
     */
    String getStatusString() const {
        if (apMode) {
            return "AP: " + String(Config::WiFi::AP_SSID);
        } else {
            return "WiFi: " + String(Config::WiFi::STA_SSID);
        }
    }
    
private:
    /**
     * Try to connect to existing WiFi network
     */
    bool connectStation() {
        Serial.println(F("[WiFi] Connecting to WiFi..."));
        Serial.print(F("[WiFi] SSID: "));
        Serial.println(Config::WiFi::STA_SSID);
        
        WiFi.mode(WIFI_STA);
        WiFi.begin(Config::WiFi::STA_SSID, Config::WiFi::STA_PASSWORD);
        
        unsigned long startTime = millis();
        while (WiFi.status() != WL_CONNECTED) {
            if (millis() - startTime > Config::WiFi::CONNECT_TIMEOUT_MS) {
                Serial.println(F("[WiFi] Connection timeout"));
                WiFi.disconnect();
                return false;
            }
            delay(500);
            Serial.print(".");
        }
        
        Serial.println();
        Serial.println(F("[WiFi] Connected!"));
        ipAddress = WiFi.localIP().toString();
        Serial.print(F("[WiFi] IP Address: "));
        Serial.println(ipAddress);
        
        apMode = false;
        return true;
    }
    
    /**
     * Start Access Point mode
     */
    void startAccessPoint() {
        Serial.println(F("[WiFi] Starting Access Point..."));
        
        WiFi.mode(WIFI_AP);
        IPAddress apIP(192, 168, 4, 1);
        IPAddress gateway(192, 168, 4, 1);
        IPAddress subnet(255, 255, 255, 0);
        WiFi.softAPConfig(apIP, gateway, subnet);
        WiFi.softAP(Config::WiFi::AP_SSID, Config::WiFi::AP_PASSWORD);
        
        ipAddress = WiFi.softAPIP().toString();
        
        Serial.println(F("[WiFi] Access Point started"));
        Serial.print(F("[WiFi] SSID: "));
        Serial.println(Config::WiFi::AP_SSID);
        Serial.print(F("[WiFi] Password: "));
        Serial.println(Config::WiFi::AP_PASSWORD);
        Serial.print(F("[WiFi] IP Address: "));
        Serial.println(ipAddress);
        
        apMode = true;
    }
};

#endif // WIFI_MANAGER_H
