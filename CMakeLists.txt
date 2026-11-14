cmake_minimum_required(VERSION 3.25)

project(
  battleship
  VERSION 2.0.0
  DESCRIPTION "Battleship (Bataille Navale) in C11 and GTK 3"
  HOMEPAGE_URL "https://github.com/DmytroPalahin/Battleship"
  LANGUAGES C)

# ---------------------------------------------------------------- options ---
option(BATTLESHIP_BUILD_TESTS "Build the test-suite" ON)
option(BATTLESHIP_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)
option(BATTLESHIP_ENABLE_SANITIZERS "Instrument with ASan and UBSan" OFF)
option(BATTLESHIP_ENABLE_COVERAGE "Instrument for code coverage" OFF)

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

if(BATTLESHIP_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
