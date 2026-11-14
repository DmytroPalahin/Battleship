# The GTK 3 application: view, controller and application shell.

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED IMPORTED_TARGET gtk+-3.0>=3.22)
find_program(GLIB_COMPILE_RESOURCES NAMES glib-compile-resources REQUIRED)

# ------------------------------------------------------------------ audio ---
# Sound is an optional capability: AUTO enables it when GStreamer is present.
set(_bs_audio OFF)
if(NOT BATTLESHIP_ENABLE_AUDIO STREQUAL "OFF")
  pkg_check_modules(GSTREAMER IMPORTED_TARGET gstreamer-1.0>=1.14)
  if(GSTREAMER_FOUND)
    set(_bs_audio ON)
  elseif(BATTLESHIP_ENABLE_AUDIO STREQUAL "ON")
    message(FATAL_ERROR "BATTLESHIP_ENABLE_AUDIO=ON but gstreamer-1.0 was not found")
  endif()
endif()
message(STATUS "Battleship sound effects: ${_bs_audio}")

# -------------------------------------------------------------- resources ---
# Images, sounds and the style sheet are compiled into the executable.
set(_bs_assets_dir "${PROJECT_SOURCE_DIR}/assets")
set(_bs_resource_files
    style.css
    images/background-game.jpg
    images/background-menu.jpg
    images/background-rules.jpg
    images/background-trailer.jpg
    images/explosion.png
    images/flag.png
    images/miss.png
    images/rocket.png
    images/ship.png
    images/ship-sunk-enemy.png
    images/ship-sunk-player.png
    images/versus.png)
set(BATTLESHIP_SOUND_RESOURCES "")
if(_bs_audio)
  list(APPEND _bs_resource_files sounds/hit.wav sounds/miss.wav)
  set(BATTLESHIP_SOUND_RESOURCES
      "\n    <file>sounds/hit.wav</file>\n    <file>sounds/miss.wav</file>")
endif()

set(_bs_resource_xml "${CMAKE_CURRENT_BINARY_DIR}/battleship.gresource.xml")
set(_bs_resource_c "${CMAKE_CURRENT_BINARY_DIR}/battleship-resources.c")
configure_file(battleship.gresource.xml.in "${_bs_resource_xml}" @ONLY)

list(TRANSFORM _bs_resource_files PREPEND "${_bs_assets_dir}/" OUTPUT_VARIABLE _bs_resource_deps)
add_custom_command(
  OUTPUT "${_bs_resource_c}"
  COMMAND "${GLIB_COMPILE_RESOURCES}" --generate-source "--sourcedir=${_bs_assets_dir}"
          "--target=${_bs_resource_c}" "${_bs_resource_xml}"
  DEPENDS "${_bs_resource_xml}" ${_bs_resource_deps}
  COMMENT "Bundling images, sounds and style sheet"
  VERBATIM)

# Generated code: built on its own so that the project warnings do not apply.
add_library(battleship_resources OBJECT "${_bs_resource_c}")
target_link_libraries(battleship_resources PRIVATE PkgConfig::GTK3)

# ------------------------------------------------------------- executable ---
add_executable(battleship main.c app.c controller.c view.c
                          $<TARGET_OBJECTS:battleship_resources>)
target_link_libraries(battleship PRIVATE battleship::core PkgConfig::GTK3
                                         battleship_build_flags)

if(_bs_audio)
  target_sources(battleship PRIVATE audio_gst.c)
  target_link_libraries(battleship PRIVATE PkgConfig::GSTREAMER)
else()
  target_sources(battleship PRIVATE audio_null.c)
endif()

# --------------------------------------------------------------- install ---
install(TARGETS battleship RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT gui)
if(UNIX AND NOT APPLE)
  set(_bs_packaging "${PROJECT_SOURCE_DIR}/packaging")
  install(FILES "${_bs_packaging}/io.github.dmytropalahin.Battleship.desktop"
          DESTINATION "${CMAKE_INSTALL_DATADIR}/applications")
  install(FILES "${_bs_packaging}/io.github.dmytropalahin.Battleship.metainfo.xml"
          DESTINATION "${CMAKE_INSTALL_DATADIR}/metainfo")
  install(FILES "${_bs_packaging}/io.github.dmytropalahin.Battleship.png"
          DESTINATION "${CMAKE_INSTALL_DATADIR}/icons/hicolor/256x256/apps")
endif()
