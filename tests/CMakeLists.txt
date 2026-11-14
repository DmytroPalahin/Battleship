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

# --------------------------------------------------------------- CLI tests ---
if(BATTLESHIP_BUILD_CLI)
  add_test(NAME cli_version COMMAND battleship-cli --version)
  set_tests_properties(cli_version PROPERTIES PASS_REGULAR_EXPRESSION "${PROJECT_VERSION}"
                                              LABELS "cli")

  add_test(NAME cli_help COMMAND battleship-cli --help)
  set_tests_properties(cli_help PROPERTIES PASS_REGULAR_EXPRESSION "simulate" LABELS "cli")

  add_test(NAME cli_rejects_unknown_option COMMAND battleship-cli simulate --bogus)
  set_tests_properties(cli_rejects_unknown_option PROPERTIES WILL_FAIL TRUE LABELS "cli")

  # A benchmark run must be byte-for-byte reproducible for a given seed.
  add_test(
    NAME cli_simulation_is_reproducible
    COMMAND
      ${CMAKE_COMMAND} -DCLI=$<TARGET_FILE:battleship-cli>
      -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR} -P
      "${CMAKE_CURRENT_SOURCE_DIR}/cli_reproducible.cmake")
  set_tests_properties(cli_simulation_is_reproducible PROPERTIES LABELS "cli")

  # ... and identical on every platform: compared with a recorded histogram.
  add_test(
    NAME cli_simulation_matches_golden
    COMMAND
      ${CMAKE_COMMAND} -DCLI=$<TARGET_FILE:battleship-cli>
      -DGOLDEN=${CMAKE_CURRENT_SOURCE_DIR}/data/simulate_seed2023.csv -P
      "${CMAKE_CURRENT_SOURCE_DIR}/cli_golden.cmake")
  set_tests_properties(cli_simulation_matches_golden PROPERTIES LABELS "cli")

  # A scripted terminal game: the player quits after a few commands.
  add_test(
    NAME cli_play_scripted
    COMMAND
      ${CMAKE_COMMAND} -DCLI=$<TARGET_FILE:battleship-cli>
      -DINPUT=${CMAKE_CURRENT_SOURCE_DIR}/data/play_session.txt -P
      "${CMAKE_CURRENT_SOURCE_DIR}/cli_play.cmake")
  set_tests_properties(cli_play_scripted PROPERTIES LABELS "cli")
endif()
