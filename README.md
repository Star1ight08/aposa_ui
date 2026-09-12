# aposaui

A single-header Win32 UI toolkit: a software-rasterized canvas, theme and DPI
metrics, a font interface with UTF-8 text rendering, a hit table and interaction
state, a set of general widgets, and the Win32 side of things — message
normalization and a window host.

Written in C23 with no external dependencies beyond the C standard library and
the Win32 API it wraps.

## Getting started

Expand the implementation in exactly one translation unit:

```c
#define APOSAUI_IMPLEMENTATION
#include "aposaui.h"
```

Everywhere else plain inclusion gives you the declarations:

```c
#include "aposaui.h"
```

A smallest useful program:

```c
static LRESULT CALLBACK window_proc(HWND window, UINT message, WPARAM w_param, LPARAM l_param);

int main(void) {
	const AposaUiAllocator allocator = {.user = NULL, .allocate = my_allocate, .deallocate = my_deallocate};
	const AposaUiHostDesc desc = {.title = "Example",
				      .primary_font_family = "Segoe UI",
				      .numeric_font_family = "Consolas",
				      .primary_font_size = 13u,
				      .numeric_font_size = 12u,
				      .client_width = 1280,
				      .client_height = 900,
				      .window_proc = window_proc,
				      .user = NULL};
	AposaUiHost *host = NULL;
	MSG message;

	if (aposaui_host_create(&desc, &allocator, &host) != 0) {
		return 1;
	}
	ShowWindow((HWND)aposaui_host_window(host), SW_SHOWDEFAULT);
	while (GetMessageW(&message, NULL, 0u, 0u) > 0) {
		TranslateMessage(&message);
		DispatchMessageW(&message);
	}
	aposaui_host_destroy(host);
	return 0;
}
```

## How it is split

| | Contents | Under `APOSAUI_NO_PLATFORM` |
|---|---|---|
| Portable | Canvas and drawing primitives, theme and DPI metrics, the font interface, UTF-8 text rendering, the hit table and interaction state, and the widgets: icons, control backgrounds, badges, menus, scrollbars, virtualized lists, the animation timeline | compiled |
| Platform | GDI font backend, message-to-event translation, and the host: window class, window, fonts, DIB presentation, dark title bar | not compiled |

The library deliberately does **not** own the message loop, the window
procedure, or the paint policy. Those are where applications genuinely differ,
and taking them over would only force every caller through an intermediary.

## Configuration

| Macro | Effect |
|---|---|
| `APOSAUI_IMPLEMENTATION` | Expands the implementation. It sits **after** the include guard's `#endif`, so an include for declarations followed later by an include for the implementation both take effect |
| `APOSAUI_NO_PLATFORM` | Compiles the portable half only. No `windows.h`, and no user32 or gdi32 at link time |
| `APOSAUI_STATIC` | Makes `APOSAUI_DEF` expand to `static`, keeping the implementation private to the translation unit that instantiates it |

## Conventions

**Naming.** Public symbols are `aposaui_`; private functions and file-scope
objects use a double underscore, `aposaui__`. The latter exists so that a caller
who writes a wrapper named `aposaui_something` cannot collide with a private
helper added later. Please do not use a single underscore for new internals.

**No allocation.** The library never calls `malloc` or `free`. Anything needing
dynamic storage takes a caller-supplied `AposaUiAllocator`, and the hit table's
storage is supplied by the caller too (`aposaui_hit_table_init` takes an array
and a capacity).

**Text is UTF-8.** Everywhere the library draws text. Wide strings appear only
where the underlying Win32 API is intrinsically wide, such as `WM_DROPFILES` or
`FindFirstFileW`.

**Geometry is registered while painting.** Widgets record their hit rectangles in
an `AposaUiHitTable` as they draw, rather than laying out and then hit testing
separately. Each rectangle is computed once, so the two can never drift apart.

## The font backend is the extension point

The library declares `aposaui_font_create`, `_destroy`, `_ascent`, `_line_height`
and `_glyph`. On Win32 the GDI backend implements them. Under
`APOSAUI_NO_PLATFORM` the caller supplies them instead — see
[`tests/aposaui_stub_font.c`](tests/aposaui_stub_font.c), which doubles as
documentation for the interface: every codepoint maps to one solid square glyph,
so text layout and blending can be verified pixel by pixel.

`AposaUiFont` is opaque; the backend owns its layout.

## Using it

A frame goes: paint, which registers hits, then hand the hit to the interaction
state.

```c
AposaUiCanvas canvas = {.width = w, .height = h, .stride = w * 4u, .pixels = pixels};
AposaUiThemeColors colors = aposaui_theme_colors(APOSAUI_THEME_MODE_DARK);

aposaui_hit_table_reset(&hits);
aposaui_control_background(&canvas, rect, 6.0, false, hovered, pressed, &colors);
aposaui_icon_paint(&canvas, APOSAUI_ICON_ZOOM_IN, rect, &metrics, colors.text_primary);

AposaUiId activated = aposaui_interaction_release(&interaction, aposaui_hit_table_test(&hits, x, y));
```

`aposaui_hit_table_test` returns the entry with the **greatest layer**, and a
later entry wins a tie. See `APOSAUI_LAYER_CHROME`, `_MENU`, `_PANEL` and
`_OVERLAY`.

Input arrives as normalized events rather than every caller decoding message
parameters itself:

```c
AposaUiEvent event;
(void)aposaui_event_from_message(window, message, w_param, l_param, &event);
if (event.type == APOSAUI_EVENT_KEY) {
	int command = lookup(event.virtual_key, event.modifiers);
	if (!event.is_repeat) { ... }
}
```

## Building and testing

```sh
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

On Windows an extra `aposaui_platform` library is compiled, which checks that
the GDI backend, the event translation and the host are self-contained. The test
target defines `APOSAUI_NO_PLATFORM`, so the binary it produces links without
user32 or gdi32 — a property you can confirm with `dumpbin /dependents`.

## License

Apache License 2.0 — see [LICENSE](LICENSE).

It is permissive: use, modification and redistribution are allowed, including
commercially. In return it asks that you keep the copyright and license notices
intact and state which files you changed. Unlike MIT it also grants an express
patent licence from contributors, which is the usual reason to pick it for
anything a patent could touch.
