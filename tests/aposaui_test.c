#include "aposaui.h"
#include "aposaui_stub_font.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define APOSAUI_TEST_WIDTH 16u
#define APOSAUI_TEST_HEIGHT 8u
#define APOSAUI_TEST_GUARD 0xdeadbeefu
#define APOSAUI_TEST_LEAD 2u
#define APOSAUI_TEST_TAIL 2u

static uint32_t aposaui_test_storage[APOSAUI_TEST_LEAD + APOSAUI_TEST_WIDTH * APOSAUI_TEST_HEIGHT +
				     APOSAUI_TEST_TAIL];

static void aposaui_test_reset(void) {
	const size_t total = sizeof(aposaui_test_storage) / sizeof(aposaui_test_storage[0]);

	for (size_t index = 0u; index < total; ++index) {
		aposaui_test_storage[index] = APOSAUI_TEST_GUARD;
	}
	for (size_t index = 0u; index < APOSAUI_TEST_WIDTH * APOSAUI_TEST_HEIGHT; ++index) {
		aposaui_test_storage[APOSAUI_TEST_LEAD + index] = 0xff000000u;
	}
}

static AposaUiCanvas aposaui_test_canvas(void) {
	AposaUiCanvas canvas = {.width = APOSAUI_TEST_WIDTH,
				.height = APOSAUI_TEST_HEIGHT,
				.stride = APOSAUI_TEST_WIDTH * (uint32_t)sizeof(uint32_t),
				.pixels = (uint8_t *)(void *)(aposaui_test_storage + APOSAUI_TEST_LEAD)};

	return canvas;
}

static uint32_t aposaui_test_pixel(uint32_t x, uint32_t y) {
	return aposaui_test_storage[APOSAUI_TEST_LEAD + y * APOSAUI_TEST_WIDTH + x];
}

static bool aposaui_test_guards_intact(void) {
	for (size_t index = 0u; index < APOSAUI_TEST_LEAD; ++index) {
		if (aposaui_test_storage[index] != APOSAUI_TEST_GUARD) {
			return false;
		}
	}
	for (size_t index = 0u; index < APOSAUI_TEST_TAIL; ++index) {
		if (aposaui_test_storage[APOSAUI_TEST_LEAD + APOSAUI_TEST_WIDTH * APOSAUI_TEST_HEIGHT + index] !=
		    APOSAUI_TEST_GUARD) {
			return false;
		}
	}
	return true;
}

static int aposaui_rect_contains_matches_the_half_open_rectangle(void) {
	const AposaUiRect rect = {.x = 10, .y = 10, .width = 5, .height = 5};

	if (!aposaui_rect_contains(rect, 10, 10) || !aposaui_rect_contains(rect, 14, 14)) {
		return 1;
	}
	if (aposaui_rect_contains(rect, 15, 15) || aposaui_rect_contains(rect, 9, 10) ||
	    aposaui_rect_contains(rect, 10, 9)) {
		return 1;
	}
	return aposaui_rect_contains((AposaUiRect){.x = 0, .y = 0, .width = -1, .height = 4}, 0, 0);
}

static int aposaui_rect_inset_shrinks_each_edge(void) {
	const AposaUiRect inset = aposaui_rect_inset((AposaUiRect){.x = 0, .y = 0, .width = 10, .height = 10}, 2);

	return inset.x != 2 || inset.y != 2 || inset.width != 6 || inset.height != 6;
}

static int aposaui_paint_fill_rect_clips_to_canvas(void) {
	AposaUiCanvas canvas = aposaui_test_canvas();

	aposaui_test_reset();
	aposaui_paint_fill_rect(&canvas, (AposaUiRect){.x = -4, .y = -4, .width = 8, .height = 8}, 0xffff0000u);
	if (aposaui_test_pixel(0u, 0u) != 0xffff0000u || aposaui_test_pixel(3u, 3u) != 0xffff0000u) {
		return 1;
	}
	if (aposaui_test_pixel(4u, 0u) != 0xff000000u || aposaui_test_pixel(0u, 4u) != 0xff000000u) {
		return 1;
	}
	return aposaui_test_guards_intact() ? 0 : 1;
}

