#include "Font.h"
#include "fonts/Font5x7Fixed.h"
#include "fonts/Font5x7FixedMono.h"
#include "fonts/Ancient4x8.h"

namespace  {
    const Font fonts[]= {
        {nullptr, 1},
        {&Font5x7Fixed, 8},
        {&Font5x7FixedMono, 8},
        {&Ancient4x8, 9}
    };

    const uint8_t count = sizeof(fonts) / sizeof(fonts[0]);
}

const Font& Font::withID(ID id) {
    uint8_t index = min(uint8_t(id), count);
    return fonts[index];
}
