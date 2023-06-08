cmake_policy(SET CMP0074 NEW)
cmake_minimum_required(VERSION 3.13)

add_executable(simple_for_pyembed simple.cpp)
target_link_libraries(simple_for_pyembed pyembed)

add_executable(extending_for_pyembed extending.cpp)
target_link_libraries(extending_for_pyembed pyembed)

add_executable(embedding_for_pyembed embedding.cpp)
target_link_libraries(embedding_for_pyembed pyembed)