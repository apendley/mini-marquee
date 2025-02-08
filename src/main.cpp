#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>

#include <Adafruit_IS31FL3741.h>
#include "MarqueeController.h"
#include "Font.h"

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ESPStringTemplate.h>
#include <AsyncJson.h>
#include <ArduinoJson.h>
#include "transliterateUTF8.h"

// Only include in main.cpp!
#include "config.h"
#include "html/index_html.h"
#include "html/error_html.h"
#include "html/styles_css.h"

// Uncomment to print logs in this file to the serial console.
// #define LOGGER Serial
#include "Logger.h"

//////////////////////////////
// Config constants
//////////////////////////////
const char* ssid = SSID;
const char* passphrase = PASSPHRASE;
const IPAddress localIP = LOCAL_IP;
const IPAddress subnetMask = SUBNET_MASK;

//////////////////////////////
// LED matrix
//////////////////////////////
Adafruit_IS31FL3741_QT_buffered display(CONFIG_COLOR_ORDER);

//////////////////////////////
// Marquee controller
//////////////////////////////
MarqueeController marquee(display);
bool isShowingConnectMessage = true;

//////////////////////////////
// Settings definitions
//////////////////////////////
struct ColorSetting {
    const char* name;
    const char* hexString;
};

struct UnsignedByteSetting {
    const char* name;
    uint8_t value;
};

// Color settings
const ColorSetting colorSettings[] = {
    // A lot of these colors are biased away from red because red is
    // heavily over-represented on this matrix when powered by 3.3 volts.
    {"Rainbow", "#000000"},
    {"White", "#CFFFFF"},
    {"Red", "#FF0000"},
    {"Orange", "#E0A500"},
    {"Yellow", "#CFFF00"},
    {"Green", "#00FF00"},
    {"Cyan", "#00FFFF"},
    {"Blue", "#0000FF"},
    {"Purple", "#8000FF"},
    {"Magenta", "#FF00FF"},
};
const uint8_t colorSettingsCount = sizeof(colorSettings) / sizeof(colorSettings[0]);
uint8_t colorSettingsIndex = 0;

// Font settings
const UnsignedByteSetting fontSettings[] = {
    {"Adafruit", uint8_t(Font::ID::adafruit)},
    {"Fixed", uint8_t(Font::ID::fixed)},
    {"Fixed Mono", uint8_t(Font::ID::fixedMono)},
    {"Ancient", uint8_t(Font::ID::ancient)},
};
const uint8_t fontSettingsCount = sizeof(fontSettings) / sizeof(fontSettings[0]);
// The marquee controller will track the current font ID.

// Scroll delay/text speed settings
const UnsignedByteSetting scrollDelaySettings[] = {
    {"Very Slow", 100},
    {"Slow", 75},
    {"Moderate", 50},
    {"Fast", 30},
    {"Very Fast", 20},
};
const uint8_t scrollDelaySettingsCount = sizeof(scrollDelaySettings) / sizeof(scrollDelaySettings[0]);
uint8_t scrollDelayIndex = 1;

// Brightness settings
// Note that colors will lose their resolution at low brightness.
const UnsignedByteSetting brightnessSettings[] = {
    {"Very Dim", 70},
    {"Dim", 126},
    {"Moderate", 170},
    {"Bright", 220},
    {"Very Bright", 255},
};
const uint8_t brightnessSettingsCount = sizeof(brightnessSettings) / sizeof(brightnessSettings[0]);
uint8_t brightnessIndex = 2;

// Display rotation settings
const UnsignedByteSetting rotationSettings[] = {
    {"Down", 0},
    {"Left", 1},
    {"Up", 2},
    {"Right", 3},
};
const uint8_t rotationSettingsCount = sizeof(rotationSettings) / sizeof(rotationSettings[0]);
// The marquee controller will track the current rotation setting.

//////////////////////////////
// Web server
//////////////////////////////
AsyncWebServer server(80);

//////////////////////////////
// HTML template rendering
//////////////////////////////
const size_t renderedHTMLDocumentSize = 4 * 1024;
char renderedHTMLDocument[renderedHTMLDocumentSize] = {0};
bool htmlDocumentNeedsRender = true;

// Definitions for all of the substitutions that will be made.
struct ColorTokenGroup {
    static constexpr int tokenCount = 3;
    const char* selected;
    const char* name;
    const char* hexString;
};

struct ValueTokenGroup {
    static constexpr int tokenCount = 2;    
    const char* selected;
    const char* name;
};

