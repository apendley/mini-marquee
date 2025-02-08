#pragma once

#include <Arduino.h>

// Since the MiniMarquee can only render ASCII characters, decode
// UTF8 characters in the input into ASCII characters so the
// mini-marquee can render some sensible representation of them..
bool transliterateUTF8(const char* input, char* output, size_t outputSize);