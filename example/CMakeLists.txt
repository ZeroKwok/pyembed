cmake_policy(SET CMP0074 NEW)
cmake_minimum_required(VERSION 3.13)

project(example_for_pymebed)

#add_executable(${PROJECT_NAME} embedding.cpp extending.cpp)
add_executable(${PROJECT_NAME} example.cpp)

target_link_libraries(${PROJECT_NAME} pymebed)
