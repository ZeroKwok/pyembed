cmake_policy(SET CMP0074 NEW)
cmake_minimum_required(VERSION 3.13)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG          ${CMAKE_CURRENT_SOURCE_DIR}/bin/python)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE        ${CMAKE_CURRENT_SOURCE_DIR}/bin/python)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_CURRENT_SOURCE_DIR}/bin/python)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG          ${CMAKE_CURRENT_SOURCE_DIR}/bin/python)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE        ${CMAKE_CURRENT_SOURCE_DIR}/bin/python)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_CURRENT_SOURCE_DIR}/bin/python)

project(example_for_pymebed)

#add_executable(${PROJECT_NAME} embedding.cpp extending.cpp)
add_executable(${PROJECT_NAME} example.cpp)

target_link_libraries(${PROJECT_NAME} pymebed)
