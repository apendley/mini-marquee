#include "MarqueeServer.h"

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncJson.h>
#include <ArduinoJson.h>
#include "transliterateUTF8.h"

// Only include in this file
#include "html/error_html.h"
#include "html/styles_css.h"

// Uncomment to print logs in this file to the serial console.
//#define LOGGER Serial
#include "Logger.h"

namespace {
    const char* apiMessageKey = "message";
    const char* apiSpeedKey = "speed";
    const char* apiBrightnessKey = "brightness";
    const char* apiDisplayRotationKey = "rotation";
    const char* apiFontKey = "font";
    const char* apiColorKey = "textColor";

    const char* ssid = MARQUEE_SSID;
    const char* passphrase = MARQUEE_PASSPHRASE;
}

void MarqueeServer::begin() {
    static const int wifiChannel = 6;    
    static const int maxClients = 4;

    localIP.fromString(MARQUEE_LOCAL_IP);
    subnetMask.fromString(MARQUEE_SUBNET_MASK);

    LOGLN("Initializing WiFi Hotspot");
    WiFi.mode(WIFI_MODE_AP);
    WiFi.softAPConfig(localIP, localIP, subnetMask);
    WiFi.softAP(ssid, passphrase, wifiChannel, 0, maxClients);

    addHandlers();
    server.begin();

    LOGFMT("styles.css size: %d\n\r", sizeof(styles_css));

    // Set initial message to show connection info.
    String connectMessage = "Wi-Fi: ";
    connectMessage += ssid;

    if (passphrase != nullptr) {
        connectMessage += "   Password: ";
        connectMessage += passphrase;
    }

    connectMessage += "   URL: http://";
    connectMessage += localIP.toString();

    marquee.setMessage(connectMessage.c_str());
    isShowingConnectMessage = true;    
}

void MarqueeServer::addHandlers() {
	// Requested page not found
	server.onNotFound([](AsyncWebServerRequest *request) {
        LOGFMT("Not found: %s %s\n\r", request->host().c_str(), request->url().c_str());
        AsyncWebServerResponse *response = request->beginResponse(404, "text/html", not_found_html);
        request->send(response);
	});  

    // Serve style sheet
    server.on("/styles.css", HTTP_GET, [](AsyncWebServerRequest *request){
        AsyncWebServerResponse *response = request->beginResponse(200, "text/css", styles_css);
        request->send(response);
    });

    // Serve the main page
	server.on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
        LOGLN("/ GET");
        renderer.render();
        AsyncWebServerResponse *response = request->beginResponse(200, "text/html", renderer.getRenderedDocument());
		request->send(response);
	});      

    // Handle HTML Form submission
    server.on("/update", HTTP_POST, [this](AsyncWebServerRequest* request) {
        LOGLN("/update POST");

        if (request->hasParam(apiMessageKey, true)) {
            String message = request->getParam(apiMessageKey, true)->value();
            apiSetMessage(message.c_str());
        }

        if (request->hasParam(apiColorKey, true)) {
            String indexString = request->getParam(apiColorKey, true)->value();

            if (indexString != "") {
                uint8_t index = atoi(indexString.c_str());
                apiSetColor(index);
            }
        }

        if (request->hasParam(apiSpeedKey, true)) {
            String speedString = request->getParam(apiSpeedKey, true)->value();

            if (speedString != "") {
                uint8_t index = atoi(speedString.c_str());
                apiSetSpeed(index);
            }
        }

        if (request->hasParam(apiBrightnessKey, true)) {
            String brightnessString = request->getParam(apiBrightnessKey, true)->value();

            if (brightnessString != "") {
                uint8_t index = atoi(brightnessString.c_str()); 
                apiSetBrightness(index);
            }
        }

        if (request->hasParam(apiDisplayRotationKey, true)) {
            String rotationString = request->getParam(apiDisplayRotationKey, true)->value();

            if (rotationString != "") {
                uint8_t index = atoi(rotationString.c_str());
                apiSetDisplayRotation(index);
            }
        }        

        if (request->hasParam(apiFontKey, true)) {
            String indexString = request->getParam(apiFontKey, true)->value();

            if (indexString != "") {
                uint8_t index = atoi(indexString.c_str());
                apiSetFont(index);
            }
        }

        renderer.render();
        
        request->send(200, "text/html", renderer.getRenderedDocument()); 
    });

    // Settings JSON API - GET
    server.on("/settings", HTTP_GET, [this](AsyncWebServerRequest* request) {
        LOGLN("/settings GET");

        sendSettingsResponse(request);
    });    

    // Settings JSON API - POST
    AsyncCallbackJsonWebHandler* handler = new AsyncCallbackJsonWebHandler("/settings", [this](AsyncWebServerRequest *request, JsonVariant &jsonVariant) {
        LOGLN("/settings POST");

        JsonObject json = jsonVariant.as<JsonObject>();

        // Update the message
        const char* message = json[apiMessageKey];
        apiSetMessage(message);

        // Update the color
        uint8_t color = json[apiColorKey];
        apiSetColor(color);

        // Update the brightness
        uint8_t newBrightnessIndex = json[apiBrightnessKey];
        apiSetBrightness(newBrightnessIndex);

        // Update the text speed
        uint8_t newSpeedIndex = json[apiSpeedKey];
        apiSetSpeed(newSpeedIndex);

        // Update the font
        uint8_t newFontIndex = json[apiFontKey];
        apiSetFont(newFontIndex);

        // Update the rotation
        uint8_t newRotation = json[apiDisplayRotationKey];
        apiSetDisplayRotation(newRotation);

        // Send updated response
        sendSettingsResponse(request);
    });
    server.addHandler(handler);
}

