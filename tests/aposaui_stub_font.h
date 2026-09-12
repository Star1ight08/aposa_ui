#ifndef APOSAUI_STUB_FONT_H
#define APOSAUI_STUB_FONT_H

#include "aposaui.h"

/*
 * A font backend for builds that compile the platform half out. aposaui.h
 * declares the font interface and the Win32 section implements it against GDI;
 * a build that defines APOSAUI_NO_PLATFORM supplies its own instead, and this
 * is that one. Every codepoint maps to the same solid square glyph, so text
 * layout and blending can be checked exactly without a display.
 */
#define APOSAUI_STUB_FONT_SIZE 4u
#define APOSAUI_STUB_GLYPH_ADVANCE 2
#define APOSAUI_STUB_GLYPH_SIDE 2u

/**
 * aposaui_stub_allocator - allocator backing the stub font
 *
 * Return: an allocator the stub backend allocates the font through.
 */
AposaUiAllocator aposaui_stub_allocator(void);

#endif // APOSAUI_STUB_FONT_H
