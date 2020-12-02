cmake_minimum_required (VERSION 3.12)

find_package (range-v3 CONFIG REQUIRED)
find_package (fmt REQUIRED)

configure_file(input_file_loader.cpp.in input_file_loader.cpp @ONLY)
add_library(aoc-helper ${CMAKE_CURRENT_BINARY_DIR}/input_file_loader.cpp)
target_include_directories(aoc-helper PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties (
  aoc-helper
  PROPERTIES CXX_STANDARD 20
      CXX_STANDARD_REQUIRED YES
      CXX_EXTENSIONS NO
)

function(aoc2020 number)
  add_executable(day${number} "day${number}.cpp")
  target_link_libraries (day${number} PUBLIC range-v3::range-v3 fmt::fmt aoc-helper)
  set_target_properties (
    day${number}
    PROPERTIES CXX_STANDARD 20
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
  )
endfunction()

aoc2020(1)
aoc2020(2)

