/*
 * Copyright 2026 Star1ight08
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* aposaui - a single-header Win32 UI toolkit.
 *
 * Usage:
 *
 *     #define APOSAUI_IMPLEMENTATION
 *     #include "aposaui.h"
 *
 * in exactly one translation unit, and plain
 *
 *     #include "aposaui/aposaui.h"
 *
 * everywhere else. The implementation section sits after the header guard on
 * purpose, so including this file for declarations first and again for the
 * implementation both work.
 *
 * Configuration:
 *
 *     APOSAUI_NO_PLATFORM  compile the portable half only. The canvas, theme,
 *                          font interface and widgets then build and link
 *                          without user32 or gdi32, which is what the headless
 *                          UI tests rely on.
 *     APOSAUI_STATIC       make every declaration and definition static, so the
 *                          implementation is private to the translation unit
 *                          that instantiates it.
 *
 * Text the library draws is UTF-8. Wide strings appear only where the
 * underlying Win32 API is intrinsically wide.
 *
 * Naming: public symbols are aposaui_; private functions and file-scope objects
 * are aposaui__ (double underscore), so a caller who writes a wrapper named
 * aposaui_something cannot collide with a future private helper.
 */

#ifndef APOSAUI_H
#define APOSAUI_H

#define APOSAUI_VERSION_MAJOR 0
#define APOSAUI_VERSION_MINOR 1

#ifdef APOSAUI_STATIC
#define APOSAUI_DEF static
#else
#define APOSAUI_DEF extern
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*
 * The platform half of the API speaks Win32 types, so windows.h has to be
 * visible to callers. A build that defines APOSAUI_NO_PLATFORM never sees it.
 */
#ifndef APOSAUI_NO_PLATFORM
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------
 * Portable: allocator, colour, rectangle, canvas
 * ---------------------------------------------------------------------- */

/*
 * Allocation is always caller-supplied, so the library never reaches for
 * malloc and an embedder can route every allocation through its own arena.
 * The stdlib-backed allocator the implementations use ignores @user.
 */
typedef struct AposaUiAllocator {
	void *user;
	void *(*allocate)(void *user, size_t size, size_t alignment);
	void (*deallocate)(void *user, void *memory);
} AposaUiAllocator;

typedef uint32_t AposaUiColor;

/*
 * Rectangles carry signed coordinates and extents so a layout that does not fit
 * its window produces a degenerate rectangle instead of wrapping around zero.
 * Every primitive clips against the canvas, so out-of-range rectangles are safe.
 */
typedef struct AposaUiRect {
	int32_t x;
	int32_t y;
	int32_t width;
	int32_t height;
} AposaUiRect;

typedef struct AposaUiCanvas {
	uint32_t width;
	uint32_t height;
	uint32_t stride;
	uint8_t *pixels;
} AposaUiCanvas;

/**
 * aposaui_rect_contains - test whether a point lies inside a rectangle
 * @rect: rectangle to test
 * @x: point x
 * @y: point y
 *
 * Return: true when the point is inside the half-open rectangle.
 */
APOSAUI_DEF bool aposaui_rect_contains(AposaUiRect rect, int32_t x, int32_t y);

/**
 * aposaui_rect_inset - shrink a rectangle on every side
 * @rect: rectangle to shrink
 * @amount: distance to move each edge inwards, may be negative
 *
 * Return: the inset rectangle, degenerate when @amount exceeds half the extent.
 */
APOSAUI_DEF AposaUiRect aposaui_rect_inset(AposaUiRect rect, int32_t amount);

/**
 * aposaui_paint_fill_rect - write an opaque rectangle
 * @canvas: destination canvas
 * @rect: rectangle in canvas pixels
 * @color: 0xAARRGGBB colour whose alpha is ignored
 */
APOSAUI_DEF void aposaui_paint_fill_rect(AposaUiCanvas *canvas, AposaUiRect rect, AposaUiColor color);

/**
 * aposaui_paint_blend_rect - blend a rectangle using the colour alpha
 * @canvas: destination canvas
 * @rect: rectangle in canvas pixels
 * @color: 0xAARRGGBB colour
 */
APOSAUI_DEF void aposaui_paint_blend_rect(AposaUiCanvas *canvas, AposaUiRect rect, AposaUiColor color);

/**
 * aposaui_paint_rounded_rect - blend a rectangle with antialiased round corners
 * @canvas: destination canvas
 * @rect: rectangle in canvas pixels
 * @radius: corner radius in pixels, clamped to half the shorter side
 * @color: 0xAARRGGBB colour
 */
APOSAUI_DEF void aposaui_paint_rounded_rect(AposaUiCanvas *canvas, AposaUiRect rect, double radius, AposaUiColor color);

/**
 * aposaui_paint_disc - blend an antialiased filled circle
 * @canvas: destination canvas
 * @center_x: circle centre x in pixels
 * @center_y: circle centre y in pixels
 * @radius: circle radius in pixels
 * @color: 0xAARRGGBB colour
 */
APOSAUI_DEF void aposaui_paint_disc(AposaUiCanvas *canvas, double center_x, double center_y, double radius,
				    AposaUiColor color);

/**
 * aposaui_paint_ring - blend an antialiased circle outline
 * @canvas: destination canvas
 * @center_x: circle centre x in pixels
 * @center_y: circle centre y in pixels
 * @radius: radius of the stroke centreline in pixels
 * @thickness: stroke width in pixels
 * @color: 0xAARRGGBB colour
 */
APOSAUI_DEF void aposaui_paint_ring(AposaUiCanvas *canvas, double center_x, double center_y, double radius,
				    double thickness, AposaUiColor color);

/**
 * aposaui_paint_blend_pixel - blend one pixel with an extra coverage factor
 * @canvas: destination canvas
 * @x: pixel column, ignored when outside the canvas
 * @y: pixel row, ignored when outside the canvas
 * @color: 0xAARRGGBB colour
 * @coverage: additional 0-255 coverage multiplied onto the colour alpha
 */
APOSAUI_DEF void aposaui_paint_blend_pixel(AposaUiCanvas *canvas, int32_t x, int32_t y, AposaUiColor color,
					   uint32_t coverage);

/* -------------------------------------------------------------------------
 * Portable: theme and metrics
 * ---------------------------------------------------------------------- */

/*
 * The canvas colour must stay 0x202020: cpu_renderer_pack_pixel() hard-codes 32
 * as the background it composites straight and premultiplied alpha against, so
 * changing one without the other blends transparent images onto the wrong base.
 */
#define APOSAUI_COLOR_CANVAS 0xff202020u
#define APOSAUI_COLOR_BAR 0xff161616u
#define APOSAUI_COLOR_SEPARATOR 0xff2a2a2au
#define APOSAUI_COLOR_TEXT_PRIMARY 0xffe6e6e6u
#define APOSAUI_COLOR_TEXT_SECONDARY 0xff8a8a8au
#define APOSAUI_COLOR_TEXT_DIM 0xff6a6a6au
#define APOSAUI_COLOR_STATUS_DOT 0xff7ac8a0u
#define APOSAUI_COLOR_BADGE_BACKGROUND 0xff242424u
#define APOSAUI_COLOR_BADGE_TEXT 0xffa0a0a0u
#define APOSAUI_COLOR_CONTROL_STROKE 0xff8a8a8au
#define APOSAUI_COLOR_CONTROL_STROKE_ACTIVE 0xffe6e6e6u
#define APOSAUI_COLOR_CONTROL_HOVER 0x18ffffffu
#define APOSAUI_COLOR_CONTROL_PRESSED 0x30ffffffu

#define APOSAUI_BASE_DPI 96u

typedef enum AposaUiThemeMode {
	APOSAUI_THEME_MODE_DARK,
	APOSAUI_THEME_MODE_LIGHT,
} AposaUiThemeMode;

typedef struct AposaUiThemeColors {
	AposaUiColor canvas;
	AposaUiColor bar;
	AposaUiColor separator;
	AposaUiColor text_primary;
	AposaUiColor text_secondary;
	AposaUiColor text_dim;
	AposaUiColor status_dot;
	AposaUiColor badge_background;
	AposaUiColor badge_text;
	AposaUiColor control_stroke;
	AposaUiColor control_stroke_active;
	AposaUiColor control_hover;
	AposaUiColor control_pressed;
	AposaUiColor panel;
	AposaUiColor panel_sidebar;
	AposaUiColor panel_border;
	AposaUiColor option_selected;
	AposaUiColor option_accent;
} AposaUiThemeColors;

typedef struct AposaUiMetrics {
	uint32_t top_bar_height;
	uint32_t bottom_bar_height;
	uint32_t separator_height;
	uint32_t padding_x;
	uint32_t gap;
	uint32_t primary_font_size;
	uint32_t secondary_font_size;
	uint32_t button_size;
	uint32_t button_gap;
	uint32_t status_dot_radius;
	uint32_t badge_padding_x;
	uint32_t badge_height;
} AposaUiMetrics;

/**
 * aposaui_stroke_width - line width for the drawn icons and outlines
 * @metrics: scaled metrics for the display density
 *
 * Return: the stroke width in physical pixels, never below 1.
 */
APOSAUI_DEF int32_t aposaui_stroke_width(const AposaUiMetrics *metrics);

/**
 * aposaui_metrics_for_dpi - scale the interface metrics for a display density
 * @dpi: display density in dots per inch, treated as 96 when zero
 *
 * Return: metrics in physical pixels.
 */
APOSAUI_DEF AposaUiMetrics aposaui_metrics_for_dpi(uint32_t dpi);

/**
 * aposaui_theme_colors - select the palette for a theme mode
 * @mode: dark or light
 *
 * Return: the colour set for @mode.
 */
APOSAUI_DEF AposaUiThemeColors aposaui_theme_colors(AposaUiThemeMode mode);

/* -------------------------------------------------------------------------
 * Portable: font interface
 *
 * AposaUiFont is opaque; the backend that fills it in lives in the platform
 * section. Only aposaui_font_create/destroy/ascent/line_height/glyph are
 * backend-specific, and aposaui_font_measure_text/draw_text are written against
 * this interface alone.
 * ---------------------------------------------------------------------- */

typedef struct AposaUiGlyph {
	int32_t advance;
	int32_t bearing_x;
	int32_t bearing_y;
	uint32_t width;
	uint32_t height;
	const uint8_t *coverage;
} AposaUiGlyph;

typedef struct AposaUiFont AposaUiFont;

/**
 * aposaui_font_create - open a system font at a pixel size
 * @allocator: allocator owning the font and its glyph cache, copied by value
 * @family: UTF-8 family name, substituted by the system when unavailable
 * @pixel_size: em size in pixels
 * @out_font: receives the font on success
 *
 * Return: 0 on success, or a negative value when the font cannot be opened.
 */
APOSAUI_DEF int aposaui_font_create(const AposaUiAllocator *allocator, const char *family, uint32_t pixel_size,
				    AposaUiFont **out_font);

/**
 * aposaui_font_destroy - release a font and every cached glyph
 * @font: font to destroy, may be NULL
 */
APOSAUI_DEF void aposaui_font_destroy(AposaUiFont *font);

/**
 * aposaui_font_ascent - distance from the baseline to the top of the line
 * @font: font to inspect
 *
 * Return: ascent in pixels, or 0 when @font is NULL.
 */
APOSAUI_DEF uint32_t aposaui_font_ascent(const AposaUiFont *font);

/**
 * aposaui_font_line_height - full line advance
 * @font: font to inspect
 *
 * Return: line height in pixels, or 0 when @font is NULL.
 */
APOSAUI_DEF uint32_t aposaui_font_line_height(const AposaUiFont *font);

/**
 * aposaui_font_glyph - look up a glyph, rasterizing it on first use
 * @font: font whose glyph cache is updated
 * @codepoint: Unicode scalar value
 *
 * Return: borrowed glyph valid until the font is destroyed, or NULL when the
 * codepoint cannot be rasterized or the cache is exhausted.
 */
APOSAUI_DEF const AposaUiGlyph *aposaui_font_glyph(AposaUiFont *font, uint32_t codepoint);

/**
 * aposaui_font_measure_text - total advance of a UTF-8 string
 * @font: font whose glyph cache is updated
 * @text: NUL-terminated UTF-8 string
 *
 * Return: advance width in pixels, or 0 when @font or @text is NULL.
 */
APOSAUI_DEF int32_t aposaui_font_measure_text(AposaUiFont *font, const char *text);

