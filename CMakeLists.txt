cmake_minimum_required(VERSION 3.28)
project(hash_table LANGUAGES C)


set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED YES)
set(CMAKE_C_EXTENSIONS OFF)

add_executable(${PROJECT_NAME}
  src/main.c
  src/hash_table.c
  src/util.c
)

target_include_directories(${PROJECT_NAME} PRIVATE src)  # if headers live in src/