cmake_policy(SET CMP0074 NEW)
cmake_minimum_required(VERSION 3.13)

project(pymebed)

option(PYMEBED_BUILD_EXAMPLE "Whether to compile the example project" ON)
option(PYMEBED_STATIC_RUNTIME "Enable link with runtime statically" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC AND PYMEBED_STATIC_RUNTIME)
    foreach(flag_var CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
        string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
    endforeach(flag_var)
endif()

if(MSVC)
    add_compile_options(/utf-8)
endif()

set(Boost_USE_STATIC_LIBS ON)
add_compile_definitions(BOOST_LIB_DIAGNOSTIC=1)
add_compile_definitions(BOOST_PYTHON_STATIC_LIB=1)
add_compile_definitions(AUTO_LINK_LIB_DIAGNOSTIC=1)

#find_package(nlog REQUIRED)
find_package(utility REQUIRED)
find_package(Boost REQUIRED filesystem)
find_package(Python3 REQUIRED COMPONENTS Development)

# 调试输出
message(Python3_INCLUDE_DIRS " ${Python3_INCLUDE_DIRS}")
message(Python3_LIBRARY_DIRS " ${Python3_LIBRARY_DIRS}")
message(Python3_LIBRARIES " ${Python3_LIBRARIES}")
message(Python3_Development_FOUND " ${Python3_Development_FOUND}")

include_directories(${utility_INCLUDE_DIRS})
link_directories   (${utility_LIBRARY_DIRS})
include_directories(${Boost_INCLUDE_DIRS})
link_directories   (${Boost_LIBRARY_DIRS})

file(GLOB INCLUDE_FILES 
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp")

file(GLOB SOURCE_FILES 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES} ${INCLUDE_FILES})

# pyconfig.h 存在自动连接 pragma comment(lib,"python38.lib")
target_include_directories(${PROJECT_NAME} PUBLIC ${Python3_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME} PUBLIC ${Python3_LIBRARY_DIRS})

# 不能连接CMake目标, 否则将导致: 
#   Assertion failed: !PyErr_Occurred(), file D:\a\1\s\Objects\typeobject.c, line 3129
#target_link_libraries(${PROJECT_NAME} PUBLIC Python3::Python)

set(CMAKE_MODULE_PATH "${utility_SOURCE_DIR}/cmake")
set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "d")
set_target_properties(${PROJECT_NAME} PROPERTIES 
    PDB_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    COMPILE_PDB_NAME_DEBUG  "${PROJECT_NAME}d"
    COMPILE_PDB_NAME_RELEASE ${PROJECT_NAME}
    COMPILE_PDB_NAME_MINSIZEREL ${PROJECT_NAME}
    COMPILE_PDB_NAME_RELWITHDEBINFO ${PROJECT_NAME})
target_compile_definitions(${PROJECT_NAME} PRIVATE UTILITY_SUPPORT_BOOST)
target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>)
set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${INCLUDE_FILES}")

include(internal_util)

install(EXPORT ${PROJECT_NAME} FILE ${PROJECT_NAME}Config.cmake DESTINATION cmake)
install(TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib 
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE DESTINATION ./ OPTIONAL)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/README.md DESTINATION ./ OPTIONAL)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/example DESTINATION ./)

install_pdb(${PROJECT_NAME})

if(PYMEBED_BUILD_EXAMPLE)
    add_subdirectory(examples)
endif()