#include "transliterateUTF8.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "anyascii.h"
#include "utf8.h"

#ifdef __cplusplus
}
#endif

bool transliterateUTF8(const char* input, char* output, size_t outputSize) {
    if (input == nullptr || output == nullptr) {
        return false;
    }

    // State tracking
	uint32_t utf32;
	uint32_t state = 0;

    // Reserve space for null character
    size_t remainingSize = outputSize - 1;    

    // Verification
    size_t expectedWritten = 0;
    size_t written = 0;    
    
    // Perform the transliteration from UTF8 code points to ASCII.
    while(*input != 0) {
		utf8_decode(&state, &utf32, (unsigned char) *input);

        if (state == UTF8_ACCEPT) {
        	const char *r;
			size_t rlen = anyascii(utf32, &r);
            expectedWritten += rlen;

            if (rlen <= remainingSize) {
			    memcpy(output, r, rlen);
			    output += rlen;
                remainingSize -= rlen;            
                written += rlen;
            } else {
                break;
            }
        } else if (state == UTF8_REJECT) {
            state = UTF8_ACCEPT;
        }

        input++;
	}
    
    // Write the terminating null character
	*output = 0;

    // If we wrote as many bytes as we expected to write, we're good.
    return written == expectedWritten;
}