const ColorTokenGroup colorTokenGroups[colorSettingsCount] = {
    {"{{CS0}}", "{{CN0}}", "{{C0}}"},
    {"{{CS1}}", "{{CN1}}", "{{C1}}"},
    {"{{CS2}}", "{{CN2}}", "{{C2}}"},
    {"{{CS3}}", "{{CN3}}", "{{C3}}"},
    {"{{CS4}}", "{{CN4}}", "{{C4}}"},
    {"{{CS5}}", "{{CN5}}", "{{C5}}"},
    {"{{CS6}}", "{{CN6}}", "{{C6}}"},
    {"{{CS7}}", "{{CN7}}", "{{C7}}"},
    {"{{CS8}}", "{{CN8}}", "{{C8}}"},
    {"{{CS9}}", "{{CN9}}", "{{C9}}"},
};

const ValueTokenGroup brightnessTokenGroups[brightnessSettingsCount] = {
    {"{{BS0}}", "{{BN0}}"},
    {"{{BS1}}", "{{BN1}}"},
    {"{{BS2}}", "{{BN2}}"},
    {"{{BS3}}", "{{BN3}}"},
    {"{{BS4}}", "{{BN4}}"},
};

const ValueTokenGroup textSpeedTokenGroups[scrollDelaySettingsCount] = {
    {"{{TS0}}", "{{TN0}}"},
    {"{{TS1}}", "{{TN1}}"},
    {"{{TS2}}", "{{TN2}}"},
    {"{{TS3}}", "{{TN3}}"},
    {"{{TS4}}", "{{TN4}}"},
};

const ValueTokenGroup fontTokenGroups[fontSettingsCount] = {
    {"{{FS0}}", "{{FN0}}"},
    {"{{FS1}}", "{{FN1}}"},
    {"{{FS2}}", "{{FN2}}"},
    {"{{FS3}}", "{{FN3}}"},
};

const ValueTokenGroup rotationTokenGroups[rotationSettingsCount] = {
    {"{{RS0}}", "{{RN0}}"},
    {"{{RS1}}", "{{RN1}}"},
    {"{{RS2}}", "{{RN2}}"},
    {"{{RS3}}", "{{RN3}}"},
};

const char*  backgroundColorToken = "{{BGCOLOR}}";

const size_t subTokensCount = 1 // backgroundColorToken
                              + colorSettingsCount * ColorTokenGroup::tokenCount
                              + brightnessSettingsCount * ValueTokenGroup::tokenCount
                              + scrollDelaySettingsCount * ValueTokenGroup::tokenCount
                              + fontSettingsCount * ValueTokenGroup::tokenCount
                              + rotationSettingsCount * ValueTokenGroup::tokenCount;

//////////////////////////////
// API property keys
//////////////////////////////
const char* apiMessageKey = "message";
const char* apiSpeedKey = "speed";
const char* apiBrightnessKey = "brightness";
const char* apiRotationKey = "rotation";
const char* apiFontKey = "font";
const char* apiColorKey = "textColor";

//////////////////////////////
// Timing
//////////////////////////////
uint32_t lastUpdateTime = 0;

//////////////////////////////
// Forward reference
//////////////////////////////
void initDisplay();
void initHotSpot();
void addWebServerHandlers();
void renderHTML();
void sendSettingsResponse(AsyncWebServerRequest *request);
void apiSetMessage(const char* message);
void apiSetColor(uint8_t index);
void apiSetBrightness(uint8_t index);
void apiSetSpeed(uint8_t index);
void apiSetFont(uint8_t index);
void apiSetRotation(uint8_t rotation);

//////////////////////////////
// Setup
//////////////////////////////
void setup() {
    Serial.begin(115200);
    // while(!Serial) { delay(1); }

    LOGFMT("index.html size: %d\n\r", sizeof(index_html));
    LOGFMT("styles.css size: %d\n\r", sizeof(styles_css));

    initDisplay();
    initHotSpot();
    lastUpdateTime = millis();
}

//////////////////////////////
// Loop
//////////////////////////////
void loop() {
    uint32_t now = millis();
    uint32_t dt = now - lastUpdateTime;
    lastUpdateTime = now;

    if (dt > 0) {
        marquee.update(dt);
    }
}