static int aposaui_paint_fill_rect_ignores_the_colour_alpha(void) {
	AposaUiCanvas canvas = aposaui_test_canvas();

	aposaui_test_reset();
	aposaui_paint_fill_rect(&canvas, (AposaUiRect){.x = 0, .y = 0, .width = 1, .height = 1}, 0x00123456u);
	return aposaui_test_pixel(0u, 0u) != 0xff123456u;
}

static int aposaui_paint_blend_rect_is_alpha_correct(void) {
	AposaUiCanvas canvas = aposaui_test_canvas();

	aposaui_test_reset();
	aposaui_paint_blend_rect(&canvas, (AposaUiRect){.x = 0, .y = 0, .width = 1, .height = 1}, 0x80ffffffu);
	return aposaui_test_pixel(0u, 0u) != 0xff808080u;
}

static int aposaui_paint_blend_rect_ignores_a_transparent_colour(void) {
	AposaUiCanvas canvas = aposaui_test_canvas();

	aposaui_test_reset();
	aposaui_paint_blend_rect(&canvas, (AposaUiRect){.x = 0, .y = 0, .width = 16, .height = 8}, 0x00ffffffu);
	return aposaui_test_pixel(0u, 0u) != 0xff000000u || aposaui_test_pixel(15u, 7u) != 0xff000000u;
}

static int aposaui_paint_rounded_rect_leaves_corners_clear(void) {
	AposaUiCanvas canvas = aposaui_test_canvas();
	const AposaUiRect rect = {.x = 0, .y = 0, .width = (int32_t)APOSAUI_TEST_WIDTH, .height = (int32_t)APOSAUI_TEST_HEIGHT};

	aposaui_test_reset();
	aposaui_paint_rounded_rect(&canvas, rect, 4.0, 0xffff0000u);
	if (aposaui_test_pixel(0u, 0u) != 0xff000000u) {
		return 1;
	}
	if (aposaui_test_pixel(APOSAUI_TEST_WIDTH / 2u, APOSAUI_TEST_HEIGHT / 2u) != 0xffff0000u) {
		return 1;
	}
	return aposaui_test_guards_intact() ? 0 : 1;
}

static int aposaui_paint_disc_clips_to_canvas(void) {
	AposaUiCanvas canvas = aposaui_test_canvas();

	aposaui_test_reset();
	aposaui_paint_disc(&canvas, -8.0, -8.0, 20.0, 0xffffffffu);
	return aposaui_test_guards_intact() ? 0 : 1;
}

static int aposaui_metrics_scale_with_dpi(void) {
	const AposaUiMetrics base = aposaui_metrics_for_dpi(APOSAUI_BASE_DPI);
	const AposaUiMetrics doubled = aposaui_metrics_for_dpi(APOSAUI_BASE_DPI * 2u);

	return base.top_bar_height != 34u || doubled.top_bar_height != 68u || doubled.button_size != base.button_size * 2u;
}

static int aposaui_metrics_treat_zero_dpi_as_the_base(void) {
	const AposaUiMetrics base = aposaui_metrics_for_dpi(APOSAUI_BASE_DPI);
	const AposaUiMetrics zero = aposaui_metrics_for_dpi(0u);

	return memcmp(&base, &zero, sizeof(base)) != 0;
}

static int aposaui_metrics_keep_a_visible_separator(void) {
	return aposaui_metrics_for_dpi(1u).separator_height != 1u;
}

static int aposaui_stroke_width_is_never_below_one(void) {
	const AposaUiMetrics base = aposaui_metrics_for_dpi(APOSAUI_BASE_DPI);
	const AposaUiMetrics large = aposaui_metrics_for_dpi(APOSAUI_BASE_DPI * 4u);

	if (aposaui_stroke_width(&base) != 1) {
		return 1;
	}
	if (aposaui_stroke_width(NULL) != 1) {
		return 1;
	}
	return aposaui_stroke_width(&large) != 6;
}

