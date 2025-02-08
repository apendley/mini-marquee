// Adapted by Aaron Pendley from Adafruit_NeoPixel library. 

// Original license:
/*!
 * @section author Author
 *
 * Written by Phil "Paint Your Dragon" Burgess for Adafruit Industries,
 * with contributions by PJRC, Michael Miller and other members of the
 * open source community.
 *
 * @section license License
 *
 * This file is part of the Adafruit_NeoPixel library.
 *
 * Adafruit_NeoPixel is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * Adafruit_NeoPixel is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with NeoPixel. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

#include <Arduino.h>

inline void hsv2rgb(uint16_t h, uint8_t s, uint8_t v, uint8_t& r, uint8_t& g, uint8_t& b) {
    // Remap 0-65535 to 0-1529. Pure red is CENTERED on the 64K rollover;
    // 0 is not the start of pure red, but the midpoint...a few values above
    // zero and a few below 65536 all yield pure red (similarly, 32768 is the
    // midpoint, not start, of pure cyan). The 8-bit RGB hexcone (256 values
    // each for red, green, blue) really only allows for 1530 distinct hues
    // (not 1536, more on that below), but the full unsigned 16-bit type was
    // chosen for hue so that one's code can easily handle a contiguous color
    // wheel by allowing hue to roll over in either direction.    
    uint16_t hue = (h * 1530L + 32768) / 65536;

    // Because red is centered on the rollover point (the +32768 above,
    // essentially a fixed-point +0.5), the above actually yields 0 to 1530,
    // where 0 and 1530 would yield the same thing. Rather than apply a
    // costly modulo operator, 1530 is handled as a special case below.

    // So you'd think that the color "hexcone" (the thing that ramps from
    // pure red, to pure yellow, to pure green and so forth back to red,
    // yielding six slices), and with each color component having 256
    // possible values (0-255), might have 1536 possible items (6*256),
    // but in reality there's 1530. This is because the last element in
    // each 256-element slice is equal to the first element of the next
    // slice, and keeping those in there this would create small
    // discontinuities in the color wheel. So the last element of each
    // slice is dropped...we regard only elements 0-254, with item 255
    // being picked up as element 0 of the next slice. Like this:
    // Red to not-quite-pure-yellow is:        255,   0, 0 to 255, 254,   0
    // Pure yellow to not-quite-pure-green is: 255, 255, 0 to   1, 255,   0
    // Pure green to not-quite-pure-cyan is:     0, 255, 0 to   0, 255, 254
    // and so forth. Hence, 1530 distinct hues (0 to 1529), and hence why
    // the constants below are not the multiples of 256 you might expect.

    // Convert hue to R,G,B (nested ifs faster than divide+mod+switch):

    // Red to Green-1    
    if (hue < 510) {
        b = 0;

        // Red to Yellow-1
        if (hue < 255) {
            r = 255;
            // g = 0 to 254
            g = hue;
        } 
        // Yellow to Green-1
        else {
            // r = 255 to 1
            r = 510 - hue;
            g = 255;
        }
    } 
    // Green to Blue-1
    else if (hue < 1020) { 
        r = 0;

        // Green to Cyan-1
        if (hue < 765) {
            g = 255;
            // b = 0 to 254
            b = hue - 510;
        } 
        // Cyan to Blue-1
        else {
            // g = 255 to 1
            g = 1020 - hue;
            b = 255;
        }
    } 
    // Blue to Red-1
    else if (hue < 1530) {
        g = 0;

        // Blue to Magenta-1
        if (hue < 1275) {
            // r = 0 to 254
            r = hue - 1020;
            b = 255;
        } 
        // Magenta to Red-1
        else {
            r = 255;
            // b = 255 to 1
            b = 1530 - hue;
        }
    } 
    // Last 0.5 Red (quicker than % operator)
    else {
        r = 255;
        g = b = 0;
    }

    // Apply saturation and value to R,G,B, pack into 32-bit result:
    uint32_t v1 = 1 + v;  // 1 to 256; allows >>8 instead of /255
    uint16_t s1 = 1 + s;  // 1 to 256; same reason
    uint8_t s2 = 255 - s; // 255 to 0

    r = ((((r * s1) >> 8) + s2) * v1) >> 8;
    g = ((((g * s1) >> 8) + s2) * v1) >> 8;
    b = ((((b * s1) >> 8) + s2) * v1) >> 8;
}