/**
 * aposaui_font_draw_text - blend a UTF-8 string onto a canvas
 * @canvas: destination canvas
 * @font: font whose glyph cache is updated
 * @x: pen start x in canvas pixels
 * @baseline_y: baseline y in canvas pixels
 * @text: NUL-terminated UTF-8 string
 * @color: 0xAARRGGBB colour
 *
 * Return: the pen x position after the last glyph.
 */
APOSAUI_DEF int32_t aposaui_font_draw_text(AposaUiCanvas *canvas, AposaUiFont *font, int32_t x, int32_t baseline_y,
					   const char *text, AposaUiColor color);

/**
 * aposaui_text_baseline - vertical centring baseline for a rectangle
 * @rect: box the text is centred in
 * @font: font the text is drawn with
 *
 * Return: the baseline y that centres one line of @font inside @rect.
 */
APOSAUI_DEF int32_t aposaui_text_baseline(AposaUiRect rect, const AposaUiFont *font);

/**
 * aposaui_format_duration - render a millisecond duration as m:ss
 * @milliseconds: duration to render
 * @buffer: destination buffer
 * @capacity: number of bytes available in @buffer
 */
APOSAUI_DEF void aposaui_format_duration(uint32_t milliseconds, char *buffer, size_t capacity);

/**
 * aposaui_format_dimensions - render an image size as "W x H"
 * @width: image width in pixels
 * @height: image height in pixels
 * @buffer: destination buffer
 * @capacity: number of bytes available in @buffer
 */
APOSAUI_DEF void aposaui_format_dimensions(uint32_t width, uint32_t height, char *buffer, size_t capacity);

/**
 * aposaui_format_percent - render a zoom factor as a percentage
 * @percent: zoom percentage, for example 100.0 for actual size
 * @buffer: destination buffer
 * @capacity: number of bytes available in @buffer
 */
APOSAUI_DEF void aposaui_format_percent(double percent, char *buffer, size_t capacity);

/* -------------------------------------------------------------------------
 * Portable: hit table and interaction state
 *
 * The table is filled while painting, not while laying out, so a widget's
 * geometry is computed once and cannot drift between the rectangle that is
 * drawn and the rectangle that is clicked.
 *
 * Layers arbitrate overlap: a menu is registered above the toolbar it covers,
 * and the settings panel above the menu. The greatest layer wins, and a later
 * entry wins a tie, so the caller controls precedence by paint order.
 * ---------------------------------------------------------------------- */

typedef int32_t AposaUiId;

#define APOSAUI_ID_NONE ((AposaUiId)0)

#define APOSAUI_LAYER_CHROME 0u
#define APOSAUI_LAYER_MENU 1u
#define APOSAUI_LAYER_PANEL 2u
#define APOSAUI_LAYER_OVERLAY 3u

typedef struct AposaUiHitEntry {
	AposaUiId id;
	AposaUiRect rect;
	uint32_t layer;
	bool enabled;
} AposaUiHitEntry;

typedef struct AposaUiHitTable {
	AposaUiHitEntry *entries;
	uint32_t count;
	uint32_t capacity;
} AposaUiHitTable;

/**
 * aposaui_hit_table_init - bind a table to caller-owned storage
 * @table: table to initialize
 * @storage: array the table records entries in
 * @capacity: number of entries @storage can hold
 *
 * The library never allocates the storage.
 */
APOSAUI_DEF void aposaui_hit_table_init(AposaUiHitTable *table, AposaUiHitEntry *storage, uint32_t capacity);

/**
 * aposaui_hit_table_reset - drop every entry, keeping the storage
 * @table: table to clear
 */
APOSAUI_DEF void aposaui_hit_table_reset(AposaUiHitTable *table);

/**
 * aposaui_hit_table_add - record one interactive rectangle
 * @table: table to append to
 * @id: identifier reported when the rectangle is hit
 * @rect: rectangle in canvas pixels
 * @layer: precedence against overlapping entries
 * @enabled: whether the entry can be reported at all
 *
 * Return: 0 on success, or a negative value when the table is full.
 */
APOSAUI_DEF int aposaui_hit_table_add(AposaUiHitTable *table, AposaUiId id, AposaUiRect rect, uint32_t layer,
				      bool enabled);

/**
 * aposaui_hit_table_test - find the identifier under a point
 * @table: table to search
 * @x: point x in canvas pixels
 * @y: point y in canvas pixels
 *
 * Return: the enabled entry with the greatest layer containing the point, or
 * APOSAUI_ID_NONE.
 */
APOSAUI_DEF AposaUiId aposaui_hit_table_test(const AposaUiHitTable *table, int32_t x, int32_t y);

/**
 * aposaui_hit_table_find - look up the entry recorded for an identifier
 * @table: table to search
 * @id: identifier to find
 *
 * Return: borrowed entry, or NULL when @id is absent.
 */
APOSAUI_DEF const AposaUiHitEntry *aposaui_hit_table_find(const AposaUiHitTable *table, AposaUiId id);

/**
 * aposaui_hit_table_set_enabled - enable or disable a recorded identifier
 * @table: table to update
 * @id: identifier to update
 * @enabled: new enabled state
 *
 * Return: true when @id was found and updated.
 */
APOSAUI_DEF bool aposaui_hit_table_set_enabled(AposaUiHitTable *table, AposaUiId id, bool enabled);

/*
 * The interaction state is caller-owned and outlives a frame. Press and release
 * mirror the capture pairing a window needs: press only captures when it landed
 * on something, and release only activates when the pointer came back up on the
 * same identifier it went down on.
 */
typedef struct AposaUiInteraction {
	AposaUiId hovered;
	AposaUiId pressed;
	AposaUiId focused;
	AposaUiId active;
	int32_t drag_origin_x;
	int32_t drag_origin_y;
	int32_t drag_offset_x;
	int32_t drag_offset_y;
	bool capturing;
} AposaUiInteraction;

/**
 * aposaui_interaction_init - reset an interaction state to idle
 * @interaction: state to initialize
 */
APOSAUI_DEF void aposaui_interaction_init(AposaUiInteraction *interaction);

/**
 * aposaui_interaction_hover - record the identifier under the pointer
 * @interaction: state to update
 * @id: identifier under the pointer, or APOSAUI_ID_NONE
 */
APOSAUI_DEF void aposaui_interaction_hover(AposaUiInteraction *interaction, AposaUiId id);

/**
 * aposaui_interaction_press - begin a press on an identifier
 * @interaction: state to update
 * @id: identifier that was pressed, or APOSAUI_ID_NONE
 * @x: pointer x in canvas pixels
 * @y: pointer y in canvas pixels
 *
 * Return: true when the caller should take pointer capture.
 */
APOSAUI_DEF bool aposaui_interaction_press(AposaUiInteraction *interaction, AposaUiId id, int32_t x, int32_t y);

/**
 * aposaui_interaction_release - finish a press and report the activation
 * @interaction: state to update
 * @hit: identifier under the pointer at release, or APOSAUI_ID_NONE
 *
 * Return: the identifier to activate, which is the pressed identifier only when
 * @hit matches it, or APOSAUI_ID_NONE.
 */
APOSAUI_DEF AposaUiId aposaui_interaction_release(AposaUiInteraction *interaction, AposaUiId hit);

/**
 * aposaui_interaction_cancel - abandon any press or drag in progress
 * @interaction: state to clear
 */
APOSAUI_DEF void aposaui_interaction_cancel(AposaUiInteraction *interaction);

/**
 * aposaui_interaction_is_hovered - test the hovered identifier
 * @interaction: state to inspect
 * @id: identifier to compare
 *
 * Return: true when @id is hovered.
 */
APOSAUI_DEF bool aposaui_interaction_is_hovered(const AposaUiInteraction *interaction, AposaUiId id);

/**
 * aposaui_interaction_is_pressed - test the pressed identifier
 * @interaction: state to inspect
 * @id: identifier to compare
 *
 * Return: true when @id is pressed.
 */
APOSAUI_DEF bool aposaui_interaction_is_pressed(const AposaUiInteraction *interaction, AposaUiId id);

/* -------------------------------------------------------------------------
 * Input events
 *
 * The event itself is portable data; only translating a platform message into
 * one is not. Callers that care about gestures (zoom, pan, playback) read these
 * fields instead of decoding message parameters themselves.
 * ---------------------------------------------------------------------- */

typedef enum AposaUiEventType {
	APOSAUI_EVENT_NONE = -1,
	APOSAUI_EVENT_POINTER_MOVE,
	APOSAUI_EVENT_POINTER_DOWN,
	APOSAUI_EVENT_POINTER_UP,
	APOSAUI_EVENT_POINTER_LEAVE,
	APOSAUI_EVENT_WHEEL,
	APOSAUI_EVENT_KEY,
	APOSAUI_EVENT_RESIZE,
	APOSAUI_EVENT_TIMER,
	APOSAUI_EVENT_FILES_DROPPED,
	APOSAUI_EVENT_CLOSE,
} AposaUiEventType;

/*
 * Modifier bits. These values are wire-compatible with the Win32 virtual-key
 * modifier encoding the viewer's keybinding table already uses, so a binding
 * table written against one works against the other.
 */
#define APOSAUI_MOD_CONTROL 1u
#define APOSAUI_MOD_SHIFT 2u
#define APOSAUI_MOD_ALT 4u

typedef struct AposaUiEvent {
	AposaUiEventType type;
	int32_t x;               /* pointer position in client pixels */
	int32_t y;
	double wheel_notches;    /* signed wheel distance, in notches of 120 units */
	uint32_t virtual_key;    /* raw keyboard code, untranslated */
	uint32_t modifiers;      /* APOSAUI_MOD_* */
	bool is_repeat;          /* keyboard auto-repeat rather than a fresh press */
	uint32_t window_width;   /* client size, for APOSAUI_EVENT_RESIZE */
	uint32_t window_height;
	uint32_t timer_id;
} AposaUiEvent;

#ifndef APOSAUI_NO_PLATFORM

/**
 * aposaui_event_from_message - translate a Win32 message into an event
 * @window: window the message was delivered to, needed to unproject the wheel
 * @message: message identifier, for example WM_MOUSEMOVE
 * @w_param: message wParam
 * @l_param: message lParam
 * @out_event: event written for every message, possibly APOSAUI_EVENT_NONE
 *
 * Return: true when @message produced a meaningful event, false when the caller
 * should fall through to the default window procedure.
 */
APOSAUI_DEF bool aposaui_event_from_message(void *window, uint32_t message, uint64_t w_param, int64_t l_param,
					    AposaUiEvent *out_event);

/**
 * aposaui_modifiers_from_keyboard - sample the current modifier key state
 *
 * Return: the APOSAUI_MOD_* bits currently held down.
 */
APOSAUI_DEF uint32_t aposaui_modifiers_from_keyboard(void);

/* -------------------------------------------------------------------------
 * Win32 host
 *
 * Owns the things that are the same in every Win32 window: the window class,
 * the fonts, the device-independent bitmap bookkeeping and the blit. It does
 * not own the message loop, the window procedure or the paint policy, because
 * those are where applications differ.
 * ---------------------------------------------------------------------- */

typedef struct AposaUiHost AposaUiHost;

typedef struct AposaUiHostDesc {
	const char *title;                 /* UTF-8 window title, converted internally */
	const char *primary_font_family;   /* UTF-8, e.g. "Segoe UI" */
	const char *numeric_font_family;   /* UTF-8, e.g. "Consolas" */
	uint32_t primary_font_size;
	uint32_t numeric_font_size;
	int32_t client_width;              /* initial client size before DPI adjustment */
	int32_t client_height;
	WNDPROC window_proc;               /* receives the new HWND, as any Win32 window proc */
	void *user;                        /* handed to the window proc as lpCreateParams */
} AposaUiHostDesc;

/**
 * aposaui_host_create - register the window class and create the window
 * @desc: window description and fonts to open
 * @allocator: allocator owning the host and its fonts, copied by value
 * @out_host: receives the host on success
 *
 * The window starts hidden, so a caller that needs to set up a rendering
 * backend first can do so before the first frame is shown. The class is
 * registered once per process and reused after that.
 *
 * Return: 0 on success, or a negative value on failure.
 */
APOSAUI_DEF int aposaui_host_create(const AposaUiHostDesc *desc, const AposaUiAllocator *allocator,
				    AposaUiHost **out_host);

/**
 * aposaui_host_destroy - release a host, its fonts and its window
 * @host: host to destroy, may be NULL
 */
APOSAUI_DEF void aposaui_host_destroy(AposaUiHost *host);

