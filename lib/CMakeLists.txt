# lib/CMakeLists.txt

include_directories (
  ${CMAKE_HOME_DIRECTORY}/lib
)

file (GLOB header_files
  "*.h" "*/*.h" "*/*/*.h"
)

file (GLOB source_files
  "*.cpp" "*/*.cpp" "*/*/*.cpp"
)

add_library (STeCaLib
  ${source_files}
  ${header_files}
)

target_link_libraries (STeCaLib
  Qt5::Core
)

# eof
