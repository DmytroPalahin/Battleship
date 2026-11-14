# The game engine: pure C11, no third-party dependency. This is the "model".

configure_file("${PROJECT_SOURCE_DIR}/cmake/version.h.in"
               "${PROJECT_BINARY_DIR}/generated/battleship/version.h" @ONLY)

add_library(battleship_core STATIC ai.c board.c common.c game.c rng.c)
add_library(battleship::core ALIAS battleship_core)

target_include_directories(
  battleship_core PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
                         "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/generated>")
target_link_libraries(battleship_core PRIVATE battleship_build_flags)
set_target_properties(battleship_core PROPERTIES POSITION_INDEPENDENT_CODE ON OUTPUT_NAME
                                                                              battleship_core)
