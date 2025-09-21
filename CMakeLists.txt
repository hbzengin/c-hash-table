cmake_minimum_required(VERSION 3.28)
project(hash_table LANGUAGES C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED YES)
set(CMAKE_C_EXTENSIONS OFF)

add_library(hash_table_lib
  src/hash_table.c
  src/util.c
)
target_include_directories(hash_table_lib PUBLIC src)

add_executable(hash_table
  main.c          
  tests/test.c
)

target_include_directories(hash_table PRIVATE src tests)
target_link_libraries(hash_table PRIVATE hash_table_lib)