static int aposaui_theme_colors_separate_the_two_modes(void) {
	const AposaUiThemeColors dark = aposaui_theme_colors(APOSAUI_THEME_MODE_DARK);
	const AposaUiThemeColors light = aposaui_theme_colors(APOSAUI_THEME_MODE_LIGHT);

	if (dark.canvas != APOSAUI_COLOR_CANVAS || light.canvas == dark.canvas) {
		return 1;
	}
	if (light.text_primary == dark.text_primary || light.panel == dark.panel) {
		return 1;
	}
	return light.canvas != 0xfff2f2f2u;
}

static int aposaui_format_duration_renders_minutes_and_seconds(void) {
	char buffer[32];

	aposaui_format_duration(0u, buffer, sizeof(buffer));
	if (strcmp(buffer, "0:00") != 0) {
		return 1;
	}
	aposaui_format_duration(65000u, buffer, sizeof(buffer));
	if (strcmp(buffer, "1:05") != 0) {
		return 1;
	}
	aposaui_format_duration(3599000u, buffer, sizeof(buffer));
	return strcmp(buffer, "59:59") != 0;
}

static int aposaui_format_dimensions_uses_the_multiplication_sign(void) {
	char buffer[64];

	aposaui_format_dimensions(1920u, 1080u, buffer, sizeof(buffer));
	return strcmp(buffer, "1920 \xc3\x97 1080") != 0;
}

static int aposaui_format_percent_rounds_to_whole_percent(void) {
	char buffer[32];

	aposaui_format_percent(100.0, buffer, sizeof(buffer));
	if (strcmp(buffer, "100%") != 0) {
		return 1;
	}
	aposaui_format_percent(33.6, buffer, sizeof(buffer));
	return strcmp(buffer, "34%") != 0;
}

static int aposaui_format_helpers_ignore_a_null_buffer(void) {
	char buffer[8];

	aposaui_format_duration(1000u, NULL, sizeof(buffer));
	aposaui_format_dimensions(1u, 1u, NULL, sizeof(buffer));
	aposaui_format_percent(1.0, NULL, sizeof(buffer));
	aposaui_format_duration(1000u, buffer, 0u);
	return 0;
}

static int aposaui_font_backend_round_trips_a_font(void) {
	const AposaUiAllocator allocator = aposaui_stub_allocator();
	AposaUiFont *font = NULL;

	if (aposaui_font_create(&allocator, "Stub", APOSAUI_STUB_FONT_SIZE, &font) != 0 || font == NULL) {
		return 1;
	}
	if (aposaui_font_ascent(font) != APOSAUI_STUB_FONT_SIZE ||
	    aposaui_font_line_height(font) != APOSAUI_STUB_FONT_SIZE + 2u) {
		return 1;
	}
	if (aposaui_font_glyph(font, 'A') == NULL) {
		return 1;
	}
	aposaui_font_destroy(font);
	return 0;
}

static int aposaui_font_create_rejects_bad_arguments(void) {
	const AposaUiAllocator allocator = aposaui_stub_allocator();
	AposaUiFont *font = NULL;

	if (aposaui_font_create(NULL, "Stub", APOSAUI_STUB_FONT_SIZE, &font) == 0) {
		return 1;
	}
	if (aposaui_font_create(&allocator, "Stub", 0u, &font) == 0) {
		return 1;
	}
	if (aposaui_font_create(&allocator, NULL, APOSAUI_STUB_FONT_SIZE, &font) == 0) {
		return 1;
	}
	if (aposaui_font_ascent(NULL) != 0u || aposaui_font_line_height(NULL) != 0u) {
		return 1;
	}
	aposaui_font_destroy(NULL);
	return aposaui_font_glyph(NULL, 'A') != NULL;
}

