#pragma once

#include <Arduino.h>
#include "hsv2rgb.h"

namespace Color {
    uint8_t sine8(uint8_t theta);
    uint8_t gamma8(uint8_t x);

    inline uint8_t scale8(uint8_t c, uint8_t brightness) {
        return ((uint16_t)c * brightness) / 255;        
    }
}

namespace Color {
    struct RGB {
        inline RGB() : r(0), g(0), b(0) { }
        inline RGB(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b) { }
        inline RGB(uint32_t packedColor) : r(packedColor >> 16), g(packedColor >> 8), b(packedColor) { }

        static RGB gray(uint8_t brightness) {
            return RGB(brightness, brightness, brightness);
        }

        inline bool operator==(const RGB& other) const {
            return r == other.r && g == other.g && b == other.b;
        }

        inline bool operator!=(const RGB& other) const {
            return !(*this == other);
        }

        inline bool isBlack() const {
            return (r == 0) && (g == 0) & (b == 0);
        }

        inline uint32_t packed() const {
            return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
        }

        inline uint16_t packed565() const {
            return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
        }

        inline void scale(uint8_t brightness) {
            r = scale8(r, brightness);
            g = scale8(g, brightness);
            b = scale8(b, brightness);
        }

        inline RGB scaled(uint8_t brightness) const {
            RGB rgb(r, g, b);
            rgb.scale(brightness);
            return rgb;
        }

        inline void applyGamma() {
            r = gamma8(r);
            g = gamma8(g);
            b = gamma8(b);
        }

        inline RGB gammaApplied() const {
            RGB rgb(r, g, b);
            rgb.applyGamma();
            return rgb;
        }

        // "#RRGGBB": # symbol + 6 characters for RGB components + terminating null character
        static constexpr int hexStringBufferSize = 8;
        typedef char HexStringBuffer[hexStringBufferSize];

        inline bool toHexString(HexStringBuffer buffer) const {
            int written = snprintf(buffer, hexStringBufferSize, "#%02X%02X%02X", r, g, b);
            return (written >= 0 && written < hexStringBufferSize);
        }

        static inline RGB fromHexString(const char* colorString) {
            if (colorString == nullptr || strlen(colorString) < 7) { 
                return RGB();
            }

            char colorBuffer[3] = {0};
            colorBuffer[2] = 0;

            // skip the #                    
            uint8_t i = 1;

            // First two bytes are red
            colorBuffer[0] = colorString[i++];
            colorBuffer[1] = colorString[i++];
            uint8_t red = strtol(colorBuffer, NULL, 16);

            // Next two bytes are green
            colorBuffer[0] = colorString[i++];
            colorBuffer[1] = colorString[i++];
            uint8_t green = strtol(colorBuffer, NULL, 16);

            // Next two bytes are blue
            colorBuffer[0] = colorString[i++];
            colorBuffer[1] = colorString[i++];
            uint8_t blue = strtol(colorBuffer, NULL, 16);                

            return RGB(red, green, blue);
        }           

        uint8_t r;
        uint8_t g;
        uint8_t b;
    } __attribute__((packed));

    struct HSV {
        inline HSV(uint16_t h, uint8_t s = 255, uint8_t v = 255) 
            : h(h), s(s), v(v)
        {
        }

        HSV withValue(uint8_t value) {
            return HSV(h, s, value);
        }

        static inline HSV fromRGB(uint8_t r, uint8_t g, uint8_t b) {
            uint8_t rangeMin = min(min(r, g), b);
            uint8_t rangeMax = max(max(r, g), b);
            
            uint8_t v = rangeMax;            
            if (v == 0) {
                return HSV(0, 0, 0);
            }

            uint8_t s = 255 * long(rangeMax - rangeMin) / v;
            if (s == 0) {
                return HSV(0, 0, v);
            }

            uint16_t h;            
        
            if (rangeMax == r) {
                h = 0 + 11008 * (g - b) / (rangeMax - rangeMin);
            }
            else if (rangeMax == g) {
                h = 21760 + 11008 * (b - r) / (rangeMax - rangeMin);
            }
            else {
                h = 43776 + 11008 * (r - g) / (rangeMax - rangeMin);
            }
        
            return HSV(h, s, v);
        }

        static inline HSV fromRGB(RGB rgb) {
            return fromRGB(rgb.r, rgb.g, rgb.b);
        }

        inline RGB toRGB() const {
            uint8_t r, g, b;
            hsv2rgb(h, s, v, r, g, b);
            return RGB(r, g, b);
        }

        uint16_t h;
        uint8_t s;
        uint8_t v;
    } __attribute__((packed));
}
