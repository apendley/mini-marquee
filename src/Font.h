#pragma once

#include <Arduino.h>
#include <Adafruit_GFX.h>

struct Font {
    enum class ID: uint8_t {
        adafruit,
        fixed,
        fixedMono,
        ancient
    };

    static const Font& withID(ID id);

    struct AdafruitFontInfo {
        static const uint8_t xAdvance = 6;
        static const uint8_t fontHeight = 6;
    };

    uint32_t charWidth(uint8_t c) const {
        if (gfxFont == nullptr) {
            return AdafruitFontInfo::xAdvance;
        }

        if (c < gfxFont->first || c > gfxFont->last) {
            return AdafruitFontInfo::xAdvance;
        }

        return gfxFont->glyph[c - gfxFont->first].xAdvance;
    }
    

    uint32_t textWidth(const char* str) const {
        const uint32_t len = strlen(str);
        
        if (gfxFont == nullptr) {
            return len * AdafruitFontInfo::xAdvance;
        }
        
        uint32_t w = 0;
    
        for (uint32_t i = 0; i < len; i++) {
            w += charWidth(str[i]);
        }
        
        return w;
    }

    const GFXfont* gfxFont;
    const uint8_t yOffset;
};