static int aposaui_font_measure_text_counts_glyphs(void) {
	const AposaUiAllocator allocator = aposaui_stub_allocator();
	AposaUiFont *font = NULL;
	int result = 1;

	if (aposaui_font_create(&allocator, "Stub", APOSAUI_STUB_FONT_SIZE, &font) != 0) {
		return 1;
	}
	if (aposaui_font_measure_text(font, "AB") == APOSAUI_STUB_GLYPH_ADVANCE * 2 &&
	    aposaui_font_measure_text(font, "") == 0 && aposaui_font_measure_text(NULL, "AB") == 0 &&
	    aposaui_font_measure_text(font, NULL) == 0 &&
	    aposaui_font_measure_text(font, "\xe4\xb8\xad") == APOSAUI_STUB_GLYPH_ADVANCE) {
		result = 0;
	}
	aposaui_font_destroy(font);
	return result;
}

static int aposaui_font_draw_text_advances_the_pen_and_blends(void) {
	AposaUiCanvas canvas = aposaui_test_canvas();
	const AposaUiAllocator allocator = aposaui_stub_allocator();
	AposaUiFont *font = NULL;
	int32_t pen = 0;
	int result = 1;

	if (aposaui_font_create(&allocator, "Stub", APOSAUI_STUB_FONT_SIZE, &font) != 0) {
		return 1;
	}
	aposaui_test_reset();
	pen = aposaui_font_draw_text(&canvas, font, 0, (int32_t)APOSAUI_STUB_FONT_SIZE, "AB", 0xffff0000u);
	if (pen != APOSAUI_STUB_GLYPH_ADVANCE * 2) {
		goto cleanup;
	}
	if (aposaui_test_pixel(0u, 0u) != 0xffff0000u || aposaui_test_pixel(1u, 1u) != 0xffff0000u) {
		goto cleanup;
	}
	if (aposaui_test_pixel(2u, 0u) != 0xffff0000u || aposaui_test_pixel(4u, 0u) != 0xff000000u ||
	    aposaui_test_pixel(0u, 2u) != 0xff000000u) {
		goto cleanup;
	}
	if (!aposaui_test_guards_intact()) {
		goto cleanup;
	}
	if (aposaui_font_draw_text(NULL, font, 0, 0, "A", 0xffffffffu) != 0 ||
	    aposaui_font_draw_text(&canvas, NULL, 7, 0, "A", 0xffffffffu) != 7 ||
	    aposaui_font_draw_text(&canvas, font, 8, 0, NULL, 0xffffffffu) != 8) {
		goto cleanup;
	}
	result = 0;

cleanup:
	aposaui_font_destroy(font);
	return result;
}

static int aposaui_text_baseline_centres_one_line(void) {
	const AposaUiAllocator allocator = aposaui_stub_allocator();
	AposaUiFont *font = NULL;
	int result = 1;

	if (aposaui_font_create(&allocator, "Stub", APOSAUI_STUB_FONT_SIZE, &font) != 0) {
		return 1;
	}
	if (aposaui_text_baseline((AposaUiRect){.x = 0, .y = 0, .width = 10, .height = 20}, font) == 11) {
		result = 0;
	}
	aposaui_font_destroy(font);
	return result;
}

#define APOSAUI_TEST_HIT_CAPACITY 8u

static AposaUiHitTable aposaui_test_hit_table(AposaUiHitEntry *storage, uint32_t capacity) {
	AposaUiHitTable table;

	aposaui_hit_table_init(&table, storage, capacity);
	return table;
}

static int aposaui_hit_table_prefers_the_highest_layer(void) {
	AposaUiHitEntry storage[APOSAUI_TEST_HIT_CAPACITY];
	AposaUiHitTable table = aposaui_test_hit_table(storage, APOSAUI_TEST_HIT_CAPACITY);
	const AposaUiRect shared = {.x = 0, .y = 0, .width = 10, .height = 10};

	if (aposaui_hit_table_add(&table, 1, shared, APOSAUI_LAYER_CHROME, true) != 0) {
		return 1;
	}
	if (aposaui_hit_table_add(&table, 2, shared, APOSAUI_LAYER_PANEL, true) != 0) {
		return 1;
	}
	if (aposaui_hit_table_test(&table, 5, 5) != 2) {
		return 1;
	}
	return aposaui_hit_table_test(&table, 20, 20) != APOSAUI_ID_NONE;
}

