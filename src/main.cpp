#include <Arduino.h>
#include <Adafruit_IS31FL3741.h>

#include "Settings.h"
#include "MarqueeController.h"
#include "MarqueeServer.h"
#include "WebRenderer.h"
#include "Font.h"

// Uncomment to print logs in this file to the serial console.
//#define LOGGER Serial
#include "Logger.h"

//////////////////////////////
// Main object graph
//////////////////////////////
Adafruit_IS31FL3741_QT_buffered display(MARQUEE_COLOR_ORDER);
Settings settings;
MarqueeController marquee(display);
WebRenderer webRenderer(settings);
MarqueeServer marqueeServer(settings, marquee, webRenderer);

//////////////////////////////
// Timing
//////////////////////////////
uint32_t lastUpdateTime = 0;

//////////////////////////////
// Forward reference
//////////////////////////////
void initDisplay();

//////////////////////////////
// Setup
//////////////////////////////
void setup() {
    Serial.begin(115200);
    // while(!Serial) { delay(1); }

    initDisplay();
    marqueeServer.begin();
    
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
    // Init I2C bus and speed it up as much as we can
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
    marquee.setRotation(settings.displayRotations.current().value);
    marquee.setScrollDelay(settings.scrollDelays.current().value);
    marquee.setFontID(Font::ID(settings.fonts.current().value));
    marquee.setBrightness(settings.brightnessValues.current().value);
}