/**
 * aposaui_host_window - access the window the host created
 * @host: host to inspect
 *
 * Return: the HWND, or NULL when @host is NULL.
 */
APOSAUI_DEF void *aposaui_host_window(const AposaUiHost *host);

/**
 * aposaui_host_dpi - display density the host was created against
 * @host: host to inspect
 *
 * Return: dots per inch, never zero.
 */
APOSAUI_DEF uint32_t aposaui_host_dpi(const AposaUiHost *host);

/**
 * aposaui_host_system_dpi - display density of the primary monitor
 *
 * Return: dots per inch, or the base density when it cannot be queried.
 */
APOSAUI_DEF uint32_t aposaui_host_system_dpi(void);

/**
 * aposaui_host_apply_dark_title_bar - match the title bar to a dark interface
 * @host: host whose window is updated
 * @enabled: whether the title bar should be dark
 */
APOSAUI_DEF void aposaui_host_apply_dark_title_bar(AposaUiHost *host, bool enabled);

/**
 * aposaui_host_primary_font - font intended for labels
 * @host: host to inspect
 *
 * Return: borrowed font, or NULL when @host is NULL.
 */
APOSAUI_DEF AposaUiFont *aposaui_host_primary_font(const AposaUiHost *host);

/**
 * aposaui_host_numeric_font - font intended for numeric readouts
 * @host: host to inspect
 *
 * Return: borrowed font, or NULL when @host is NULL.
 */
APOSAUI_DEF AposaUiFont *aposaui_host_numeric_font(const AposaUiHost *host);

/**
 * aposaui_host_present - blit a canvas to a device context
 * @host: host owning the bitmap bookkeeping
 * @device_context: destination, typically the HDC from BeginPaint
 * @canvas: canvas to present
 *
 * The caller keeps control of when painting happens, which is why the device
 * context is passed in rather than acquired here.
 */
APOSAUI_DEF void aposaui_host_present(AposaUiHost *host, void *device_context, const AposaUiCanvas *canvas);

#endif /* APOSAUI_NO_PLATFORM */

/* -------------------------------------------------------------------------
 * Portable: widgets
 *
 * Widgets draw straight onto the canvas and register their own hit rectangles
 * as they go, so the rectangle that is painted is the rectangle that is
 * clickable. Geometry is passed in on every call and the library keeps no
 * widget objects, which is what keeps hit testing and painting from drifting
 * apart.
 * ---------------------------------------------------------------------- */

typedef enum AposaUiIcon {
	APOSAUI_ICON_ZOOM_IN,
	APOSAUI_ICON_ZOOM_OUT,
	APOSAUI_ICON_FIT,
	APOSAUI_ICON_CHEVRON_DOWN,
	APOSAUI_ICON_NAVIGATE_LEFT,
	APOSAUI_ICON_NAVIGATE_RIGHT,
	APOSAUI_ICON_PLAY,
	APOSAUI_ICON_PLAY_REVERSE,
	APOSAUI_ICON_PAUSE,
} AposaUiIcon;

/**
 * aposaui_icon_paint - draw a vector icon centred in a box
 * @canvas: destination canvas
 * @icon: icon to draw
 * @box: box the icon is centred in and sized from
 * @metrics: scaled metrics, which set the stroke width
 * @color: 0xAARRGGBB colour
 */
APOSAUI_DEF void aposaui_icon_paint(AposaUiCanvas *canvas, AposaUiIcon icon, AposaUiRect box,
				    const AposaUiMetrics *metrics, AposaUiColor color);

/**
 * aposaui_control_background - paint a control's hover, press or selection state
 * @canvas: destination canvas
 * @rect: control rectangle
 * @radius: corner radius in pixels
 * @selected: whether the control is the current selection
 * @is_hovered: whether the pointer is over the control
 * @is_pressed: whether the control is held down
 * @colors: active theme colours
 *
 * Only the background is drawn; the caller draws the label or icon over it.
 *
 * Return: the stroke colour the control should outline itself with.
 */
APOSAUI_DEF AposaUiColor aposaui_control_background(AposaUiCanvas *canvas, AposaUiRect rect, double radius,
						    bool selected, bool is_hovered, bool is_pressed,
						    const AposaUiThemeColors *colors);

/**
 * aposaui_text_centered - draw a string horizontally centred in a box
 * @canvas: destination canvas
 * @font: font to draw with
 * @rect: box the text is centred in
 * @text: NUL-terminated UTF-8 string
 * @color: 0xAARRGGBB colour
 */
APOSAUI_DEF void aposaui_text_centered(AposaUiCanvas *canvas, AposaUiFont *font, AposaUiRect rect, const char *text,
				       AposaUiColor color);

/**
 * aposaui_badge_measure - width of a badge holding a string
 * @text: NUL-terminated UTF-8 string
 * @metrics: scaled metrics, which set the horizontal padding
 * @font: font the text is measured with
 *
 * Return: the badge width in pixels.
 */
APOSAUI_DEF int32_t aposaui_badge_measure(const char *text, const AposaUiMetrics *metrics, AposaUiFont *font);

/**
 * aposaui_badge_paint - draw a pill holding a string, aligned to a right edge
 * @canvas: destination canvas
 * @text: NUL-terminated UTF-8 string
 * @right_edge: x the badge's right side is placed against
 * @bar: bar the badge is vertically centred in, and whose baseline it uses
 * @metrics: scaled metrics
 * @font: font to draw with
 * @colors: active theme colours
 *
 * Return: the x of the badge's left edge, so callers can stack another element
 * to its left.
 */
APOSAUI_DEF int32_t aposaui_badge_paint(AposaUiCanvas *canvas, const char *text, int32_t right_edge, AposaUiRect bar,
					const AposaUiMetrics *metrics, AposaUiFont *font,
					const AposaUiThemeColors *colors);

/**
 * aposaui_menu_panel_paint - draw a dropdown panel behind its rows
 * @canvas: destination canvas
 * @menu: panel rectangle
 * @colors: active theme colours
 */
APOSAUI_DEF void aposaui_menu_panel_paint(AposaUiCanvas *canvas, AposaUiRect menu, const AposaUiThemeColors *colors);

/**
 * aposaui_menu_item_paint - draw one dropdown row
 * @canvas: destination canvas
 * @menu: panel rectangle the rows divide evenly
 * @row: zero-based row index
 * @label: left-aligned UTF-8 label
 * @shortcut: right-aligned UTF-8 shortcut hint, or NULL
 * @row_count: number of rows in the panel
 * @metrics: scaled metrics
 * @font: font to draw with
 * @colors: active theme colours
 */
APOSAUI_DEF void aposaui_menu_item_paint(AposaUiCanvas *canvas, AposaUiRect menu, int32_t row, const char *label,
					 const char *shortcut, int32_t row_count, const AposaUiMetrics *metrics,
					 AposaUiFont *font, const AposaUiThemeColors *colors);

/**
 * aposaui_scrollbar_paint - draw a scrollbar track and thumb
 * @canvas: destination canvas
 * @track: track rectangle
 * @thumb: thumb rectangle, positioned by the caller
 * @is_hovered: whether the pointer is over the thumb
 * @is_pressed: whether the thumb is being dragged
 * @colors: active theme colours
 *
 * Both rectangles are rounded by half their width, so a zero-width track draws
 * nothing.
 */
APOSAUI_DEF void aposaui_scrollbar_paint(AposaUiCanvas *canvas, AposaUiRect track, AposaUiRect thumb, bool is_hovered,
					 bool is_pressed, const AposaUiThemeColors *colors);

/*
 * A list is driven by a callback rather than an array so the caller builds each
 * row only when it is about to be drawn. That is what lets a list of thousands
 * of rows cost the same as a list of ten.
 */
typedef struct AposaUiListRow {
	const char *label;
	const char *value;
} AposaUiListRow;

typedef struct AposaUiListModel {
	void *user;
	uint32_t row_count;
	/*
	 * Fill @out_row for @index and return true, or return false to skip the row
	 * without consuming vertical space. The strings are borrowed and need only
	 * survive the call.
	 */
	bool (*get_row)(void *user, uint32_t index, AposaUiListRow *out_row);
} AposaUiListModel;

/**
 * aposaui_list_paint - draw the visible rows of a virtualized list
 * @canvas: destination canvas
 * @list: list rectangle
 * @row_height: height of one row in pixels
 * @model: row source, NULL or empty to draw @empty_text instead
 * @scroll_offset: index of the first visible row
 * @visible_count: maximum number of rows to draw
 * @font: font to draw with
 * @colors: active theme colours
 * @empty_text: UTF-8 text drawn when the model has no rows, or NULL for nothing
 *
 * The caller paints the list background and any scrollbar; this draws the rows
 * over them, left-aligned labels and right-aligned value badges.
 */
APOSAUI_DEF void aposaui_list_paint(AposaUiCanvas *canvas, AposaUiRect list, int32_t row_height,
				    const AposaUiListModel *model, uint32_t scroll_offset, uint32_t visible_count,
				    AposaUiFont *font, const AposaUiThemeColors *colors, const char *empty_text);

/*
 * A timeline is a rail with a gutter at each end for a label, so the rail has to
 * be placed before the labels are measured. Splitting the geometry out lets a
 * caller hit test the rail without rebuilding it.
 */
typedef struct AposaUiTimeline {
	AposaUiRect bar;      /* the whole control, including the label gutters */
	AposaUiRect track;    /* the rail the progress runs along */
	int32_t leading_x;    /* where the leading label starts */
	int32_t trailing_x;   /* where the trailing label starts */
} AposaUiTimeline;

/**
 * aposaui_timeline_layout - place the rail and label positions of a timeline
 * @bar: the whole control rectangle, including the label gutters
 * @label_width: width reserved for each end label
 * @label_gap: space between a label and the rail
 *
 * Return: the timeline geometry within @bar.
 */
APOSAUI_DEF AposaUiTimeline aposaui_timeline_layout(AposaUiRect bar, int32_t label_width, int32_t label_gap);

/**
 * aposaui_timeline_paint - draw a timeline with its leading and trailing labels
 * @canvas: destination canvas
 * @timeline: geometry from aposaui_timeline_layout()
 * @progress: position of the thumb, clamped to 0.0-1.0
 * @font: font for the end labels
 * @colors: active theme colours
 * @leading_text: label at the start, typically the elapsed time
 * @trailing_text: label at the end, typically the total duration
 *
 * The rail is drawn in the widget's own neutral tones; only the filled portion
 * and the thumb take the theme accent.
 */
APOSAUI_DEF void aposaui_timeline_paint(AposaUiCanvas *canvas, const AposaUiTimeline *timeline, double progress,
					AposaUiFont *font, const AposaUiThemeColors *colors, const char *leading_text,
					const char *trailing_text);

/**
 * aposaui_timeline_progress_at - convert a pointer x on a timeline to progress
 * @timeline: geometry from aposaui_timeline_layout()
 * @x: pointer x in canvas pixels
 *
 * Return: progress in 0.0-1.0, clamped at both ends.
 */
APOSAUI_DEF double aposaui_timeline_progress_at(const AposaUiTimeline *timeline, int32_t x);

#ifdef __cplusplus
}
#endif

#endif /* APOSAUI_H */

#ifdef APOSAUI_IMPLEMENTATION
#ifndef APOSAUI_NO_PLATFORM
#include <shellapi.h>
#include <windowsx.h>
#endif

#include <math.h>
#include <stdio.h>
#include <string.h>

/* -------------------------------------------------------------------------
 * Portable implementation
 * ---------------------------------------------------------------------- */

APOSAUI_DEF bool aposaui_rect_contains(AposaUiRect rect, int32_t x, int32_t y) {
	return x >= rect.x && y >= rect.y && x < rect.x + rect.width && y < rect.y + rect.height;
}

APOSAUI_DEF AposaUiRect aposaui_rect_inset(AposaUiRect rect, int32_t amount) {
	return (AposaUiRect){.x = rect.x + amount,
			     .y = rect.y + amount,
			     .width = rect.width - amount * 2,
			     .height = rect.height - amount * 2};
}

static uint32_t aposaui__blend_channel(uint32_t source, uint32_t destination, uint32_t alpha) {
	return (source * alpha + destination * (255u - alpha) + 127u) / 255u;
}