static int aposaui_hit_table_prefers_the_later_entry_on_a_layer_tie(void) {
	AposaUiHitEntry storage[APOSAUI_TEST_HIT_CAPACITY];
	AposaUiHitTable table = aposaui_test_hit_table(storage, APOSAUI_TEST_HIT_CAPACITY);
	const AposaUiRect shared = {.x = 0, .y = 0, .width = 10, .height = 10};

	(void)aposaui_hit_table_add(&table, 1, shared, APOSAUI_LAYER_CHROME, true);
	(void)aposaui_hit_table_add(&table, 2, shared, APOSAUI_LAYER_CHROME, true);
	return aposaui_hit_table_test(&table, 5, 5) != 2;
}

static int aposaui_hit_table_ignores_disabled_entries(void) {
	AposaUiHitEntry storage[APOSAUI_TEST_HIT_CAPACITY];
	AposaUiHitTable table = aposaui_test_hit_table(storage, APOSAUI_TEST_HIT_CAPACITY);
	const AposaUiRect shared = {.x = 0, .y = 0, .width = 10, .height = 10};

	(void)aposaui_hit_table_add(&table, 1, shared, APOSAUI_LAYER_CHROME, false);
	(void)aposaui_hit_table_add(&table, 2, shared, APOSAUI_LAYER_PANEL, true);
	if (aposaui_hit_table_test(&table, 5, 5) != 2) {
		return 1;
	}
	if (!aposaui_hit_table_set_enabled(&table, 1, true)) {
		return 1;
	}
	if (aposaui_hit_table_test(&table, 5, 5) != 2) {
		return 1;
	}
	if (aposaui_hit_table_set_enabled(&table, 3, true)) {
		return 1;
	}
	return !aposaui_hit_table_set_enabled(&table, 2, false) || aposaui_hit_table_test(&table, 5, 5) != 1;
}

static int aposaui_hit_table_reports_a_full_table(void) {
	AposaUiHitEntry storage[2];
	AposaUiHitTable table = aposaui_test_hit_table(storage, 2u);
	const AposaUiRect rect = {.x = 0, .y = 0, .width = 4, .height = 4};

	if (aposaui_hit_table_add(&table, 1, rect, APOSAUI_LAYER_CHROME, true) != 0) {
		return 1;
	}
	if (aposaui_hit_table_add(&table, 2, rect, APOSAUI_LAYER_CHROME, true) != 0) {
		return 1;
	}
	if (aposaui_hit_table_add(&table, 3, rect, APOSAUI_LAYER_CHROME, true) == 0) {
		return 1;
	}
	if (table.count != 2u) {
		return 1;
	}
	aposaui_hit_table_reset(&table);
	return table.count != 0u || aposaui_hit_table_test(&table, 1, 1) != APOSAUI_ID_NONE;
}

static int aposaui_hit_table_finds_a_recorded_entry(void) {
	AposaUiHitEntry storage[APOSAUI_TEST_HIT_CAPACITY];
	AposaUiHitTable table = aposaui_test_hit_table(storage, APOSAUI_TEST_HIT_CAPACITY);
	const AposaUiHitEntry *entry;
	const AposaUiRect rect = {.x = 3, .y = 4, .width = 5, .height = 6};

	(void)aposaui_hit_table_add(&table, 7, rect, APOSAUI_LAYER_MENU, true);
	entry = aposaui_hit_table_find(&table, 7);
	if (entry == NULL || entry->rect.x != 3 || entry->rect.height != 6 || entry->layer != APOSAUI_LAYER_MENU) {
		return 1;
	}
	if (aposaui_hit_table_find(&table, 8) != NULL) {
		return 1;
	}
	return aposaui_hit_table_init(&table, NULL, 0u), aposaui_hit_table_test(&table, 4, 5) != APOSAUI_ID_NONE;
}

static int aposaui_interaction_activates_only_on_a_matching_release(void) {
	AposaUiInteraction interaction;

	aposaui_interaction_init(&interaction);
	if (!aposaui_interaction_press(&interaction, 5, 10, 20)) {
		return 1;
	}
	if (!interaction.capturing || !aposaui_interaction_is_pressed(&interaction, 5)) {
		return 1;
	}
	if (aposaui_interaction_release(&interaction, 6) != APOSAUI_ID_NONE) {
		return 1;
	}
	if (interaction.capturing || aposaui_interaction_is_pressed(&interaction, 5)) {
		return 1;
	}
	if (!aposaui_interaction_press(&interaction, 5, 10, 20)) {
		return 1;
	}
	return aposaui_interaction_release(&interaction, 5) != 5;
}