void MarqueeServer::sendSettingsResponse(AsyncWebServerRequest *request) {
    JsonDocument responseJSON;

    // The device's initially displayed message is the connection details,
    // so be careful not to leak them through the API.
    if (isShowingConnectMessage == false) {
        responseJSON[apiMessageKey] = marquee.getMessage();        
    } else {
        responseJSON[apiMessageKey] = "";
    }

    responseJSON[apiColorKey] = settings.colors.currentIndex();
    responseJSON[apiSpeedKey] = settings.scrollDelays.currentIndex();
    responseJSON[apiBrightnessKey] = settings.brightnessValues.currentIndex();
    responseJSON[apiDisplayRotationKey] = settings.displayRotations.currentIndex();
    responseJSON[apiFontKey] = settings.fonts.currentIndex();

    // According to the arduino json assistant (https://arduinojson.org/v7/assistant/#/step1),
    // our current model will use 145 bytes + the size of the message buffer.
    // Since the message buffer length is capped at 512 bytes, the largest
    // possible payload we'd send would be 657 bytes.
    // We'll use 1k to allow for a litle bit of breathing room.
    const int jsonOutputBufferSize = 1024;
    char jsonOutputBuffer[jsonOutputBufferSize];
    int bytesSerialized = serializeJson(responseJSON, jsonOutputBuffer, jsonOutputBufferSize);

    LOGFMT("serialized json output size: %d\n\r", bytesSerialized);

    if (bytesSerialized >= 0 && bytesSerialized < jsonOutputBufferSize) {
        AsyncWebServerResponse *response = request->beginResponse(200, "application/json", jsonOutputBuffer);
        request->send(response);        
    } else {
        LOGLN("Error serializing settings");
        AsyncWebServerResponse *response = request->beginResponse(500, "text/html", internal_error_html);
        request->send(response);            
    }    
}

void MarqueeServer::apiSetMessage(const char* message) {
    if (message != nullptr && strlen(message) > 0 && strcmp(message, marquee.getMessage()) != 0) {
        char decoded[MarqueeController::messageBufferSize];

        // Returns true if the full message was decoded, or false if the buffer was not
        // big enough to contain the entire message. Either way, the decoded message 
        // string is null terminated, so it is safe to use.
        bool transliterateResult = transliterateUTF8(message, decoded, MarqueeController::messageBufferSize);

        // CAREFUL: Arduino's serial library has a small buffer size
        // for printing messages, and printing out large strings can crash the firmware!
        // This is why we only log the number of bytes decoded below, rather than the entire string.
        if (transliterateResult) {
            LOGFMT("   message decoded: %d bytes\n\r", strlen(decoded));
        } else {
            LOGFMT("   partial message decoded: %d bytes\n\r", strlen(decoded));
        }
        
        marquee.setMessage(decoded);
        isShowingConnectMessage = false;
        renderer.setDirty();
    }
}

void MarqueeServer::apiSetColor(uint8_t index) {
    if (settings.colors.setIndex(index)) {
        LOGFMT("   color index: %d, name: %s, value: %s\n\r", index, settings.colors.current().name, settings.colors.current().hexString);

        const Color::RGB rgb = Color::RGB::fromHexString(settings.colors.current().hexString);
        marquee.setColor(rgb);
        renderer.setDirty();
    }
}

void MarqueeServer::apiSetBrightness(uint8_t index) {
    if (settings.brightnessValues.setIndex(index)) {
        LOGFMT("   brightness index: %d, value: %d\n\r", index, settings.brightnessValues.current().value);

        marquee.setBrightness(settings.brightnessValues.current().value);
        renderer.setDirty();
    }
}

void MarqueeServer::apiSetSpeed(uint8_t index) {
    if (settings.scrollDelays.setIndex(index)) {
        LOGFMT("   speed index: %d, delay: %d\n\r", index, settings.scrollDelays.current().value);

        marquee.setScrollDelay(settings.scrollDelays.current().value);
        renderer.setDirty();
    }        
}
void MarqueeServer::apiSetFont(uint8_t index) {
    if (settings.fonts.setIndex(index)) {
        LOGFMT("   font index: %d, name: %s\n\r", index, settings.fonts.current().name);

        marquee.setFontID(Font::ID(index));
        renderer.setDirty();
    }
}

void MarqueeServer::apiSetDisplayRotation(uint8_t index) {
    if (settings.displayRotations.setIndex(index)) {
        LOGFMT("   rotation index: %d, name: %s \n\r", index, settings.displayRotations.current().name);

        marquee.setRotation(settings.displayRotations.current().value);
        renderer.setDirty();
    }
}