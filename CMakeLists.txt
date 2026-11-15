cmake_minimum_required(VERSION 3.25)

project(
  battleship
  VERSION 2.2.1
  DESCRIPTION "Battleship (Bataille Navale) in C11 and GTK 3"
  HOMEPAGE_URL "https://github.com/DmytroPalahin/battleship"
  LANGUAGES C)

# ---------------------------------------------------------------- options ---
option(BATTLESHIP_BUILD_GUI "Build the GTK 3 application" ON)
option(BATTLESHIP_BUILD_CLI "Build the terminal front-end and simulator" ON)
option(BATTLESHIP_BUILD_TESTS "Build the test-suite" ON)
option(BATTLESHIP_BUILD_DOCS "Add a 'docs' target (requires Doxygen)" OFF)
option(BATTLESHIP_GUI_SMOKE_TEST "Register the GUI end-to-end test (needs a display)" OFF)
option(BATTLESHIP_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)
option(BATTLESHIP_ENABLE_SANITIZERS "Instrument with ASan and UBSan" OFF)
option(BATTLESHIP_ENABLE_COVERAGE "Instrument for code coverage" OFF)
set(BATTLESHIP_ENABLE_AUDIO
    "AUTO"
    CACHE STRING "Sound effects through GStreamer: ON, OFF or AUTO (if available)")
set_property(CACHE BATTLESHIP_ENABLE_AUDIO PROPERTY STRINGS AUTO ON OFF)

# --------------------------------------------------------------- language ---
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE
      RelWithDebInfo
      CACHE STRING "Build type" FORCE)
endif()

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
include(GNUInstallDirs)
include(BuildFlags)

# ---------------------------------------------------------------- targets ---
add_subdirectory(src/core)

if(BATTLESHIP_BUILD_CLI)
  add_subdirectory(src/cli)
endif()

if(BATTLESHIP_BUILD_GUI)
  add_subdirectory(src/ui)
endif()

if(BATTLESHIP_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

if(BATTLESHIP_BUILD_DOCS)
  add_subdirectory(docs)
endif()

include(Packaging)
