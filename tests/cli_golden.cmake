# Compares a seeded simulation with the histogram recorded in the repository.
# Usage: cmake -DCLI=<battleship-cli> -DGOLDEN=<csv file> -P cli_golden.cmake
#
# The engine uses integer arithmetic only, so the result must be identical on
# every compiler, operating system and architecture. A difference means that
# the random generator, the fleet placement or a strategy changed behaviour:
# either a bug, or a deliberate change that must come with a new golden file
#
#     battleship-cli simulate --csv --games 300 --seed 2023 > tests/data/simulate_seed2023.csv

execute_process(
  COMMAND "${CLI}" simulate --csv --games 300 --seed 2023
  OUTPUT_VARIABLE actual
  RESULT_VARIABLE result)
if(NOT result EQUAL 0)
  message(FATAL_ERROR "simulate exited with status ${result}")
endif()

file(READ "${GOLDEN}" expected)
string(REPLACE "\r\n" "\n" actual "${actual}")
string(REPLACE "\r\n" "\n" expected "${expected}")

if(NOT actual STREQUAL expected)
  message(FATAL_ERROR "the simulation no longer matches ${GOLDEN}\n"
                      "If the change of behaviour is intended, regenerate the file.")
endif()