static int aposaui_interaction_ignores_a_press_on_nothing(void) {
	AposaUiInteraction interaction;

	aposaui_interaction_init(&interaction);
	if (aposaui_interaction_press(&interaction, APOSAUI_ID_NONE, 10, 20)) {
		return 1;
	}
	if (interaction.capturing || aposaui_interaction_release(&interaction, APOSAUI_ID_NONE) != APOSAUI_ID_NONE) {
		return 1;
	}
	return aposaui_interaction_press(NULL, 5, 0, 0);
}

static int aposaui_interaction_records_the_drag_origin(void) {
	AposaUiInteraction interaction;

	aposaui_interaction_init(&interaction);
	(void)aposaui_interaction_press(&interaction, 5, 11, 22);
	if (interaction.drag_origin_x != 11 || interaction.drag_origin_y != 22 || interaction.active != 5) {
		return 1;
	}
	aposaui_interaction_cancel(&interaction);
	if (interaction.active != APOSAUI_ID_NONE || interaction.capturing || interaction.drag_origin_x != 11) {
		return 1;
	}
	return interaction.focused != APOSAUI_ID_NONE;
}

static int aposaui_interaction_tracks_hover(void) {
	AposaUiInteraction interaction;

	aposaui_interaction_init(&interaction);
	aposaui_interaction_hover(&interaction, 9);
	if (!aposaui_interaction_is_hovered(&interaction, 9) || aposaui_interaction_is_hovered(&interaction, 8)) {
		return 1;
	}
	if (aposaui_interaction_is_hovered(&interaction, APOSAUI_ID_NONE) ||
	    aposaui_interaction_is_hovered(NULL, 9)) {
		return 1;
	}
	aposaui_interaction_hover(&interaction, APOSAUI_ID_NONE);
	return aposaui_interaction_is_hovered(&interaction, 9);
}

/*
 * The list and scrollbar need more room than the 16x8 primitive canvas, so they
 * get their own, and an ink counter rather than pixel coordinates: what matters
 * is that a row was drawn, not exactly where the stub glyph landed.
 */
#define APOSAUI_TEST_LIST_WIDTH 48u
#define APOSAUI_TEST_LIST_HEIGHT 24u

static uint32_t aposaui_test_list_storage[APOSAUI_TEST_LEAD +
					   APOSAUI_TEST_LIST_WIDTH * APOSAUI_TEST_LIST_HEIGHT + APOSAUI_TEST_TAIL];

static AposaUiCanvas aposaui_test_list_canvas(void) {
	AposaUiCanvas canvas = {.width = APOSAUI_TEST_LIST_WIDTH,
				.height = APOSAUI_TEST_LIST_HEIGHT,
				.stride = APOSAUI_TEST_LIST_WIDTH * (uint32_t)sizeof(uint32_t),
				.pixels = (uint8_t *)(void *)(aposaui_test_list_storage + APOSAUI_TEST_LEAD)};

	return canvas;
}

static void aposaui_test_list_reset(void) {
	const size_t total = sizeof(aposaui_test_list_storage) / sizeof(aposaui_test_list_storage[0]);

	for (size_t index = 0u; index < total; ++index) {
		aposaui_test_list_storage[index] = APOSAUI_TEST_GUARD;
	}
	for (size_t index = 0u; index < APOSAUI_TEST_LIST_WIDTH * APOSAUI_TEST_LIST_HEIGHT; ++index) {
		aposaui_test_list_storage[APOSAUI_TEST_LEAD + index] = 0xff000000u;
	}
}

