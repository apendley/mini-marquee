#pragma once

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

#include "Settings.h"
#include "MarqueeController.h"
#include "WebRenderer.h"

class MarqueeServer {
public:
    MarqueeServer(Settings& _settings, MarqueeController& _marquee, WebRenderer& _renderer) :
        server(80),
        settings(_settings),
        marquee(_marquee),
        renderer(_renderer)
    {

    }

    void begin();

private:
    void addHandlers();
    void sendSettingsResponse(AsyncWebServerRequest *request);
    void apiSetMessage(const char* message);
    void apiSetColor(uint8_t index);
    void apiSetBrightness(uint8_t index);
    void apiSetSpeed(uint8_t index);
    void apiSetFont(uint8_t index);
    void apiSetDisplayRotation(uint8_t index);
    
private:
    AsyncWebServer server;
    Settings& settings;
    MarqueeController& marquee;    
    WebRenderer& renderer;

    IPAddress localIP;
    IPAddress subnetMask;

    bool isShowingConnectMessage = true;
};