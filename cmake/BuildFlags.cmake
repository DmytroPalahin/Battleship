# Build flags shared by every target of the project.
#
# Targets opt in with:  target_link_libraries(<target> PRIVATE battleship_build_flags)
#
# Keeping the flags on an INTERFACE target (instead of CMAKE_C_FLAGS) means they
# never leak into third-party code and can be read in one place.

add_library(battleship_build_flags INTERFACE)

# ---------------------------------------------------------------- warnings ---
if(MSVC)
  target_compile_options(battleship_build_flags INTERFACE /W4 /permissive-)
else()
  target_compile_options(
    battleship_build_flags
    INTERFACE -Wall
              -Wextra
              -Wpedantic
              -Wshadow
              -Wconversion
              -Wsign-conversion
              -Wcast-qual
              -Wformat=2
              -Wundef
              -Wvla
              -Wdouble-promotion
              -Wstrict-prototypes
              -Wmissing-prototypes
              -Wold-style-definition)
endif()

if(BATTLESHIP_WARNINGS_AS_ERRORS)
  set(CMAKE_COMPILE_WARNING_AS_ERROR ON)
endif()

# -------------------------------------------------------------- sanitizers ---
if(BATTLESHIP_ENABLE_SANITIZERS)
  if(MSVC)
    message(FATAL_ERROR "BATTLESHIP_ENABLE_SANITIZERS is only supported with GCC and Clang")
  endif()
  set(_bs_sanitizer_flags -fsanitize=address,undefined -fno-sanitize-recover=undefined
                          -fno-omit-frame-pointer)
  target_compile_options(battleship_build_flags INTERFACE ${_bs_sanitizer_flags})
  target_link_options(battleship_build_flags INTERFACE ${_bs_sanitizer_flags})
endif()

# ---------------------------------------------------------------- coverage ---
if(BATTLESHIP_ENABLE_COVERAGE)
  if(MSVC)
    message(FATAL_ERROR "BATTLESHIP_ENABLE_COVERAGE is only supported with GCC and Clang")
  endif()
  target_compile_options(battleship_build_flags INTERFACE --coverage -O0 -g)
  target_link_options(battleship_build_flags INTERFACE --coverage)
endif()