static size_t aposaui_test_list_ink(void) {
	size_t count = 0u;

	for (size_t index = 0u; index < APOSAUI_TEST_LIST_WIDTH * APOSAUI_TEST_LIST_HEIGHT; ++index) {
		if (aposaui_test_list_storage[APOSAUI_TEST_LEAD + index] != 0xff000000u) {
			++count;
		}
	}
	return count;
}

static bool aposaui_test_list_source(void *user, uint32_t index, AposaUiListRow *out_row) {
	static const char *const labels[] = {"Open File", "Zoom In"};
	static const char *const values[] = {"Ctrl+O", "Ctrl++"};
	const uint32_t count = *(const uint32_t *)user;

	if (index >= count) {
		return false;
	}
	out_row->label = labels[index];
	out_row->value = values[index];
	return true;
}

static int aposaui_list_paint_draws_rows_and_value_badges(void) {
	AposaUiCanvas canvas = aposaui_test_list_canvas();
	const AposaUiAllocator allocator = aposaui_stub_allocator();
	const AposaUiThemeColors colors = aposaui_theme_colors(APOSAUI_THEME_MODE_DARK);
	const AposaUiRect list = {.x = 0, .y = 0, .width = 48, .height = 20};
	AposaUiFont *font = NULL;
	uint32_t row_count = 2u;
	const AposaUiListModel model = {.user = &row_count, .row_count = 2u, .get_row = aposaui_test_list_source};
	size_t with_rows;
	size_t without_rows;
	int result = 1;

	if (aposaui_font_create(&allocator, "Stub", APOSAUI_STUB_FONT_SIZE, &font) != 0) {
		return 1;
	}
	aposaui_test_list_reset();
	aposaui_list_paint(&canvas, list, 6, &model, 0u, 2u, font, &colors, NULL);
	with_rows = aposaui_test_list_ink();
	if (with_rows == 0u) {
		goto cleanup;
	}
	/* An empty model must leave the canvas alone when there is no placeholder. */
	aposaui_test_list_reset();
	aposaui_list_paint(&canvas, list, 6, NULL, 0u, 2u, font, &colors, NULL);
	without_rows = aposaui_test_list_ink();
	if (without_rows != 0u) {
		goto cleanup;
	}
	/* And draw the placeholder when there is one. */
	aposaui_list_paint(&canvas, list, 6, NULL, 0u, 2u, font, &colors, "No keyboard shortcuts configured.");
	if (aposaui_test_list_ink() == 0u) {
		goto cleanup;
	}
	/* Scrolling past the end must not draw rows. */
	aposaui_test_list_reset();
	aposaui_list_paint(&canvas, list, 6, &model, 2u, 2u, font, &colors, NULL);
	if (aposaui_test_list_ink() != 0u) {
		goto cleanup;
	}
	result = 0;

cleanup:
	aposaui_font_destroy(font);
	return result;
}

static int aposaui_scrollbar_paint_draws_track_and_thumb(void) {
	AposaUiCanvas canvas = aposaui_test_list_canvas();
	const AposaUiThemeColors colors = aposaui_theme_colors(APOSAUI_THEME_MODE_DARK);
	const AposaUiRect track = {.x = 40, .y = 0, .width = 6, .height = 20};
	const AposaUiRect thumb = {.x = 40, .y = 2, .width = 6, .height = 8};

	aposaui_test_list_reset();
	aposaui_scrollbar_paint(&canvas, track, thumb, false, false, &colors);
	if (aposaui_test_list_ink() == 0u) {
		return 1;
	}
	if (aposaui_test_list_storage[APOSAUI_TEST_LEAD + 0u * APOSAUI_TEST_LIST_WIDTH + 40u] !=
	    0xff000000u) {
		return 1;
	}
	return 0;
}

