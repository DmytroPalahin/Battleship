# Engine tests. Each file is a stand-alone executable built on tests/bs_test.h.

function(battleship_add_test name)
  add_executable(${name} ${name}.c)
  target_link_libraries(${name} PRIVATE battleship::core battleship_build_flags)
  target_include_directories(${name} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
  add_test(NAME ${name} COMMAND ${name})
  set_tests_properties(${name} PROPERTIES LABELS "unit" TIMEOUT 120)
endfunction()

battleship_add_test(test_rng)
battleship_add_test(test_board)
battleship_add_test(test_ai)
battleship_add_test(test_game)
battleship_add_test(test_simulation)
set_tests_properties(test_simulation PROPERTIES LABELS "unit;property")