static uint32_t aposaui__blend_color(uint32_t source, uint32_t destination, uint32_t alpha) {
	const uint32_t red = aposaui__blend_channel((source >> 16u) & 0xffu, (destination >> 16u) & 0xffu, alpha);
	const uint32_t green = aposaui__blend_channel((source >> 8u) & 0xffu, (destination >> 8u) & 0xffu, alpha);
	const uint32_t blue = aposaui__blend_channel(source & 0xffu, destination & 0xffu, alpha);

	return 0xff000000u | (red << 16u) | (green << 8u) | blue;
}

static bool aposaui__clip_rect(const AposaUiCanvas *canvas, AposaUiRect rect, AposaUiRect *out_clipped) {
	int64_t left = rect.x;
	int64_t top = rect.y;
	int64_t right = (int64_t)rect.x + rect.width;
	int64_t bottom = (int64_t)rect.y + rect.height;

	if (canvas == NULL || canvas->pixels == NULL || rect.width <= 0 || rect.height <= 0) {
		return false;
	}
	if (left < 0) {
		left = 0;
	}
	if (top < 0) {
		top = 0;
	}
	if (right > canvas->width) {
		right = canvas->width;
	}
	if (bottom > canvas->height) {
		bottom = canvas->height;
	}
	if (right <= left || bottom <= top) {
		return false;
	}
	*out_clipped = (AposaUiRect){.x = (int32_t)left,
				     .y = (int32_t)top,
				     .width = (int32_t)(right - left),
				     .height = (int32_t)(bottom - top)};
	return true;
}

static uint32_t *aposaui__row(AposaUiCanvas *canvas, int32_t y) {
	return (uint32_t *)(void *)(canvas->pixels + (size_t)y * canvas->stride);
}

static double aposaui__coverage_from_distance(double distance) {
	const double coverage = 0.5 - distance;

	if (coverage <= 0.0) {
		return 0.0;
	}
	if (coverage >= 1.0) {
		return 1.0;
	}
	return coverage;
}

APOSAUI_DEF void aposaui_paint_fill_rect(AposaUiCanvas *canvas, AposaUiRect rect, AposaUiColor color) {
	AposaUiRect clipped;

	if (!aposaui__clip_rect(canvas, rect, &clipped)) {
		return;
	}
	const uint32_t opaque = 0xff000000u | (color & 0x00ffffffu);

	for (int32_t y = clipped.y; y < clipped.y + clipped.height; ++y) {
		uint32_t *row = aposaui__row(canvas, y);

		for (int32_t x = clipped.x; x < clipped.x + clipped.width; ++x) {
			row[x] = opaque;
		}
	}
}

APOSAUI_DEF void aposaui_paint_blend_rect(AposaUiCanvas *canvas, AposaUiRect rect, AposaUiColor color) {
	const uint32_t alpha = (color >> 24u) & 0xffu;
	AposaUiRect clipped;

	if (alpha == 0u) {
		return;
	}
	if (alpha == 255u) {
		aposaui_paint_fill_rect(canvas, rect, color);
		return;
	}
	if (!aposaui__clip_rect(canvas, rect, &clipped)) {
		return;
	}
	for (int32_t y = clipped.y; y < clipped.y + clipped.height; ++y) {
		uint32_t *row = aposaui__row(canvas, y);

		for (int32_t x = clipped.x; x < clipped.x + clipped.width; ++x) {
			row[x] = aposaui__blend_color(color, row[x], alpha);
		}
	}
}

APOSAUI_DEF void aposaui_paint_blend_pixel(AposaUiCanvas *canvas, int32_t x, int32_t y, AposaUiColor color,
					   uint32_t coverage) {
	const uint32_t alpha = ((color >> 24u) & 0xffu) * (coverage > 255u ? 255u : coverage) / 255u;
	uint32_t *row;

	if (canvas == NULL || canvas->pixels == NULL || alpha == 0u || x < 0 || y < 0 || (uint32_t)x >= canvas->width ||
	    (uint32_t)y >= canvas->height) {
		return;
	}
	row = aposaui__row(canvas, y);
	row[x] = aposaui__blend_color(color, row[x], alpha);
}

APOSAUI_DEF void aposaui_paint_rounded_rect(AposaUiCanvas *canvas, AposaUiRect rect, double radius,
					    AposaUiColor color) {
	const uint32_t alpha = (color >> 24u) & 0xffu;
	const double half_width = (double)rect.width * 0.5;
	const double half_height = (double)rect.height * 0.5;
	const double center_x = (double)rect.x + half_width;
	const double center_y = (double)rect.y + half_height;
	AposaUiRect clipped;

	if (alpha == 0u || rect.width <= 0 || rect.height <= 0) {
		return;
	}
	if (radius > half_width) {
		radius = half_width;
	}
	if (radius > half_height) {
		radius = half_height;
	}
	if (radius <= 0.0) {
		aposaui_paint_blend_rect(canvas, rect, color);
		return;
	}
	if (!aposaui__clip_rect(canvas, rect, &clipped)) {
		return;
	}
	for (int32_t y = clipped.y; y < clipped.y + clipped.height; ++y) {
		const double offset_y = fabs((double)y + 0.5 - center_y) - (half_height - radius);

		for (int32_t x = clipped.x; x < clipped.x + clipped.width; ++x) {
			const double offset_x = fabs((double)x + 0.5 - center_x) - (half_width - radius);
			const double inside = fmin(fmax(offset_x, offset_y), 0.0);
			const double corner = hypot(fmax(offset_x, 0.0), fmax(offset_y, 0.0));
			const double coverage = aposaui__coverage_from_distance(inside + corner - radius);

			aposaui_paint_blend_pixel(canvas, x, y, color, (uint32_t)(coverage * 255.0 + 0.5));
		}
	}
}

APOSAUI_DEF void aposaui_paint_disc(AposaUiCanvas *canvas, double center_x, double center_y, double radius,
				    AposaUiColor color) {
	const AposaUiRect bounds = {.x = (int32_t)floor(center_x - radius - 1.0),
				    .y = (int32_t)floor(center_y - radius - 1.0),
				    .width = (int32_t)ceil(radius * 2.0 + 3.0),
				    .height = (int32_t)ceil(radius * 2.0 + 3.0)};
	AposaUiRect clipped;

	if (radius <= 0.0 || ((color >> 24u) & 0xffu) == 0u || !aposaui__clip_rect(canvas, bounds, &clipped)) {
		return;
	}
	for (int32_t y = clipped.y; y < clipped.y + clipped.height; ++y) {
		const double delta_y = (double)y + 0.5 - center_y;

		for (int32_t x = clipped.x; x < clipped.x + clipped.width; ++x) {
			const double delta_x = (double)x + 0.5 - center_x;
			const double coverage = aposaui__coverage_from_distance(hypot(delta_x, delta_y) - radius);

			aposaui_paint_blend_pixel(canvas, x, y, color, (uint32_t)(coverage * 255.0 + 0.5));
		}
	}
}

APOSAUI_DEF void aposaui_paint_ring(AposaUiCanvas *canvas, double center_x, double center_y, double radius,
				    double thickness, AposaUiColor color) {
	const double half_thickness = thickness * 0.5;
	const double outer = radius + half_thickness;
	const AposaUiRect bounds = {.x = (int32_t)floor(center_x - outer - 1.0),
				    .y = (int32_t)floor(center_y - outer - 1.0),
				    .width = (int32_t)ceil(outer * 2.0 + 3.0),
				    .height = (int32_t)ceil(outer * 2.0 + 3.0)};
	AposaUiRect clipped;

	if (radius <= 0.0 || thickness <= 0.0 || ((color >> 24u) & 0xffu) == 0u ||
	    !aposaui__clip_rect(canvas, bounds, &clipped)) {
		return;
	}
	for (int32_t y = clipped.y; y < clipped.y + clipped.height; ++y) {
		const double delta_y = (double)y + 0.5 - center_y;

		for (int32_t x = clipped.x; x < clipped.x + clipped.width; ++x) {
			const double delta_x = (double)x + 0.5 - center_x;
			const double distance = fabs(hypot(delta_x, delta_y) - radius) - half_thickness;
			const double coverage = aposaui__coverage_from_distance(distance);

			aposaui_paint_blend_pixel(canvas, x, y, color, (uint32_t)(coverage * 255.0 + 0.5));
		}
	}
}

APOSAUI_DEF void aposaui_paint_line(AposaUiCanvas *canvas, double x0, double y0, double x1, double y1,
				    double thickness, AposaUiColor color) {
	const double half_thickness = thickness * 0.5;
	const double min_x = fmin(x0, x1) - half_thickness - 1.0;
	const double min_y = fmin(y0, y1) - half_thickness - 1.0;
	const double max_x = fmax(x0, x1) + half_thickness + 1.0;
	const double max_y = fmax(y0, y1) + half_thickness + 1.0;
	const AposaUiRect bounds = {.x = (int32_t)floor(min_x),
				    .y = (int32_t)floor(min_y),
				    .width = (int32_t)ceil(max_x - min_x) + 1,
				    .height = (int32_t)ceil(max_y - min_y) + 1};
	const double delta_x = x1 - x0;
	const double delta_y = y1 - y0;
	const double length_squared = delta_x * delta_x + delta_y * delta_y;
	AposaUiRect clipped;

	if (thickness <= 0.0 || length_squared <= 0.0 || ((color >> 24u) & 0xffu) == 0u ||
	    !aposaui__clip_rect(canvas, bounds, &clipped)) {
		return;
	}
	for (int32_t y = clipped.y; y < clipped.y + clipped.height; ++y) {
		const double sample_y = (double)y + 0.5;

		for (int32_t x = clipped.x; x < clipped.x + clipped.width; ++x) {
			const double sample_x = (double)x + 0.5;
			double t = ((sample_x - x0) * delta_x + (sample_y - y0) * delta_y) / length_squared;
			double nearest_x;
			double nearest_y;
			double coverage;

			if (t < 0.0) {
				t = 0.0;
			}
			if (t > 1.0) {
				t = 1.0;
			}
			nearest_x = x0 + t * delta_x;
			nearest_y = y0 + t * delta_y;
			coverage = aposaui__coverage_from_distance(hypot(sample_x - nearest_x, sample_y - nearest_y) -
								   half_thickness);
			aposaui_paint_blend_pixel(canvas, x, y, color, (uint32_t)(coverage * 255.0 + 0.5));
		}
	}
}

static uint32_t aposaui__scale_for_dpi(uint32_t value, uint32_t dpi) {
	return (uint32_t)(((uint64_t)value * dpi + APOSAUI_BASE_DPI / 2u) / APOSAUI_BASE_DPI);
}

APOSAUI_DEF AposaUiMetrics aposaui_metrics_for_dpi(uint32_t dpi) {
	const uint32_t effective_dpi = dpi == 0u ? APOSAUI_BASE_DPI : dpi;
	AposaUiMetrics metrics;

	metrics.top_bar_height = aposaui__scale_for_dpi(34u, effective_dpi);
	metrics.bottom_bar_height = aposaui__scale_for_dpi(28u, effective_dpi);
	metrics.separator_height = aposaui__scale_for_dpi(1u, effective_dpi);
	metrics.padding_x = aposaui__scale_for_dpi(12u, effective_dpi);
	metrics.gap = aposaui__scale_for_dpi(10u, effective_dpi);
	metrics.primary_font_size = aposaui__scale_for_dpi(13u, effective_dpi);
	metrics.secondary_font_size = aposaui__scale_for_dpi(12u, effective_dpi);
	metrics.button_size = aposaui__scale_for_dpi(20u, effective_dpi);
	metrics.button_gap = aposaui__scale_for_dpi(6u, effective_dpi);
	metrics.status_dot_radius = aposaui__scale_for_dpi(3u, effective_dpi);
	metrics.badge_padding_x = aposaui__scale_for_dpi(6u, effective_dpi);
	metrics.badge_height = aposaui__scale_for_dpi(16u, effective_dpi);
	if (metrics.separator_height == 0u) {
		metrics.separator_height = 1u;
	}
	return metrics;
}

APOSAUI_DEF int32_t aposaui_stroke_width(const AposaUiMetrics *metrics) {
	const int32_t stroke = metrics == NULL ? 1 : (int32_t)(metrics->button_size / 12u);

	return stroke < 1 ? 1 : stroke;
}

