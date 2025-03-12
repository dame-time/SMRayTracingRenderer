# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles/SMRayTracingRenderer_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/SMRayTracingRenderer_autogen.dir/ParseCache.txt"
  "SMRayTracingRenderer_autogen"
  )
endif()
