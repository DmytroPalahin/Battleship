# Runs the simulator twice with the same seed and requires identical output.
# Usage: cmake -DCLI=<battleship-cli> -DWORK_DIR=<dir> -P cli_reproducible.cmake

foreach(run first second)
  execute_process(
    COMMAND "${CLI}" simulate --games 200 --seed 2023
    OUTPUT_VARIABLE output_${run}
    RESULT_VARIABLE result_${run})
  if(NOT result_${run} EQUAL 0)
    message(FATAL_ERROR "simulate exited with status ${result_${run}}")
  endif()
endforeach()

if(NOT output_first STREQUAL output_second)
  file(WRITE "${WORK_DIR}/simulate_first.txt" "${output_first}")
  file(WRITE "${WORK_DIR}/simulate_second.txt" "${output_second}")
  message(FATAL_ERROR "two runs with the same seed differ; see ${WORK_DIR}/simulate_*.txt")
endif()

# A different seed must change the numbers, or the seed is being ignored.
execute_process(COMMAND "${CLI}" simulate --games 200 --seed 2024 OUTPUT_VARIABLE output_other)
if(output_first STREQUAL output_other)
  message(FATAL_ERROR "the seed has no effect on the simulation")
endif()

foreach(level easy medium hard)
  if(NOT output_first MATCHES "${level}")
    message(FATAL_ERROR "level '${level}' is missing from the report:\n${output_first}")
  endif()
endforeach()
