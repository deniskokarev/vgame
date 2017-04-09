/**
 * Simple stub to make Adafruit GFX lib happy on STM32 HAL
 * necessary to mimic the basic Arduino IDE library the GFX
 * depends on
 * @author Denis Kokarev
 */
#include <cstdint>
#include <cstring>
#include <cstdlib>

typedef bool boolean;
typedef char* __FlashStringHelper;

/**
 * Arduino has a concept of Printer class, which Adafruit GFX inherts,
 * so we need to have our own rudimentary implementation
 */
struct Print {
	virtual void write(uint8_t) = 0;
	void print(const char *s) {
		while (*s)
			write(*s++);
	}
};

inline uint8_t _pgm_read_byte(const void *addr) {
	return *(static_cast<const uint8_t*>(addr));
}
#define pgm_read_byte(addr) _pgm_read_byte(addr)

inline uint16_t _pgm_read_word(const void *addr) {
	return *(static_cast<const uint16_t*>(addr));
}
#define pgm_read_word(addr) _pgm_read_word(addr)

inline uint32_t _pgm_read_dword(const void *addr) {
	return *(static_cast<const uint32_t*>(addr));
}
#define pgm_read_dword(addr) _pgm_read_dword(addr)