//////////////////////////////
// Implementation
//////////////////////////////
void initDisplay() {
    // Init SPI bus and speed it up as much as we can
    Wire1.setPins(SDA1, SCL1);
    Wire1.setClock(1000000);
    Wire1.begin();

    if( !display.begin(IS3741_ADDR_DEFAULT, &Wire1) ) {
        LOGLN("LED matrix not found");
        while(true);
    }
    
    // Set up matrix
    display.setLEDscaling(255); 
    display.setGlobalCurrent(255);
    display.enable(true);

    // RGB test pattern to help verify the color order of the matrix LEDs is correct.
    Color::RGB testPatternColors[] = {0xFF0000, 0x00FF00, 0x0000FF};
    const int testPatternColorsCount = sizeof(testPatternColors) / sizeof(testPatternColors[0]);

    for (int i = 0; i < testPatternColorsCount; i++) {
        display.fill(testPatternColors[i].packed565());
        display.show();
        delay(1000);
    }

    // Set default settings
    marquee.setRotation(INITIAL_ROTATION);
    marquee.setScrollDelay(scrollDelaySettings[scrollDelayIndex].value);
    marquee.setFontID(Font::ID::adafruit);
    marquee.setBrightness(brightnessSettings[brightnessIndex].value);

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

void initHotSpot() {
    static const int wifiChannel = 6;    
    static const int maxClients = 4;

    LOGLN("Initializing WiFi Hotspot");
    WiFi.mode(WIFI_MODE_AP);
    WiFi.softAPConfig(localIP, localIP, subnetMask);
    WiFi.softAP(ssid, passphrase, wifiChannel, 0, maxClients);

    addWebServerHandlers();
    server.begin();    
}

void addWebServerHandlers() {
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
	server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        LOGLN("/ GET");
        renderHTML();
        AsyncWebServerResponse *response = request->beginResponse(200, "text/html", renderedHTMLDocument);
		request->send(response);
	});      

    // Handle HTML Form submission
    server.on("/update", HTTP_POST, [](AsyncWebServerRequest* request) {
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

        if (request->hasParam(apiRotationKey, true)) {
            String rotationString = request->getParam(apiRotationKey, true)->value();

            if (rotationString != "") {
                uint8_t index = atoi(rotationString.c_str());
                apiSetRotation(index);
            }
        }        

        if (request->hasParam(apiFontKey, true)) {
            String indexString = request->getParam(apiFontKey, true)->value();

            if (indexString != "") {
                uint8_t index = atoi(indexString.c_str());
                apiSetFont(index);
            }
        }

        renderHTML();
        
        request->send(200, "text/html", renderedHTMLDocument); 
    });

    // Settings JSON API - GET
    server.on("/settings", HTTP_GET, [](AsyncWebServerRequest* request) {
        LOGLN("/settings GET");

        sendSettingsResponse(request);
    });    

    // Settings JSON API - POST
    AsyncCallbackJsonWebHandler* handler = new AsyncCallbackJsonWebHandler("/settings", [](AsyncWebServerRequest *request, JsonVariant &jsonVariant) {
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
        uint8_t newRotation = json[apiRotationKey];
        apiSetRotation(newRotation);

        // Send updated response
        sendSettingsResponse(request);
    });
    server.addHandler(handler);
}

