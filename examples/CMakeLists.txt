cmake_policy(SET CMP0074 NEW)
cmake_minimum_required(VERSION 3.13)

add_executable(simple_for_pymebed simple.cpp)
target_link_libraries(simple_for_pymebed pymebed)

add_executable(extending_for_pymebed extending.cpp)
target_link_libraries(extending_for_pymebed pymebed)

add_executable(embedding_for_pymebed embedding.cpp)
target_link_libraries(embedding_for_pymebed pymebed)