APOSAUI_DEF AposaUiThemeColors aposaui_theme_colors(AposaUiThemeMode mode) {
	if (mode == APOSAUI_THEME_MODE_LIGHT) {
		return (AposaUiThemeColors){.canvas = 0xfff2f2f2u,
					    .bar = 0xfffafafau,
					    .separator = 0xffd6d6d6u,
					    .text_primary = 0xff202020u,
					    .text_secondary = 0xff606060u,
					    .text_dim = 0xff8a8a8au,
					    .status_dot = 0xff2f8f65u,
					    .badge_background = 0xffe5e5e5u,
					    .badge_text = 0xff505050u,
					    .control_stroke = 0xff6a6a6au,
					    .control_stroke_active = 0xff202020u,
					    .control_hover = 0x18000000u,
					    .control_pressed = 0x30000000u,
					    .panel = 0xffffffffu,
					    .panel_sidebar = 0xfff0f0f0u,
					    .panel_border = 0xffd0d0d0u,
					    .option_selected = 0xffe1ebfbu,
					    .option_accent = 0xff2d6cdfu};
	}
	return (AposaUiThemeColors){.canvas = APOSAUI_COLOR_CANVAS,
				    .bar = APOSAUI_COLOR_BAR,
				    .separator = APOSAUI_COLOR_SEPARATOR,
				    .text_primary = APOSAUI_COLOR_TEXT_PRIMARY,
				    .text_secondary = APOSAUI_COLOR_TEXT_SECONDARY,
				    .text_dim = APOSAUI_COLOR_TEXT_DIM,
				    .status_dot = APOSAUI_COLOR_STATUS_DOT,
				    .badge_background = APOSAUI_COLOR_BADGE_BACKGROUND,
				    .badge_text = APOSAUI_COLOR_BADGE_TEXT,
				    .control_stroke = APOSAUI_COLOR_CONTROL_STROKE,
				    .control_stroke_active = APOSAUI_COLOR_CONTROL_STROKE_ACTIVE,
				    .control_hover = APOSAUI_COLOR_CONTROL_HOVER,
				    .control_pressed = APOSAUI_COLOR_CONTROL_PRESSED,
				    .panel = 0xff242424u,
				    .panel_sidebar = 0xff1c1c1cu,
				    .panel_border = 0xff3a3a3au,
				    .option_selected = 0xff293b58u,
				    .option_accent = 0xff78a8ffu};
}

/*
 * The text renderer below is portable: it talks to the font interface only.
 * Filling that interface in is the backend's job, and the backend is platform
 * code, so a headless build is expected to supply its own aposaui_font_create,
 * aposaui_font_destroy, aposaui_font_ascent, aposaui_font_line_height and
 * aposaui_font_glyph. That is the extension point as much as it is a test
 * convenience.
 */

static const uint8_t APOSAUI__GAMMA_TABLE[256] = {
	0u, 6u, 9u, 12u, 15u, 17u, 19u, 21u, 23u, 25u, 27u, 29u, 31u, 33u, 34u, 36u, 38u, 39u, 41u, 43u, 44u, 46u, 47u,
	49u, 50u, 51u, 53u, 54u, 56u, 57u, 58u, 60u, 61u, 62u, 64u, 65u, 66u, 67u, 69u, 70u, 71u, 72u, 74u, 75u, 76u, 77u,
	78u, 79u, 81u, 82u, 83u, 84u, 85u, 86u, 87u, 89u, 90u, 91u, 92u, 93u, 94u, 95u, 96u, 97u, 98u, 99u, 100u, 101u,
	102u, 104u, 105u, 106u, 107u, 108u, 109u, 110u, 111u, 112u, 113u, 114u, 115u, 116u, 117u, 118u, 119u, 120u, 121u,
	121u, 122u, 123u, 124u, 125u, 126u, 127u, 128u, 129u, 130u, 131u, 132u, 133u, 134u, 135u, 136u, 136u, 137u, 138u,
	139u, 140u, 141u, 142u, 143u, 144u, 145u, 145u, 146u, 147u, 148u, 149u, 150u, 151u, 152u, 152u, 153u, 154u, 155u,
	156u, 157u, 158u, 159u, 159u, 160u, 161u, 162u, 163u, 164u, 164u, 165u, 166u, 167u, 168u, 169u, 169u, 170u, 171u,
	172u, 173u, 174u, 174u, 175u, 176u, 177u, 178u, 178u, 179u, 180u, 181u, 182u, 183u, 183u, 184u, 185u, 186u, 186u,
	187u, 188u, 189u, 190u, 190u, 191u, 192u, 193u, 194u, 194u, 195u, 196u, 197u, 197u, 198u, 199u, 200u, 201u, 201u,
	202u, 203u, 204u, 204u, 205u, 206u, 207u, 207u, 208u, 209u, 210u, 210u, 211u, 212u, 213u, 213u, 214u, 215u, 216u,
	216u, 217u, 218u, 219u, 219u, 220u, 221u, 222u, 222u, 223u, 224u, 225u, 225u, 226u, 227u, 227u, 228u, 229u, 230u,
	230u, 231u, 232u, 232u, 233u, 234u, 235u, 235u, 236u, 237u, 237u, 238u, 239u, 240u, 240u, 241u, 242u, 242u, 243u,
	244u, 245u, 245u, 246u, 247u, 247u, 248u, 249u, 249u, 250u, 251u, 252u, 252u, 253u, 254u, 254u, 255u};

#define APOSAUI__REPLACEMENT_CODEPOINT 0xfffdu

static uint32_t aposaui__decode_utf8(const char *text, size_t *out_length) {
	const uint8_t *bytes = (const uint8_t *)text;
	const uint8_t lead = bytes[0];
	uint32_t codepoint;
	size_t length;

	if (lead < 0x80u) {
		*out_length = 1u;
		return lead;
	}
	if ((lead & 0xe0u) == 0xc0u) {
		length = 2u;
		codepoint = lead & 0x1fu;
	} else if ((lead & 0xf0u) == 0xe0u) {
		length = 3u;
		codepoint = lead & 0x0fu;
	} else if ((lead & 0xf8u) == 0xf0u) {
		length = 4u;
		codepoint = lead & 0x07u;
	} else {
		*out_length = 1u;
		return APOSAUI__REPLACEMENT_CODEPOINT;
	}
	for (size_t index = 1u; index < length; ++index) {
		if ((bytes[index] & 0xc0u) != 0x80u) {
			*out_length = index;
			return APOSAUI__REPLACEMENT_CODEPOINT;
		}
		codepoint = (codepoint << 6u) | (bytes[index] & 0x3fu);
	}
	*out_length = length;
	return codepoint;
}

APOSAUI_DEF int32_t aposaui_font_measure_text(AposaUiFont *font, const char *text) {
	int32_t advance = 0;

	if (font == NULL || text == NULL) {
		return 0;
	}
	for (size_t offset = 0u; text[offset] != '\0';) {
		size_t length = 0u;
		const uint32_t codepoint = aposaui__decode_utf8(text + offset, &length);
		const AposaUiGlyph *glyph = aposaui_font_glyph(font, codepoint);

		offset += length;
		if (glyph != NULL) {
			advance += glyph->advance;
		}
	}
	return advance;
}

static void aposaui__blend_glyph(AposaUiCanvas *canvas, const AposaUiGlyph *glyph, int32_t pen_x, int32_t baseline_y,
				 AposaUiColor color) {
	const int32_t origin_x = pen_x + glyph->bearing_x;
	const int32_t origin_y = baseline_y - glyph->bearing_y;

	for (uint32_t row = 0u; row < glyph->height; ++row) {
		const uint8_t *source = glyph->coverage + (size_t)row * glyph->width;

		for (uint32_t column = 0u; column < glyph->width; ++column) {
			const uint8_t coverage = source[column];

			if (coverage != 0u) {
				aposaui_paint_blend_pixel(canvas, origin_x + (int32_t)column, origin_y + (int32_t)row,
							  color, APOSAUI__GAMMA_TABLE[coverage]);
			}
		}
	}
}

APOSAUI_DEF int32_t aposaui_font_draw_text(AposaUiCanvas *canvas, AposaUiFont *font, int32_t x, int32_t baseline_y,
					   const char *text, AposaUiColor color) {
	if (canvas == NULL || font == NULL || text == NULL) {
		return x;
	}
	for (size_t offset = 0u; text[offset] != '\0';) {
		size_t length = 0u;
		const uint32_t codepoint = aposaui__decode_utf8(text + offset, &length);
		const AposaUiGlyph *glyph = aposaui_font_glyph(font, codepoint);

		offset += length;
		if (glyph == NULL) {
			continue;
		}
		if (glyph->coverage != NULL && glyph->width != 0u && glyph->height != 0u) {
			aposaui__blend_glyph(canvas, glyph, x, baseline_y, color);
		}
		x += glyph->advance;
	}
	return x;
}

APOSAUI_DEF int32_t aposaui_text_baseline(AposaUiRect rect, const AposaUiFont *font) {
	const int32_t line_height = (int32_t)aposaui_font_line_height(font);

	return rect.y + (rect.height - line_height) / 2 + (int32_t)aposaui_font_ascent(font);
}

APOSAUI_DEF void aposaui_format_duration(uint32_t milliseconds, char *buffer, size_t capacity) {
	const uint32_t total_seconds = milliseconds / 1000u;
	const uint32_t minutes = total_seconds / 60u;
	const uint32_t seconds = total_seconds % 60u;

	if (buffer == NULL || capacity == 0u) {
		return;
	}
	(void)snprintf(buffer, capacity, "%u:%02u", minutes, seconds);
}

APOSAUI_DEF void aposaui_format_dimensions(uint32_t width, uint32_t height, char *buffer, size_t capacity) {
	if (buffer == NULL || capacity == 0u) {
		return;
	}
	(void)snprintf(buffer, capacity, "%u \xc3\x97 %u", width, height);
}

APOSAUI_DEF void aposaui_format_percent(double percent, char *buffer, size_t capacity) {
	if (buffer == NULL || capacity == 0u) {
		return;
	}
	(void)snprintf(buffer, capacity, "%.0f%%", percent);
}

APOSAUI_DEF void aposaui_hit_table_init(AposaUiHitTable *table, AposaUiHitEntry *storage, uint32_t capacity) {
	if (table == NULL) {
		return;
	}
	table->entries = storage;
	table->count = 0u;
	table->capacity = storage == NULL ? 0u : capacity;
}

APOSAUI_DEF void aposaui_hit_table_reset(AposaUiHitTable *table) {
	if (table != NULL) {
		table->count = 0u;
	}
}

APOSAUI_DEF int aposaui_hit_table_add(AposaUiHitTable *table, AposaUiId id, AposaUiRect rect, uint32_t layer,
				      bool enabled) {
	AposaUiHitEntry *entry;

	if (table == NULL || table->entries == NULL || table->count >= table->capacity) {
		return -1;
	}
	entry = &table->entries[table->count++];
	entry->id = id;
	entry->rect = rect;
	entry->layer = layer;
	entry->enabled = enabled;
	return 0;
}

APOSAUI_DEF AposaUiId aposaui_hit_table_test(const AposaUiHitTable *table, int32_t x, int32_t y) {
	AposaUiId best = APOSAUI_ID_NONE;
	uint32_t best_layer = 0u;

	if (table == NULL || table->entries == NULL) {
		return APOSAUI_ID_NONE;
	}
	for (uint32_t index = 0u; index < table->count; ++index) {
		const AposaUiHitEntry *entry = &table->entries[index];

		if (!entry->enabled || !aposaui_rect_contains(entry->rect, x, y)) {
			continue;
		}
		if (best == APOSAUI_ID_NONE || entry->layer >= best_layer) {
			best = entry->id;
			best_layer = entry->layer;
		}
	}
	return best;
}

APOSAUI_DEF const AposaUiHitEntry *aposaui_hit_table_find(const AposaUiHitTable *table, AposaUiId id) {
	if (table == NULL || table->entries == NULL) {
		return NULL;
	}
	for (uint32_t index = 0u; index < table->count; ++index) {
		if (table->entries[index].id == id) {
			return &table->entries[index];
		}
	}
	return NULL;
}

APOSAUI_DEF bool aposaui_hit_table_set_enabled(AposaUiHitTable *table, AposaUiId id, bool enabled) {
	if (table == NULL || table->entries == NULL) {
		return false;
	}
	for (uint32_t index = 0u; index < table->count; ++index) {
		if (table->entries[index].id == id) {
			table->entries[index].enabled = enabled;
			return true;
		}
	}
	return false;
}

APOSAUI_DEF void aposaui_interaction_init(AposaUiInteraction *interaction) {
	if (interaction == NULL) {
		return;
	}
	*interaction = (AposaUiInteraction){.hovered = APOSAUI_ID_NONE,
					    .pressed = APOSAUI_ID_NONE,
					    .focused = APOSAUI_ID_NONE,
					    .active = APOSAUI_ID_NONE};
}

