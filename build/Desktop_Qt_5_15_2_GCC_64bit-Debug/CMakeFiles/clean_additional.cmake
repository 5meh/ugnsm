# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles/ugnsm_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/ugnsm_autogen.dir/ParseCache.txt"
  "ugnsm_autogen"
  )
endif()
