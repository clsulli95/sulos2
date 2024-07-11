#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <limits.h>

// Check if the compiler thinks you are targeting the wrong operating system
#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

// This tutorial will only work for the 32-bit ix86 targets
#if !defined(__i386__)
#error "This tutorial needs to be compiled with a ix86-elf compiler"
#endif

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;

typedef struct TerminalState {
  size_t row;
  size_t col;
  uint8_t color;
  uint16_t* buffer;
} TerminalState;

// Hardware text mode color constants
typedef enum VgaColor {
  VGA_COLOR_BLACK = 0,
  VGA_COLOR_BLUE = 1,
	VGA_COLOR_GREEN = 2,
	VGA_COLOR_CYAN = 3,
	VGA_COLOR_RED = 4,
	VGA_COLOR_MAGENTA = 5,
	VGA_COLOR_BROWN = 6,
	VGA_COLOR_LIGHT_GREY = 7,
	VGA_COLOR_DARK_GREY = 8,
	VGA_COLOR_LIGHT_BLUE = 9,
	VGA_COLOR_LIGHT_GREEN = 10,
	VGA_COLOR_LIGHT_CYAN = 11,
	VGA_COLOR_LIGHT_RED = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_BROWN = 14,
	VGA_COLOR_WHITE = 15,
} VgaColor;

static TerminalState state;

static inline void TERM_InitializeTerminalState() {
  state.row = 0;
  state.col = 0;
  state.color = 0;
  state.buffer = NULL;
}

static inline uint8_t TERM_VgaEntryColor(VgaColor fg, VgaColor bg) {
  return fg | bg << 4;
}

static inline uint16_t TERM_VgaEntry(uint8_t c, uint8_t color) {
  return (uint16_t) c | (uint16_t) color << 8;
}

size_t UTIL_SafeStrLen_API(const char* str) {
  // Catch nullptr
  if (str == NULL) {
    return 0;
  }

  size_t length = 0;
  size_t maxLength = SIZE_MAX;

  while (str[length] && length < maxLength) {
    length++;
  }

  // Error handling
  if (length == maxLength) {
    return SIZE_MAX;
  }

  return length;
}

void TERM_Initialize_API(void) {
  TERM_InitializeTerminalState();

  state.color = TERM_VgaEntryColor(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
  state.buffer = (uint16_t*) 0xB8000;

  for (size_t y = 0; y < VGA_HEIGHT; y++) { 
    for (size_t x = 0; x < VGA_WIDTH; x++) {
      const size_t index = y * VGA_WIDTH + x;
      state.buffer[index] = TERM_VgaEntry(' ', state.color);
    }
  }
}

void TERM_SetColor(uint8_t color) {
  state.color = color;
}

void TERM_PutEntryAt(char c, uint8_t color, size_t x, size_t y) {
  const size_t index = y * VGA_WIDTH + x;
  state.buffer[index] = TERM_VgaEntry(c, color);
}

void TERM_PutChar(char c) {
  if (c == '\n') {
    state.row++;
    state.col = 0;
  } else {
    TERM_PutEntryAt(c, state.color, state.col, state.row);

    if (state.col < VGA_WIDTH ) {
      state.col++;
    } else {
      state.col = 0;
      if (state.row < VGA_HEIGHT) {
        state.row++;
      } else {
        state.row = 0;
      }
    }
  }
}

void TERM_Write(const char* data, size_t size) {
  for (size_t i = 0; i < size; i++) {
    TERM_PutChar(data[i]);
  }
}

void TERM_WriteString_API(const char* data) {
  TERM_Write(data, UTIL_SafeStrLen_API(data));
}

void kernel_main(void) {
  TERM_Initialize_API();

  TERM_WriteString_API("Hello, kernel World!\n");
  TERM_WriteString_API("Line 2!\n");
}