APOSAUI_DEF void aposaui_interaction_hover(AposaUiInteraction *interaction, AposaUiId id) {
	if (interaction != NULL) {
		interaction->hovered = id;
	}
}

APOSAUI_DEF bool aposaui_interaction_press(AposaUiInteraction *interaction, AposaUiId id, int32_t x, int32_t y) {
	if (interaction == NULL || id == APOSAUI_ID_NONE) {
		return false;
	}
	interaction->pressed = id;
	interaction->active = id;
	interaction->drag_origin_x = x;
	interaction->drag_origin_y = y;
	interaction->drag_offset_x = 0;
	interaction->drag_offset_y = 0;
	interaction->capturing = true;
	return true;
}

APOSAUI_DEF AposaUiId aposaui_interaction_release(AposaUiInteraction *interaction, AposaUiId hit) {
	AposaUiId activated = APOSAUI_ID_NONE;

	if (interaction == NULL) {
		return APOSAUI_ID_NONE;
	}
	if (interaction->pressed != APOSAUI_ID_NONE && interaction->pressed == hit) {
		activated = interaction->pressed;
	}
	aposaui_interaction_cancel(interaction);
	return activated;
}

APOSAUI_DEF void aposaui_interaction_cancel(AposaUiInteraction *interaction) {
	if (interaction == NULL) {
		return;
	}
	interaction->pressed = APOSAUI_ID_NONE;
	interaction->active = APOSAUI_ID_NONE;
	interaction->capturing = false;
	interaction->drag_offset_x = 0;
	interaction->drag_offset_y = 0;
}

APOSAUI_DEF bool aposaui_interaction_is_hovered(const AposaUiInteraction *interaction, AposaUiId id) {
	return interaction != NULL && id != APOSAUI_ID_NONE && interaction->hovered == id;
}

APOSAUI_DEF bool aposaui_interaction_is_pressed(const AposaUiInteraction *interaction, AposaUiId id) {
	return interaction != NULL && id != APOSAUI_ID_NONE && interaction->pressed == id;
}

static void aposaui__icon_zoom(AposaUiCanvas *canvas, AposaUiRect box, const AposaUiMetrics *metrics, bool is_plus,
			       AposaUiColor color) {
	const double center_x = (double)box.x + box.width * 0.5;
	const double center_y = (double)box.y + box.height * 0.5;
	const double radius = (double)box.width * 0.3;
	const int32_t stroke = aposaui_stroke_width(metrics);
	const int32_t arm = (int32_t)(radius * 1.1);
	const AposaUiRect horizontal = {
	    .x = (int32_t)center_x - arm / 2, .y = (int32_t)center_y - stroke / 2, .width = arm, .height = stroke};

	aposaui_paint_ring(canvas, center_x, center_y, radius, (double)stroke, color);
	aposaui_paint_blend_rect(canvas, horizontal, color);
	if (is_plus) {
		const AposaUiRect vertical = {.x = (int32_t)center_x - stroke / 2,
					      .y = (int32_t)center_y - arm / 2,
					      .width = stroke,
					      .height = arm};

		aposaui_paint_blend_rect(canvas, vertical, color);
	}
}

static void aposaui__icon_fit(AposaUiCanvas *canvas, AposaUiRect box, const AposaUiMetrics *metrics,
			      AposaUiColor color) {
	const int32_t stroke = aposaui_stroke_width(metrics);
	const int32_t inset = box.width / 5;
	const int32_t arm = box.width / 4;
	const int32_t left = box.x + inset;
	const int32_t top = box.y + inset;
	const int32_t right = box.x + box.width - inset;
	const int32_t bottom = box.y + box.height - inset;
	const AposaUiRect arms[8] = {
	    {.x = left, .y = top, .width = arm, .height = stroke},
	    {.x = left, .y = top, .width = stroke, .height = arm},
	    {.x = right - arm, .y = top, .width = arm, .height = stroke},
	    {.x = right - stroke, .y = top, .width = stroke, .height = arm},
	    {.x = left, .y = bottom - stroke, .width = arm, .height = stroke},
	    {.x = left, .y = bottom - arm, .width = stroke, .height = arm},
	    {.x = right - arm, .y = bottom - stroke, .width = arm, .height = stroke},
	    {.x = right - stroke, .y = bottom - arm, .width = stroke, .height = arm},
	};

	for (size_t index = 0u; index < sizeof(arms) / sizeof(arms[0]); ++index) {
		aposaui_paint_blend_rect(canvas, arms[index], color);
	}
}

static void aposaui__icon_chevron_down(AposaUiCanvas *canvas, AposaUiRect box, const AposaUiMetrics *metrics,
				       AposaUiColor color) {
	const int32_t stroke = aposaui_stroke_width(metrics);
	const int32_t center_x = box.x + box.width / 2;
	const int32_t center_y = box.y + box.height / 2;
	const int32_t arm = box.width / 4;

	for (int32_t offset = 0; offset < arm; ++offset) {
		aposaui_paint_blend_rect(canvas,
					 (AposaUiRect){.x = center_x - arm + offset,
						       .y = center_y - arm / 2 + offset,
						       .width = stroke,
						       .height = stroke},
					 color);
		aposaui_paint_blend_rect(canvas,
					 (AposaUiRect){.x = center_x + arm - offset - stroke,
						       .y = center_y - arm / 2 + offset,
						       .width = stroke,
						       .height = stroke},
					 color);
	}
}

static void aposaui__icon_navigate(AposaUiCanvas *canvas, AposaUiRect box, int direction, AposaUiColor color) {
	const int32_t center_x = box.x + box.width / 2;
	const int32_t center_y = box.y + box.height / 2;
	const int32_t stroke = box.width < 24 ? 2 : 3;
	const int32_t arm = box.width < 24 ? 6 : box.width / 3;

	for (int32_t offset = 0; offset < arm; ++offset) {
		const int32_t x = direction < 0 ? center_x + arm - offset - stroke : center_x - arm + offset;
		const int32_t upper_y = center_y - arm + offset;
		const int32_t lower_y = center_y + arm - offset - stroke;

		aposaui_paint_blend_rect(canvas, (AposaUiRect){.x = x, .y = upper_y, .width = stroke, .height = stroke},
					 color);
		aposaui_paint_blend_rect(canvas, (AposaUiRect){.x = x, .y = lower_y, .width = stroke, .height = stroke},
					 color);
	}
}

static void aposaui__icon_transport(AposaUiCanvas *canvas, AposaUiRect box, int direction, bool is_paused,
				    AposaUiColor color) {
	const int32_t center_x = box.x + box.width / 2;
	const int32_t center_y = box.y + box.height / 2;

	if (is_paused) {
		aposaui_paint_fill_rect(canvas,
					(AposaUiRect){.x = center_x - 5, .y = center_y - 7, .width = 3, .height = 14},
					color);
		aposaui_paint_fill_rect(canvas,
					(AposaUiRect){.x = center_x + 2, .y = center_y - 7, .width = 3, .height = 14},
					color);
		return;
	}
	for (int32_t row = 0; row < 14; ++row) {
		const int32_t half = row < 7 ? row : 13 - row;
		const int32_t width = half + 2;
		const int32_t x = direction < 0 ? center_x + 4 - width : center_x - 6;

		aposaui_paint_fill_rect(canvas,
					(AposaUiRect){.x = x, .y = center_y - 7 + row, .width = width, .height = 1},
					color);
	}
}

APOSAUI_DEF void aposaui_icon_paint(AposaUiCanvas *canvas, AposaUiIcon icon, AposaUiRect box,
				    const AposaUiMetrics *metrics, AposaUiColor color) {
	switch (icon) {
	case APOSAUI_ICON_ZOOM_IN:
		aposaui__icon_zoom(canvas, box, metrics, true, color);
		break;
	case APOSAUI_ICON_ZOOM_OUT:
		aposaui__icon_zoom(canvas, box, metrics, false, color);
		break;
	case APOSAUI_ICON_FIT:
		aposaui__icon_fit(canvas, box, metrics, color);
		break;
	case APOSAUI_ICON_CHEVRON_DOWN:
		aposaui__icon_chevron_down(canvas, box, metrics, color);
		break;
	case APOSAUI_ICON_NAVIGATE_LEFT:
		aposaui__icon_navigate(canvas, box, -1, color);
		break;
	case APOSAUI_ICON_NAVIGATE_RIGHT:
		aposaui__icon_navigate(canvas, box, 1, color);
		break;
	case APOSAUI_ICON_PLAY:
		aposaui__icon_transport(canvas, box, 1, false, color);
		break;
	case APOSAUI_ICON_PLAY_REVERSE:
		aposaui__icon_transport(canvas, box, -1, false, color);
		break;
	case APOSAUI_ICON_PAUSE:
		aposaui__icon_transport(canvas, box, 1, true, color);
		break;
	default:
		break;
	}
}

APOSAUI_DEF AposaUiColor aposaui_control_background(AposaUiCanvas *canvas, AposaUiRect rect, double radius,
						    bool selected, bool is_hovered, bool is_pressed,
						    const AposaUiThemeColors *colors) {
	if (selected) {
		aposaui_paint_rounded_rect(canvas, rect, radius, colors->option_selected);
	} else if (is_pressed) {
		aposaui_paint_rounded_rect(canvas, rect, radius, colors->control_pressed);
	} else if (is_hovered) {
		aposaui_paint_rounded_rect(canvas, rect, radius, colors->control_hover);
	}
	return selected || is_hovered ? colors->control_stroke_active : colors->control_stroke;
}

APOSAUI_DEF void aposaui_text_centered(AposaUiCanvas *canvas, AposaUiFont *font, AposaUiRect rect, const char *text,
				       AposaUiColor color) {
	const int32_t text_width = aposaui_font_measure_text(font, text);

	aposaui_font_draw_text(canvas, font, rect.x + (rect.width - text_width) / 2, aposaui_text_baseline(rect, font),
			       text, color);
}

APOSAUI_DEF int32_t aposaui_badge_measure(const char *text, const AposaUiMetrics *metrics, AposaUiFont *font) {
	return aposaui_font_measure_text(font, text) + 2 * (int32_t)metrics->badge_padding_x;
}

APOSAUI_DEF int32_t aposaui_badge_paint(AposaUiCanvas *canvas, const char *text, int32_t right_edge, AposaUiRect bar,
					const AposaUiMetrics *metrics, AposaUiFont *font,
					const AposaUiThemeColors *colors) {
	const int32_t badge_width = aposaui_badge_measure(text, metrics, font);
	const int32_t badge_height = (int32_t)metrics->badge_height;
	const AposaUiRect badge = {.x = right_edge - badge_width,
				   .y = bar.y + (bar.height - badge_height) / 2,
				   .width = badge_width,
				   .height = badge_height};

	aposaui_paint_rounded_rect(canvas, badge, (double)badge_height * 0.25, colors->badge_background);
	aposaui_font_draw_text(canvas, font, badge.x + (int32_t)metrics->badge_padding_x,
			       aposaui_text_baseline(bar, font), text, colors->badge_text);
	return badge.x;
}

APOSAUI_DEF void aposaui_menu_panel_paint(AposaUiCanvas *canvas, AposaUiRect menu, const AposaUiThemeColors *colors) {
	aposaui_paint_rounded_rect(canvas, menu, 4.0, colors->badge_background);
}

APOSAUI_DEF void aposaui_menu_item_paint(AposaUiCanvas *canvas, AposaUiRect menu, int32_t row, const char *label,
					 const char *shortcut, int32_t row_count, const AposaUiMetrics *metrics,
					 AposaUiFont *font, const AposaUiThemeColors *colors) {
	const int32_t row_height = menu.height / row_count;
	const AposaUiRect item = {
	    .x = menu.x, .y = menu.y + row * row_height, .width = menu.width, .height = row_height};
	const int32_t padding = (int32_t)metrics->padding_x;

	aposaui_font_draw_text(canvas, font, item.x + padding, aposaui_text_baseline(item, font), label,
			       colors->text_primary);
	if (shortcut != NULL) {
		const int32_t shortcut_width = aposaui_font_measure_text(font, shortcut);

		aposaui_font_draw_text(canvas, font, item.x + item.width - padding - shortcut_width,
				       aposaui_text_baseline(item, font), shortcut, colors->text_secondary);
	}
}

