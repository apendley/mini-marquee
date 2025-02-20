#pragma once

#include <Arduino.h>

// All of the marquee settings we're tracking here have the same structure,
// which is essentially enumerated values with associated metadata.
// This template provides a consistent interface to that structure.
template <typename T>
class IndexedSetting {
public:
    IndexedSetting(const T* items, uint8_t count, uint8_t initialIndex = 0) : 
        _items(items), 
        _count(count), 
        _index(initialIndex) 
    {

    }

    uint8_t count() const { return _count; }

    const T& get(uint8_t index) const { 
        return (index < _count) ? _items[index] : _items[0]; 
    }

    const T& current() const { 
        return _items[_index]; 
    }

    uint8_t currentIndex() const { 
        return _index; 
    }

    // Returns true if index was actually set, false otherwise.
    bool setIndex(uint8_t index) {
        if (index >= _count || index == _index) {
            return false;
        }

        _index = index;
        return true;
    }

private:
    const T* _items;
    uint8_t _count;
    uint8_t _index;
};

// Main settings model interface
class Settings {
public:
    struct Color {
        const char* name;
        const char* hexString;
    };

    struct UnsignedByte {
        const char* name;
        const uint8_t value;
    };

public:
    Settings();

    IndexedSetting<Color> colors;
    IndexedSetting<UnsignedByte> fonts;
    IndexedSetting<UnsignedByte> scrollDelays;
    IndexedSetting<UnsignedByte> brightnessValues;
    IndexedSetting<UnsignedByte> displayRotations;
};