int main(void) {
	const struct {
		const char *name;
		int (*function)(void);
	} tests[] = {
	    {"aposaui_rect_contains_matches_the_half_open_rectangle",
	     aposaui_rect_contains_matches_the_half_open_rectangle},
	    {"aposaui_rect_inset_shrinks_each_edge", aposaui_rect_inset_shrinks_each_edge},
	    {"aposaui_paint_fill_rect_clips_to_canvas", aposaui_paint_fill_rect_clips_to_canvas},
	    {"aposaui_paint_fill_rect_ignores_the_colour_alpha", aposaui_paint_fill_rect_ignores_the_colour_alpha},
	    {"aposaui_paint_blend_rect_is_alpha_correct", aposaui_paint_blend_rect_is_alpha_correct},
	    {"aposaui_paint_blend_rect_ignores_a_transparent_colour",
	     aposaui_paint_blend_rect_ignores_a_transparent_colour},
	    {"aposaui_paint_rounded_rect_leaves_corners_clear", aposaui_paint_rounded_rect_leaves_corners_clear},
	    {"aposaui_paint_disc_clips_to_canvas", aposaui_paint_disc_clips_to_canvas},
	    {"aposaui_metrics_scale_with_dpi", aposaui_metrics_scale_with_dpi},
	    {"aposaui_metrics_treat_zero_dpi_as_the_base", aposaui_metrics_treat_zero_dpi_as_the_base},
	    {"aposaui_metrics_keep_a_visible_separator", aposaui_metrics_keep_a_visible_separator},
	    {"aposaui_stroke_width_is_never_below_one", aposaui_stroke_width_is_never_below_one},
	    {"aposaui_theme_colors_separate_the_two_modes", aposaui_theme_colors_separate_the_two_modes},
	    {"aposaui_format_duration_renders_minutes_and_seconds",
	     aposaui_format_duration_renders_minutes_and_seconds},
	    {"aposaui_format_dimensions_uses_the_multiplication_sign",
	     aposaui_format_dimensions_uses_the_multiplication_sign},
	    {"aposaui_format_percent_rounds_to_whole_percent", aposaui_format_percent_rounds_to_whole_percent},
	    {"aposaui_format_helpers_ignore_a_null_buffer", aposaui_format_helpers_ignore_a_null_buffer},
	    {"aposaui_font_backend_round_trips_a_font", aposaui_font_backend_round_trips_a_font},
	    {"aposaui_font_create_rejects_bad_arguments", aposaui_font_create_rejects_bad_arguments},
	    {"aposaui_font_measure_text_counts_glyphs", aposaui_font_measure_text_counts_glyphs},
	    {"aposaui_font_draw_text_advances_the_pen_and_blends",
	     aposaui_font_draw_text_advances_the_pen_and_blends},
	    {"aposaui_text_baseline_centres_one_line", aposaui_text_baseline_centres_one_line},
	    {"aposaui_hit_table_prefers_the_highest_layer", aposaui_hit_table_prefers_the_highest_layer},
	    {"aposaui_hit_table_prefers_the_later_entry_on_a_layer_tie",
	     aposaui_hit_table_prefers_the_later_entry_on_a_layer_tie},
	    {"aposaui_hit_table_ignores_disabled_entries", aposaui_hit_table_ignores_disabled_entries},
	    {"aposaui_hit_table_reports_a_full_table", aposaui_hit_table_reports_a_full_table},
	    {"aposaui_hit_table_finds_a_recorded_entry", aposaui_hit_table_finds_a_recorded_entry},
	    {"aposaui_interaction_activates_only_on_a_matching_release",
	     aposaui_interaction_activates_only_on_a_matching_release},
	    {"aposaui_interaction_ignores_a_press_on_nothing", aposaui_interaction_ignores_a_press_on_nothing},
	    {"aposaui_interaction_records_the_drag_origin", aposaui_interaction_records_the_drag_origin},
	    {"aposaui_interaction_tracks_hover", aposaui_interaction_tracks_hover},
	    {"aposaui_list_paint_draws_rows_and_value_badges", aposaui_list_paint_draws_rows_and_value_badges},
	    {"aposaui_scrollbar_paint_draws_track_and_thumb", aposaui_scrollbar_paint_draws_track_and_thumb},
	};

	for (size_t test_index = 0u; test_index < sizeof(tests) / sizeof(tests[0]); ++test_index) {
		if (tests[test_index].function() != 0) {
			fprintf(stderr, "FAILED: %s\n", tests[test_index].name);
			return 1;
		}
	}
	puts("aposaui tests passed");
	return 0;
}