APOSAUI_DEF void aposaui_scrollbar_paint(AposaUiCanvas *canvas, AposaUiRect track, AposaUiRect thumb, bool is_hovered,
					 bool is_pressed, const AposaUiThemeColors *colors) {
	const AposaUiColor thumb_color = is_pressed ? colors->option_accent
						    : (is_hovered ? colors->control_stroke_active
								  : colors->option_selected);

	aposaui_paint_rounded_rect(canvas, track, (double)track.width / 2.0, colors->control_hover);
	aposaui_paint_rounded_rect(canvas, thumb, (double)thumb.width / 2.0, thumb_color);
}

APOSAUI_DEF void aposaui_list_paint(AposaUiCanvas *canvas, AposaUiRect list, int32_t row_height,
				    const AposaUiListModel *model, uint32_t scroll_offset, uint32_t visible_count,
				    AposaUiFont *font, const AposaUiThemeColors *colors, const char *empty_text) {
	const int32_t item_padding = row_height > 0 ? row_height / 4 : 0;
	const int32_t horizontal_padding = row_height > 0 ? row_height * 3 / 8 : 0;
	const int32_t badge_height = row_height > 0 ? row_height * 5 / 8 : 0;
	const int32_t list_bottom = list.y + list.height;
	int32_t item_y = list.y + item_padding;

	if (model == NULL || model->get_row == NULL || model->row_count == 0u) {
		if (empty_text != NULL) {
			aposaui_font_draw_text(canvas, font, list.x + 12, list.y + 12, empty_text, colors->text_dim);
		}
		return;
	}
	for (uint32_t index = scroll_offset, visible_index = 0u;
	     index < model->row_count && visible_index < visible_count; ++index, ++visible_index) {
		AposaUiListRow content = {0};
		int32_t text_width;
		int32_t badge_padding;
		int32_t minimum_badge_width;
		int32_t measured_badge_width;
		int32_t key_width;
		AposaUiRect line;
		AposaUiRect badge;

		if (row_height <= 0 || item_y + row_height > list_bottom) {
			break;
		}
		if (!model->get_row(model->user, index, &content) || content.value == NULL) {
			continue;
		}
		text_width = aposaui_font_measure_text(font, content.value);
		badge_padding = item_padding > 0 ? item_padding : 1;
		minimum_badge_width = row_height * 5 / 2;
		measured_badge_width = text_width + badge_padding * 2;
		key_width = measured_badge_width > minimum_badge_width ? measured_badge_width : minimum_badge_width;
		line = (AposaUiRect){.x = list.x, .y = item_y, .width = list.width, .height = row_height};
		badge = (AposaUiRect){.x = list.x + list.width - key_width - horizontal_padding,
				      .y = item_y + (row_height - badge_height) / 2,
				      .width = key_width,
				      .height = badge_height};

		aposaui_font_draw_text(canvas, font, list.x + horizontal_padding, aposaui_text_baseline(line, font),
				       content.label, colors->text_primary);
		aposaui_paint_rounded_rect(canvas, badge, (double)badge.height / 3.0, colors->option_selected);
		aposaui_font_draw_text(canvas, font, badge.x + badge_padding, aposaui_text_baseline(badge, font),
				       content.value, colors->text_primary);
		item_y += row_height;
	}
}

#define APOSAUI__TIMELINE_TRACK_HEIGHT 6
#define APOSAUI__TIMELINE_TRACK_RADIUS 3.0
#define APOSAUI__TIMELINE_THUMB_RADIUS 6.0
#define APOSAUI__TIMELINE_BAR_RADIUS_SCALE 0.35
#define APOSAUI__TIMELINE_BAR_COLOR 0xb8202020u
#define APOSAUI__TIMELINE_TRACK_COLOR 0x557f8794u

APOSAUI_DEF AposaUiTimeline aposaui_timeline_layout(AposaUiRect bar, int32_t label_width, int32_t label_gap) {
	const int32_t track_x = bar.x + label_width + label_gap;
	const int32_t track_right = bar.x + bar.width - label_width - label_gap;
	const int32_t track_y = bar.y + bar.height / 2;
	AposaUiTimeline timeline;

	timeline.bar = bar;
	timeline.track = (AposaUiRect){.x = track_x,
				       .y = track_y - APOSAUI__TIMELINE_TRACK_HEIGHT / 2,
				       .width = track_right - track_x,
				       .height = APOSAUI__TIMELINE_TRACK_HEIGHT};
	timeline.leading_x = bar.x + label_gap;
	timeline.trailing_x = track_right + label_gap;
	return timeline;
}

APOSAUI_DEF void aposaui_timeline_paint(AposaUiCanvas *canvas, const AposaUiTimeline *timeline, double progress,
					AposaUiFont *font, const AposaUiThemeColors *colors, const char *leading_text,
					const char *trailing_text) {
	const double clamped = progress < 0.0 ? 0.0 : (progress > 1.0 ? 1.0 : progress);
	const int32_t centerline = timeline->track.y + timeline->track.height / 2;

	aposaui_paint_rounded_rect(canvas, timeline->bar,
				   (double)timeline->bar.height * APOSAUI__TIMELINE_BAR_RADIUS_SCALE,
				   APOSAUI__TIMELINE_BAR_COLOR);
	if (leading_text != NULL) {
		aposaui_font_draw_text(canvas, font, timeline->leading_x, aposaui_text_baseline(timeline->bar, font),
				       leading_text, colors->text_primary);
	}
	if (timeline->track.width > 0) {
		const int32_t filled = (int32_t)((double)timeline->track.width * clamped + 0.5);

		aposaui_paint_rounded_rect(canvas, timeline->track, APOSAUI__TIMELINE_TRACK_RADIUS,
					   APOSAUI__TIMELINE_TRACK_COLOR);
		if (filled > 0) {
			aposaui_paint_rounded_rect(canvas,
						   (AposaUiRect){.x = timeline->track.x,
								 .y = timeline->track.y,
								 .width = filled,
								 .height = timeline->track.height},
						   APOSAUI__TIMELINE_TRACK_RADIUS, colors->option_accent);
		}
		aposaui_paint_disc(canvas, (double)timeline->track.x + (double)timeline->track.width * clamped,
				   (double)centerline, APOSAUI__TIMELINE_THUMB_RADIUS, colors->option_accent);
	}
	if (trailing_text != NULL) {
		aposaui_font_draw_text(canvas, font, timeline->trailing_x, aposaui_text_baseline(timeline->bar, font),
				       trailing_text, colors->text_secondary);
	}
}

APOSAUI_DEF double aposaui_timeline_progress_at(const AposaUiTimeline *timeline, int32_t x) {
	double progress;

	if (timeline == NULL || timeline->track.width <= 0) {
		return 0.0;
	}
	progress = (double)(x - timeline->track.x) / (double)timeline->track.width;
	return progress < 0.0 ? 0.0 : (progress > 1.0 ? 1.0 : progress);
}

#ifndef APOSAUI_NO_PLATFORM

/* -------------------------------------------------------------------------
 * Win32 implementation: input events
 * ---------------------------------------------------------------------- */

APOSAUI_DEF uint32_t aposaui_modifiers_from_keyboard(void) {
	uint32_t modifiers = 0u;

	if ((GetKeyState(VK_CONTROL) & 0x8000) != 0) {
		modifiers |= APOSAUI_MOD_CONTROL;
	}
	if ((GetKeyState(VK_SHIFT) & 0x8000) != 0) {
		modifiers |= APOSAUI_MOD_SHIFT;
	}
	if ((GetKeyState(VK_MENU) & 0x8000) != 0) {
		modifiers |= APOSAUI_MOD_ALT;
	}
	return modifiers;
}

APOSAUI_DEF bool aposaui_event_from_message(void *window, uint32_t message, uint64_t w_param, int64_t l_param,
					    AposaUiEvent *out_event) {
	HWND handle = (HWND)window;

	if (out_event == NULL) {
		return false;
	}
	*out_event = (AposaUiEvent){.type = APOSAUI_EVENT_NONE};
	switch (message) {
	case WM_MOUSEMOVE:
		out_event->type = APOSAUI_EVENT_POINTER_MOVE;
		break;
	case WM_LBUTTONDOWN:
		out_event->type = APOSAUI_EVENT_POINTER_DOWN;
		break;
	case WM_LBUTTONUP:
		out_event->type = APOSAUI_EVENT_POINTER_UP;
		break;
	case WM_MOUSELEAVE:
		out_event->type = APOSAUI_EVENT_POINTER_LEAVE;
		return true;
	case WM_MOUSEWHEEL: {
		/* The wheel message is the one delivered in screen coordinates. */
		POINT cursor = {GET_X_LPARAM(l_param), GET_Y_LPARAM(l_param)};

		if (handle != NULL) {
			(void)ScreenToClient(handle, &cursor);
		}
		out_event->type = APOSAUI_EVENT_WHEEL;
		out_event->x = cursor.x;
		out_event->y = cursor.y;
		out_event->wheel_notches = (double)GET_WHEEL_DELTA_WPARAM(w_param) / (double)WHEEL_DELTA;
		return true;
	}
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		out_event->type = APOSAUI_EVENT_KEY;
		out_event->virtual_key = (uint32_t)w_param;
		out_event->modifiers = aposaui_modifiers_from_keyboard();
		out_event->is_repeat = (l_param & (1 << 30)) != 0;
		return true;
	case WM_SIZE:
		out_event->type = APOSAUI_EVENT_RESIZE;
		out_event->window_width = (uint32_t)LOWORD(l_param);
		out_event->window_height = (uint32_t)HIWORD(l_param);
		return true;
	case WM_TIMER:
		out_event->type = APOSAUI_EVENT_TIMER;
		out_event->timer_id = (uint32_t)w_param;
		return true;
	case WM_DROPFILES:
		out_event->type = APOSAUI_EVENT_FILES_DROPPED;
		return true;
	case WM_DESTROY:
		out_event->type = APOSAUI_EVENT_CLOSE;
		return true;
	default:
		return false;
	}
	out_event->x = GET_X_LPARAM(l_param);
	out_event->y = GET_Y_LPARAM(l_param);
	return true;
}

/* -------------------------------------------------------------------------
 * Win32 implementation: host
 * ---------------------------------------------------------------------- */

typedef struct AposaUiHost {
	AposaUiAllocator allocator;
	HWND window;
	AposaUiFont *primary_font;
	AposaUiFont *numeric_font;
	BITMAPINFO bitmap_info;
	uint32_t dpi;
	bool has_bitmap_info;
} AposaUiHost;

static const wchar_t APOSAUI__HOST_CLASS_NAME[] = L"AposaUiHostWindow";

APOSAUI_DEF uint32_t aposaui_host_system_dpi(void) {
	HDC device_context = GetDC(NULL);
	int32_t dpi = 0;

	if (device_context != NULL) {
		dpi = GetDeviceCaps(device_context, LOGPIXELSX);
		(void)ReleaseDC(NULL, device_context);
	}
	return dpi > 0 ? (uint32_t)dpi : APOSAUI_BASE_DPI;
}

static int aposaui__host_register_class(HINSTANCE instance, WNDPROC window_proc) {
	const WNDCLASSEXW window_class = {.cbSize = sizeof(window_class),
					  .style = CS_HREDRAW | CS_VREDRAW,
					  .lpfnWndProc = window_proc,
					  .hInstance = instance,
					  .hCursor = LoadCursorW(NULL, MAKEINTRESOURCEW(32512)),
					  .hbrBackground = NULL,
					  .lpszClassName = APOSAUI__HOST_CLASS_NAME};

	return RegisterClassExW(&window_class) == 0u && GetLastError() != ERROR_CLASS_ALREADY_EXISTS ? -1 : 0;
}

