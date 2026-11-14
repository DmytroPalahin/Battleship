# Feeds a scripted session to `battleship-cli play` and checks the transcript.
# Usage: cmake -DCLI=<battleship-cli> -DINPUT=<session file> -P cli_play.cmake

execute_process(
  COMMAND "${CLI}" play --seed 7 --level hard
  INPUT_FILE "${INPUT}"
  OUTPUT_VARIABLE output
  RESULT_VARIABLE result)

if(NOT result EQUAL 0)
  message(FATAL_ERROR "play exited with status ${result}\n${output}")
endif()

foreach(expected "level hard, seed 7" "YOUR FLEET" "ENEMY WATERS" "You fire at A1" "Try "
                 "Not understood" "Cannot fire there" "Cannot shuffle")
  string(FIND "${output}" "${expected}" position)
  if(position EQUAL -1)
    message(FATAL_ERROR "expected \"${expected}\" in the transcript:\n${output}")
  endif()
endforeach()
