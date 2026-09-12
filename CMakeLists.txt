cmake_minimum_required(VERSION 3.15)

project(aposaui
	VERSION 0.1.0
	DESCRIPTION "A single-header Win32 UI toolkit"
	LANGUAGES C
)

add_library(aposaui INTERFACE)
target_include_directories(aposaui INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_features(aposaui INTERFACE c_std_23)

option(APOSAUI_BUILD_TESTS "Build the aposaui test suite" ON)
option(APOSAUI_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" ON)

# The platform half pulls in windows.h, gdi32 and user32, so it is only built
# where it means something. Compiling it as a library on its own is the check
# that the GDI backend, the event translation and the host are self-contained.
if(WIN32)
	add_library(aposaui_platform STATIC aposaui_impl.c)
	target_compile_features(aposaui_platform PRIVATE c_std_23)
	if(MSVC)
		target_compile_options(aposaui_platform PRIVATE /W4)
		if(APOSAUI_WARNINGS_AS_ERRORS)
			target_compile_options(aposaui_platform PRIVATE /WX)
		endif()
	else()
		target_compile_options(aposaui_platform PRIVATE -Wall -Wextra -Wpedantic)
		if(APOSAUI_WARNINGS_AS_ERRORS)
			target_compile_options(aposaui_platform PRIVATE -Werror)
		endif()
	endif()
endif()

if(APOSAUI_BUILD_TESTS)
	enable_testing()

	# APOSAUI_NO_PLATFORM compiles the platform half out, so this binary links
	# without user32 or gdi32. The font backend it would otherwise provide comes
	# from tests/aposaui_stub_font.c instead.
	add_executable(aposaui_tests
		tests/aposaui_test.c
		tests/aposaui_stub_font.c
		aposaui_impl.c
	)
	target_link_libraries(aposaui_tests PRIVATE aposaui)
	target_compile_features(aposaui_tests PRIVATE c_std_23)
	target_compile_definitions(aposaui_tests PRIVATE APOSAUI_NO_PLATFORM)
	if(MSVC)
		target_compile_options(aposaui_tests PRIVATE /W4)
		if(APOSAUI_WARNINGS_AS_ERRORS)
			target_compile_options(aposaui_tests PRIVATE /WX)
		endif()
	else()
		target_compile_options(aposaui_tests PRIVATE -Wall -Wextra -Wpedantic)
		if(APOSAUI_WARNINGS_AS_ERRORS)
			target_compile_options(aposaui_tests PRIVATE -Werror)
		endif()
	endif()

	add_test(NAME aposaui_tests COMMAND aposaui_tests)
endif()
