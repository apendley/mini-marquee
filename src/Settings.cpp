#include "Settings.h"
#include "Font.h"

namespace {
    // A lot of these colors are biased away from red because red is
    // heavily over-represented on this matrix when powered by 3.3 volts.    
    const Settings::Color _colors[] = {
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

    const Settings::UnsignedByte _fonts[] = {
        {"Adafruit", uint8_t(Font::ID::adafruit)},
        {"Fixed", uint8_t(Font::ID::fixed)},
        {"Fixed Mono", uint8_t(Font::ID::fixedMono)},
        {"Ancient", uint8_t(Font::ID::ancient)},
    };

    const Settings::UnsignedByte _scrollDelays[] = {
        {"Very Slow", 100},
        {"Slow", 75},
        {"Moderate", 50},
        {"Fast", 30},
        {"Very Fast", 20},
    };

    const Settings::UnsignedByte _brightnessValues[] = {
        {"Very Dim", 70},
        {"Dim", 126},
        {"Moderate", 170},
        {"Bright", 220},
        {"Very Bright", 255},
    };

    const Settings::UnsignedByte _displayRotations[] = {
        {"Down", 0},
        {"Left", 1},
        {"Up", 2},
        {"Right", 3},
    };
}

Settings::Settings() : 
    colors(_colors, sizeof(_colors) / sizeof(_colors[0]), 0),
    fonts(_fonts, sizeof(_fonts) / sizeof(_fonts[0]), 0),
    scrollDelays(_scrollDelays, sizeof(_scrollDelays) / sizeof(_scrollDelays[0]), 1),
    brightnessValues(_brightnessValues, sizeof(_brightnessValues) / sizeof(_brightnessValues[0]), 2),
    displayRotations(_displayRotations, sizeof(_displayRotations) / sizeof(_displayRotations[0]), 2) 
{
    
}
