#pragma once

#include <Arduino.h>
#include <Adafruit_IS31FL3741.h>
#include "Font.h"
#include "Color.h"

class MarqueeController {
public:
    // including terminating null character, so max string length is 511
    static constexpr uint16_t messageBufferSize = 512;
    static constexpr uint16_t maxMessageLength = messageBufferSize - 1;

public:
    MarqueeController(Adafruit_IS31FL3741_QT_buffered& matrix) :
        matrix(matrix), 
        messageWidth(matrix.width()),
        position(matrix.width())
    {
        setMessage("Please set a message");
    }

    void resetScroll() {
        matrix.setTextWrap(false);        
        messageWidth = Font::withID(fontID).textWidth(message);
        position = matrix.width();
        scrollElapsed = 0;
        startHue = 0;
    }    

    void update(uint32_t dt) {
        scrollElapsed += dt;
    
        if (scrollElapsed >= scrollDelay) {
            scrollElapsed -= scrollDelay;

            // uint32_t drawStart = millis();

            matrix.fill(0);

            auto yOffset = Font::withID(fontID).yOffset;

            if (matrixRotation == 1 || matrixRotation == 3) {
                yOffset += 2;
            }

            matrix.setCursor(position, yOffset);

            uint32_t len = strlen(message);

            if (color.isBlack()) {
                for (unsigned int i = 0; i < len; i++) {
                    uint16_t hue = (startHue + (i * hueStep)) & 0xFFFF;
                    auto hsv = Color::HSV(hue, 255, brightness);
                    matrix.setTextColor(hsv.toRGB().gammaApplied().packed565());
                    matrix.write(message[i]);
                }
            }
            else {
                // Convert to HSV and back to replace brightness info with our own brightness setting.
                auto hsv = Color::HSV::fromRGB(color).withValue(brightness);
                matrix.setTextColor(hsv.toRGB().gammaApplied().packed565());
                matrix.print(message);                
            }

            matrix.show();

            // uint32_t drawFinish = millis();
            // LOGLN(drawFinish - drawStart);

            position--;
            if( position < -messageWidth) {
                position = matrix.width();
                
                // The hue continues where it left off at the end of the string's last character's color.
                startHue = (startHue + (len * hueStep)) & 0xFFFF;
            }
        }
    }
    
    // Message is truncated at maxMessageLength if it's too large for the buffer.
    void setMessage(const char* str) {
        strncpy(message, str, messageBufferSize);
        message[messageBufferSize - 1] = 0;
        resetScroll();
    }

    const char* getMessage() const {
        return message;
    }
                
    void setRotation(uint8_t r) {
        r = max((uint8_t)0, r);
        r = min((uint8_t)3, r);
        matrixRotation = r;
        matrix.setRotation(r);
    }

    uint8_t getRotation() const {
        return matrixRotation;
    }

    void setScrollDelay(uint8_t d) {
        scrollDelay = d;
    }

    uint8_t getScrollDelay() const {
        return scrollDelay;
    }

    void setColor(const Color::RGB& newColor) {
        if (newColor == color) {
            return;
        }

        if (newColor.isBlack()) {
            startHue = 0;
            color = newColor;
        } else {
            // Brightness is applied separately later, so color is stored with max brightness.
            color = Color::HSV::fromRGB(newColor).withValue(255).toRGB();
        }
    }

    const Color::RGB& getColor() const {
        return color;
    }

    void setBrightness(uint8_t b) {
        brightness = b;
    }

    uint8_t getBrightness() const {
        return brightness;
    }

    void setFontID(Font::ID id) {
        fontID = id;
        matrix.setFont(Font::withID(id).gfxFont);
        resetScroll();        
    }

    inline const Font::ID getFontID() const {
        return fontID;
    }

private:
    // LED matrix
    Adafruit_IS31FL3741_QT_buffered& matrix;

    // Rotation settings
    uint8_t matrixRotation = 0;

    // font settings
    Font::ID fontID = Font::ID::adafruit;

    // Color
    Color::RGB color;    

    // Brightness value
    uint8_t brightness = 255;

    // message buffer
    char message[messageBufferSize] = {0};
    int messageWidth;    

    // marque position and speed
    int32_t position;
    uint32_t scrollElapsed = 0;

    // rainbow mode
    static constexpr uint16_t hueStep = (65536 / 12);
    uint16_t startHue = 0;
    
    // Speed settings
    uint8_t scrollDelay = 50;
};