APOSAUI_DEF int aposaui_host_create(const AposaUiHostDesc *desc, const AposaUiAllocator *allocator,
				    AposaUiHost **out_host) {
	HINSTANCE instance = GetModuleHandleW(NULL);
	wchar_t title[256];
	RECT bounds = {0, 0, 0, 0};
	AposaUiHost *host;

	if (desc == NULL || allocator == NULL || allocator->allocate == NULL || allocator->deallocate == NULL ||
	    out_host == NULL || desc->window_proc == NULL) {
		return -1;
	}
	if (desc->title == NULL ||
	    MultiByteToWideChar(CP_UTF8, 0, desc->title, -1, title, (int)(sizeof(title) / sizeof(title[0]))) == 0) {
		return -1;
	}
	host = allocator->allocate(allocator->user, sizeof(*host), _Alignof(AposaUiHost));
	if (host == NULL) {
		return -1;
	}
	*host = (AposaUiHost){.allocator = *allocator, .dpi = aposaui_host_system_dpi()};
	if (aposaui__host_register_class(instance, desc->window_proc) != 0) {
		goto cleanup;
	}
	if (aposaui_font_create(&host->allocator, desc->primary_font_family, desc->primary_font_size,
				&host->primary_font) != 0) {
		goto cleanup;
	}
	if (aposaui_font_create(&host->allocator, desc->numeric_font_family, desc->numeric_font_size,
				&host->numeric_font) != 0) {
		goto cleanup;
	}
	bounds.right = desc->client_width;
	bounds.bottom = desc->client_height;
	(void)AdjustWindowRectEx(&bounds, WS_OVERLAPPEDWINDOW, FALSE, 0u);
	host->window = CreateWindowExW(0u, APOSAUI__HOST_CLASS_NAME, title, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
				       CW_USEDEFAULT, bounds.right - bounds.left, bounds.bottom - bounds.top, NULL, NULL,
				       instance, desc->user);
	if (host->window == NULL) {
		goto cleanup;
	}
	DragAcceptFiles(host->window, TRUE);
	*out_host = host;
	return 0;

cleanup:
	aposaui_host_destroy(host);
	return -1;
}

APOSAUI_DEF void aposaui_host_destroy(AposaUiHost *host) {
	AposaUiAllocator allocator;

	if (host == NULL) {
		return;
	}
	allocator = host->allocator;
	aposaui_font_destroy(host->primary_font);
	aposaui_font_destroy(host->numeric_font);
	host->primary_font = NULL;
	host->numeric_font = NULL;
	if (host->window != NULL) {
		DestroyWindow(host->window);
		host->window = NULL;
	}
	allocator.deallocate(allocator.user, host);
}

APOSAUI_DEF void *aposaui_host_window(const AposaUiHost *host) {
	return host == NULL ? NULL : (void *)host->window;
}

APOSAUI_DEF uint32_t aposaui_host_dpi(const AposaUiHost *host) {
	return host == NULL ? APOSAUI_BASE_DPI : host->dpi;
}

APOSAUI_DEF AposaUiFont *aposaui_host_primary_font(const AposaUiHost *host) {
	return host == NULL ? NULL : host->primary_font;
}

APOSAUI_DEF AposaUiFont *aposaui_host_numeric_font(const AposaUiHost *host) {
	return host == NULL ? NULL : host->numeric_font;
}

APOSAUI_DEF void aposaui_host_apply_dark_title_bar(AposaUiHost *host, bool enabled) {
	typedef HRESULT(WINAPI * AposaUiDwmSetWindowAttribute)(HWND, DWORD, LPCVOID, DWORD);
	const DWORD recent_attribute = 20u;
	const DWORD legacy_attribute = 19u;
	const BOOL value = enabled ? TRUE : FALSE;
	HMODULE library = LoadLibraryW(L"dwmapi.dll");
	AposaUiDwmSetWindowAttribute set_attribute;

	if (host == NULL || library == NULL) {
		return;
	}
	set_attribute = (AposaUiDwmSetWindowAttribute)(void *)GetProcAddress(library, "DwmSetWindowAttribute");
	if (set_attribute != NULL) {
		if (FAILED(set_attribute(host->window, recent_attribute, &value, sizeof(value)))) {
			(void)set_attribute(host->window, legacy_attribute, &value, sizeof(value));
		}
	}
	(void)FreeLibrary(library);
}

APOSAUI_DEF void aposaui_host_present(AposaUiHost *host, void *device_context, const AposaUiCanvas *canvas) {
	HDC target = (HDC)device_context;

	if (host == NULL || target == NULL || canvas == NULL || canvas->pixels == NULL || canvas->width == 0u ||
	    canvas->height == 0u) {
		return;
	}
	if (!host->has_bitmap_info || host->bitmap_info.bmiHeader.biWidth != (LONG)canvas->width ||
	    host->bitmap_info.bmiHeader.biHeight != -(LONG)canvas->height) {
		host->bitmap_info = (BITMAPINFO){0};
		host->bitmap_info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		host->bitmap_info.bmiHeader.biWidth = (LONG)canvas->width;
		host->bitmap_info.bmiHeader.biHeight = -(LONG)canvas->height;
		host->bitmap_info.bmiHeader.biPlanes = 1;
		host->bitmap_info.bmiHeader.biBitCount = 32;
		host->bitmap_info.bmiHeader.biCompression = BI_RGB;
		host->has_bitmap_info = true;
	}
	SetStretchBltMode(target, COLORONCOLOR);
	StretchDIBits(target, 0, 0, (int)canvas->width, (int)canvas->height, 0, 0, (int)canvas->width,
		      (int)canvas->height, canvas->pixels, &host->bitmap_info, DIB_RGB_COLORS, SRCCOPY);
}

/* -------------------------------------------------------------------------
 * Win32 implementation: GDI font backend
 *
 * The glyph cache is open-addressed and never evicts, so a font's footprint is
 * bounded by the number of distinct codepoints a caller ever draws.
 * ---------------------------------------------------------------------- */

#define APOSAUI__FONT_CACHE_CAPACITY 1024u

typedef struct AposaUiFontCacheEntry {
	uint32_t codepoint;
	bool is_used;
	uint8_t *coverage;
	AposaUiGlyph glyph;
} AposaUiFontCacheEntry;

struct AposaUiFont {
	AposaUiAllocator allocator;
	HDC device_context;
	HFONT font;
	HGDIOBJ previous_font;
	uint32_t ascent;
	uint32_t line_height;
	AposaUiFontCacheEntry entries[APOSAUI__FONT_CACHE_CAPACITY];
};

static uint32_t aposaui__font_hash(uint32_t codepoint) {
	codepoint ^= codepoint >> 16u;
	codepoint *= 0x7feb352du;
	codepoint ^= codepoint >> 15u;
	return codepoint;
}

static AposaUiFontCacheEntry *aposaui__font_find_slot(AposaUiFont *font, uint32_t codepoint) {
	uint32_t index = aposaui__font_hash(codepoint) & (APOSAUI__FONT_CACHE_CAPACITY - 1u);

	for (uint32_t probe = 0u; probe < APOSAUI__FONT_CACHE_CAPACITY; ++probe) {
		AposaUiFontCacheEntry *entry = &font->entries[index];

		if (!entry->is_used || entry->codepoint == codepoint) {
			return entry;
		}
		index = (index + 1u) & (APOSAUI__FONT_CACHE_CAPACITY - 1u);
	}
	return NULL;
}

static int aposaui__font_rasterize(AposaUiFont *font, uint32_t codepoint, AposaUiFontCacheEntry *entry) {
	const MAT2 identity = {{0, 1}, {0, 0}, {0, 0}, {0, 1}};
	GLYPHMETRICS metrics = {0};
	DWORD size;
	uint8_t *buffer;
	uint32_t source_stride;

	entry->codepoint = codepoint;
	entry->is_used = true;
	entry->coverage = NULL;
	entry->glyph = (AposaUiGlyph){0};
	size = GetGlyphOutlineW(font->device_context, (UINT)codepoint, GGO_GRAY8_BITMAP, &metrics, 0u, NULL, &identity);
	if (size == GDI_ERROR) {
		return 0;
	}
	entry->glyph.advance = metrics.gmCellIncX;
	entry->glyph.bearing_x = metrics.gmptGlyphOrigin.x;
	entry->glyph.bearing_y = metrics.gmptGlyphOrigin.y;
	if (size == 0u || metrics.gmBlackBoxX == 0u || metrics.gmBlackBoxY == 0u) {
		return 0;
	}
	buffer = font->allocator.allocate(font->allocator.user, size, 4u);
	if (buffer == NULL) {
		entry->is_used = false;
		return -1;
	}
	if (GetGlyphOutlineW(font->device_context, (UINT)codepoint, GGO_GRAY8_BITMAP, &metrics, size, buffer,
			     &identity) == GDI_ERROR) {
		font->allocator.deallocate(font->allocator.user, buffer);
		return 0;
	}
	source_stride = (metrics.gmBlackBoxX + 3u) & ~3u;
	for (uint32_t row = 0u; row < metrics.gmBlackBoxY; ++row) {
		for (uint32_t column = 0u; column < metrics.gmBlackBoxX; ++column) {
			const uint32_t value = buffer[(size_t)row * source_stride + column];
			const uint32_t scaled = value >= 64u ? 255u : (value * 255u + 32u) / 64u;

			buffer[(size_t)row * metrics.gmBlackBoxX + column] = (uint8_t)scaled;
		}
	}
	entry->coverage = buffer;
	entry->glyph.width = metrics.gmBlackBoxX;
	entry->glyph.height = metrics.gmBlackBoxY;
	entry->glyph.coverage = buffer;
	return 0;
}

APOSAUI_DEF const AposaUiGlyph *aposaui_font_glyph(AposaUiFont *font, uint32_t codepoint) {
	AposaUiFontCacheEntry *entry;

	if (font == NULL) {
		return NULL;
	}
	entry = aposaui__font_find_slot(font, codepoint);
	if (entry == NULL) {
		return NULL;
	}
	if (!entry->is_used && aposaui__font_rasterize(font, codepoint, entry) != 0) {
		return NULL;
	}
	return &entry->glyph;
}

APOSAUI_DEF uint32_t aposaui_font_ascent(const AposaUiFont *font) {
	return font == NULL ? 0u : font->ascent;
}

APOSAUI_DEF uint32_t aposaui_font_line_height(const AposaUiFont *font) {
	return font == NULL ? 0u : font->line_height;
}

APOSAUI_DEF int aposaui_font_create(const AposaUiAllocator *allocator, const char *family, uint32_t pixel_size,
				    AposaUiFont **out_font) {
	LOGFONTW descriptor = {0};
	TEXTMETRICW text_metrics;
	AposaUiFont *font;

	if (allocator == NULL || allocator->allocate == NULL || allocator->deallocate == NULL || family == NULL ||
	    pixel_size == 0u || out_font == NULL) {
		return -1;
	}
	font = allocator->allocate(allocator->user, sizeof(*font), _Alignof(AposaUiFont));
	if (font == NULL) {
		return -1;
	}
	memset(font, 0, sizeof(*font));
	font->allocator = *allocator;
	descriptor.lfHeight = -(LONG)pixel_size;
	descriptor.lfWeight = FW_NORMAL;
	descriptor.lfCharSet = DEFAULT_CHARSET;
	descriptor.lfOutPrecision = OUT_TT_PRECIS;
	descriptor.lfQuality = CLEARTYPE_QUALITY;
	descriptor.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
	if (MultiByteToWideChar(CP_UTF8, 0, family, -1, descriptor.lfFaceName, LF_FACESIZE) == 0) {
		goto cleanup;
	}
	font->device_context = CreateCompatibleDC(NULL);
	if (font->device_context == NULL) {
		goto cleanup;
	}
	font->font = CreateFontIndirectW(&descriptor);
	if (font->font == NULL) {
		goto cleanup;
	}
	font->previous_font = SelectObject(font->device_context, font->font);
	if (GetTextMetricsW(font->device_context, &text_metrics) == 0) {
		goto cleanup;
	}
	font->ascent = (uint32_t)text_metrics.tmAscent;
	font->line_height = (uint32_t)(text_metrics.tmHeight + text_metrics.tmExternalLeading);
	*out_font = font;
	return 0;

cleanup:
	aposaui_font_destroy(font);
	return -1;
}

APOSAUI_DEF void aposaui_font_destroy(AposaUiFont *font) {
	AposaUiAllocator allocator;

	if (font == NULL || font->allocator.deallocate == NULL) {
		return;
	}
	allocator = font->allocator;
	for (uint32_t index = 0u; index < APOSAUI__FONT_CACHE_CAPACITY; ++index) {
		if (font->entries[index].coverage != NULL) {
			allocator.deallocate(allocator.user, font->entries[index].coverage);
		}
	}
	if (font->device_context != NULL) {
		if (font->previous_font != NULL) {
			SelectObject(font->device_context, font->previous_font);
		}
		DeleteDC(font->device_context);
	}
	if (font->font != NULL) {
		DeleteObject(font->font);
	}
	allocator.deallocate(allocator.user, font);
}

#endif /* APOSAUI_NO_PLATFORM */

#endif /* APOSAUI_IMPLEMENTATION */
