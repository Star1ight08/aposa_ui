#include "aposaui_stub_font.h"

#include <stdlib.h>

/*
 * aposaui.h leaves AposaUiFont opaque precisely so the backend owns its layout.
 * This one is as small as it can be: a fixed glyph shared by every codepoint.
 */
struct AposaUiFont {
	AposaUiAllocator allocator;
	uint32_t pixel_size;
	uint32_t ascent;
	uint32_t line_height;
	AposaUiGlyph glyph;
	uint8_t coverage[APOSAUI_STUB_GLYPH_SIDE * APOSAUI_STUB_GLYPH_SIDE];
};

static void *aposaui_stub_allocate(void *user, size_t size, size_t alignment) {
	(void)user;
	(void)alignment;
	return malloc(size);
}

static void aposaui_stub_deallocate(void *user, void *memory) {
	(void)user;
	free(memory);
}

AposaUiAllocator aposaui_stub_allocator(void) {
	const AposaUiAllocator allocator = {
	    .user = NULL, .allocate = aposaui_stub_allocate, .deallocate = aposaui_stub_deallocate};

	return allocator;
}

int aposaui_font_create(const AposaUiAllocator *allocator, const char *family, uint32_t pixel_size,
			AposaUiFont **out_font) {
	AposaUiFont *font;

	if (allocator == NULL || allocator->allocate == NULL || allocator->deallocate == NULL || family == NULL ||
	    pixel_size == 0u || out_font == NULL) {
		return -1;
	}
	font = allocator->allocate(allocator->user, sizeof(*font), _Alignof(AposaUiFont));
	if (font == NULL) {
		return -1;
	}
	*font = (AposaUiFont){.allocator = *allocator,
			      .pixel_size = pixel_size,
			      .ascent = pixel_size,
			      .line_height = pixel_size + 2u,
			      .glyph = {.advance = APOSAUI_STUB_GLYPH_ADVANCE,
					.bearing_x = 0,
					.bearing_y = (int32_t)pixel_size,
					.width = APOSAUI_STUB_GLYPH_SIDE,
					.height = APOSAUI_STUB_GLYPH_SIDE,
					.coverage = font->coverage}};
	for (size_t index = 0u; index < sizeof(font->coverage); ++index) {
		font->coverage[index] = 0xffu;
	}
	*out_font = font;
	return 0;
}

void aposaui_font_destroy(AposaUiFont *font) {
	if (font == NULL) {
		return;
	}
	font->allocator.deallocate(font->allocator.user, font);
}

uint32_t aposaui_font_ascent(const AposaUiFont *font) {
	return font == NULL ? 0u : font->ascent;
}

uint32_t aposaui_font_line_height(const AposaUiFont *font) {
	return font == NULL ? 0u : font->line_height;
}

const AposaUiGlyph *aposaui_font_glyph(AposaUiFont *font, uint32_t codepoint) {
	(void)codepoint;
	return font == NULL ? NULL : &font->glyph;
}
