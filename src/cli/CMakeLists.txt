# Terminal front-end and AI simulator. Depends on the engine only.

add_executable(battleship-cli main.c play.c simulate.c)
target_link_libraries(battleship-cli PRIVATE battleship::core battleship_build_flags)

# sqrt() lives in libm on most Unix systems.
find_library(BATTLESHIP_MATH_LIBRARY m)
if(BATTLESHIP_MATH_LIBRARY)
  target_link_libraries(battleship-cli PRIVATE "${BATTLESHIP_MATH_LIBRARY}")
endif()

install(TARGETS battleship-cli RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT cli)