void renderHTML() {
    if (htmlDocumentNeedsRender == false) {
        return;
    }

    ESPStringTemplate page(renderedHTMLDocument, renderedHTMLDocumentSize);

    TokenStringPair subs[subTokensCount];
    int currentSubToken = 0;

    static const char* selectedValue = " selected";
    static const char* notSelectedValue = "";

    subs[currentSubToken++].setPair(backgroundColorToken, colorSettings[colorSettingsIndex].hexString);

    for (int i = 0; i < colorSettingsCount; i++) {
        // Set the selected modifier for only the selected item
        const char* value = (i == colorSettingsIndex) ? selectedValue : notSelectedValue;
        subs[currentSubToken++].setPair(colorTokenGroups[i].selected, value);

        // Now set the name for each item
        subs[currentSubToken++].setPair(colorTokenGroups[i].name, colorSettings[i].name);

        // Now set the hex color value for each item
        subs[currentSubToken++].setPair(colorTokenGroups[i].hexString, colorSettings[i].hexString);        
    }    

    for (int i = 0; i < brightnessSettingsCount; i++) {
        // Set the selected modifier for only the selected item
        const char* value = (i == brightnessIndex) ? selectedValue : notSelectedValue;
        subs[currentSubToken++].setPair(brightnessTokenGroups[i].selected, value);

        // Now set the name for each item
        subs[currentSubToken++].setPair(brightnessTokenGroups[i].name, brightnessSettings[i].name);
    }

    for (int i = 0; i < scrollDelaySettingsCount; i++) {
        // Set the selected modifier for only the selected item
        const char* value = (i == scrollDelayIndex) ? selectedValue : notSelectedValue;
        subs[currentSubToken++].setPair(textSpeedTokenGroups[i].selected, value);

        // Now set the name for each item
        subs[currentSubToken++].setPair(textSpeedTokenGroups[i].name, scrollDelaySettings[i].name);
    }    

    for (int i = 0; i < fontSettingsCount; i++) {
        // Set the selected modifier for only the selected item
        const char* value = Font::ID(i) == marquee.getFontID() ? selectedValue : notSelectedValue;
        subs[currentSubToken++].setPair(fontTokenGroups[i].selected, value);

        // Now set the name for each item
        subs[currentSubToken++].setPair(fontTokenGroups[i].name, fontSettings[i].name);
    }

    for (int i = 0; i < rotationSettingsCount; i++) {
        // Set the selected modifier for only the selected item
        const char* value = (i == marquee.getRotation()) ? selectedValue : notSelectedValue;
        subs[currentSubToken++].setPair(rotationTokenGroups[i].selected, value);

        // Now set the name for each item
        subs[currentSubToken++].setPair(rotationTokenGroups[i].name, rotationSettings[i].name);
    }    

    page.add(index_html, subs, subTokensCount);

    // LOGLN("\n\rRendered web page:");
    // LOGLN(renderedHTMLDocument);    
    LOGFMT("rendered web page size: %d\n\r", strlen(renderedHTMLDocument) + 1);

    htmlDocumentNeedsRender = false;
}

void sendSettingsResponse(AsyncWebServerRequest *request) {
    JsonDocument responseJSON;

    // The device's initially displayed message is the connection details,
    // so be careful not to leak them through the API.
    if (isShowingConnectMessage == false) {
        responseJSON[apiMessageKey] = marquee.getMessage();        
    } else {
        responseJSON[apiMessageKey] = "";
    }

    responseJSON[apiColorKey] = colorSettingsIndex;
    responseJSON[apiSpeedKey] = scrollDelayIndex;
    responseJSON[apiBrightnessKey] = brightnessIndex;
    responseJSON[apiRotationKey] = marquee.getRotation();
    responseJSON[apiFontKey] = uint8_t(marquee.getFontID());

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

void apiSetMessage(const char* message) {
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
        htmlDocumentNeedsRender = true;
    }
}

void apiSetColor(uint8_t index) {
    if (index < colorSettingsCount && index != colorSettingsIndex) {
        LOGFMT("   color index: %d, name: %s, value: %s\n\r", index, colorSettings[index].name, colorSettings[index].hexString);
        const Color::RGB rgb = Color::RGB::fromHexString(colorSettings[index].hexString);
        
        colorSettingsIndex = index;
        marquee.setColor(rgb);
        htmlDocumentNeedsRender = true;
    }
}

void apiSetBrightness(uint8_t index) {
    if (index < brightnessSettingsCount && index != brightnessIndex) {
        LOGFMT("   brightness index: %d, value: %d\n\r", index, brightnessSettings[brightnessIndex].value);
        brightnessIndex = index;
        marquee.setBrightness(brightnessSettings[brightnessIndex].value);
        htmlDocumentNeedsRender = true;
    }
}

void apiSetSpeed(uint8_t index) {
    if (index < scrollDelaySettingsCount && index != scrollDelayIndex) {
        LOGFMT("   speed index: %d, delay: %d\n\r", index, scrollDelaySettings[scrollDelayIndex].value);
        scrollDelayIndex = index;
        marquee.setScrollDelay(scrollDelaySettings[scrollDelayIndex].value);
        htmlDocumentNeedsRender = true;
    }        
}
void apiSetFont(uint8_t index) {
    if (index < fontSettingsCount && Font::ID(index) != marquee.getFontID()) {
        LOGFMT("   font index: %d, name: %s\n\r", index, fontSettings[index].name);
        marquee.setFontID(Font::ID(index));
        htmlDocumentNeedsRender = true;
    }
}

void apiSetRotation(uint8_t rotation) {
    if (rotation <= 3 && rotation != marquee.getRotation()) {
        LOGFMT("   rotation index: %d, name: %s \n\r", rotation, rotationSettings[rotation].name);
        marquee.setRotation(rotation);
        htmlDocumentNeedsRender = true;
    }
}