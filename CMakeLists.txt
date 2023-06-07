# This file is part of the pyembed distribution.
# Copyright (c) 2018-2023 Zero Kwok.
# 
# This is free software; you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation; either version 3 of
# the License, or (at your option) any later version.
# 
# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# Lesser General Public License for more details.
# 
# You should have received a copy of the GNU Lesser General Public
# License along with this software; 
# If not, see <http://www.gnu.org/licenses/>.
#
# Author:  Zero Kwok
# Contact: zero.kwok@foxmail.com 
# 

cmake_policy(SET CMP0074 NEW)
cmake_minimum_required(VERSION 3.13)

project(pyembed VERSION 0.1.2)

option(PYEMBED_BUILD_EXAMPLE    "Compile the example" ON)
option(PYEMBED_STATIC_RUNTIME   "Enable static linking with runtime" OFF)
option(PYEMBED_BUILD_SHARED_LIB "Build the shared library" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 设置输出位置
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG          ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE        ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG          ${CMAKE_SOURCE_DIR}/bin/libs)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE        ${CMAKE_SOURCE_DIR}/bin/libs)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_SOURCE_DIR}/bin/libs)

if(MSVC AND PYEMBED_STATIC_RUNTIME)
    foreach(flag_var CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
        string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
    endforeach(flag_var)
endif()

if(MSVC)
    add_compile_options(/utf-8)
endif()

# 强制动态库模式下使用动态库版本的Boost
if (NOT PYEMBED_BUILD_SHARED_LIB)
    set(Boost_USE_STATIC_LIBS ON)
endif()

add_compile_definitions(BOOST_PYTHON_STATIC_LIB=1)
if (UNIX)
    find_package(Boost REQUIRED COMPONENTS filesystem python)
else()
    add_compile_definitions(BOOST_LIB_DIAGNOSTIC=1)
    find_package(Boost REQUIRED COMPONENTS filesystem)
endif()
find_package(Python3 REQUIRED COMPONENTS Development)

# 调试输出
message(Python3_INCLUDE_DIRS "= ${Python3_INCLUDE_DIRS}")
message(Python3_LIBRARY_DIRS "= ${Python3_LIBRARY_DIRS}")
message(Python3_LIBRARIES "= ${Python3_LIBRARIES}")
message(Python3_Development_FOUND "= ${Python3_Development_FOUND}")

include_directories(${Boost_INCLUDE_DIRS})
link_directories   (${Boost_LIBRARY_DIRS})

file(GLOB INCLUDE_FILES 
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp")

file(GLOB SOURCE_FILES 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/utility/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/utility/*.hpp")

if (PYEMBED_BUILD_SHARED_LIB)
    # 如果采用动态库方式，则Boost也必须采用动态库，否则
    #   boost::python::detail::exception_handler等静态成员变量可能存在多个实例
    #   导致异常处理失效
    add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} ${INCLUDE_FILES})
    target_compile_definitions(${PROJECT_NAME} PRIVATE PYEMBED_SHARED_LIB)
else()
    add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES} ${INCLUDE_FILES})
    target_compile_definitions(${PROJECT_NAME} PUBLIC PYEMBED_STATIC_LIB)
endif()

# pyconfig.h 存在自动连接 pragma comment(lib,"python38.lib")
target_include_directories(${PROJECT_NAME} PUBLIC ${Python3_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME} PUBLIC ${Python3_LIBRARY_DIRS})

# 不能连接CMake目标, 否则将导致: 
#   Assertion failed: !PyErr_Occurred(), file D:\a\1\s\Objects\typeobject.c, line 3129
#target_link_libraries(${PROJECT_NAME} PUBLIC Python3::Python)

if(UNIX)
    target_link_libraries(${PROJECT_NAME} PUBLIC 
        Python3::Python 
        Boost::filesystem
        Boost::python)
endif()

set(CMAKE_MODULE_PATH "${utility_SOURCE_DIR}/cmake")
set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "d")
set_target_properties(${PROJECT_NAME} PROPERTIES 
    PDB_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    COMPILE_PDB_NAME_DEBUG  "${PROJECT_NAME}d"
    COMPILE_PDB_NAME_RELEASE ${PROJECT_NAME}
    COMPILE_PDB_NAME_MINSIZEREL ${PROJECT_NAME}
    COMPILE_PDB_NAME_RELWITHDEBINFO ${PROJECT_NAME})
target_compile_definitions(${PROJECT_NAME} PUBLIC  HAVE_SNPRINTF)
target_compile_definitions(${PROJECT_NAME} PRIVATE UTILITY_SUPPORT_BOOST)
target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>)
set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${INCLUDE_FILES}")

install(EXPORT ${PROJECT_NAME} FILE ${PROJECT_NAME}Config.cmake DESTINATION cmake)
install(TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib 
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include)

get_target_property(PROJECT_PDB_NAME_DEBUG   ${PROJECT_NAME} COMPILE_PDB_NAME_DEBUG)
get_target_property(PROJECT_PDB_NAME_RELEASE ${PROJECT_NAME} COMPILE_PDB_NAME_RELEASE)
get_target_property(PROJECT_PDB_DIRECTORY    ${PROJECT_NAME} PDB_OUTPUT_DIRECTORY)
set(PROJECT_PDB_NAME "$<$<CONFIG:Debug>:${PROJECT_PDB_NAME_DEBUG}>$<$<NOT:$<CONFIG:Debug>>:${PROJECT_PDB_NAME_RELEASE}>.pdb")
install(FILES "${PROJECT_PDB_DIRECTORY}/${PROJECT_PDB_NAME}" DESTINATION lib OPTIONAL)
install(FILES "${PROJECT_PDB_DIRECTORY}/\${CMAKE_INSTALL_CONFIG_NAME}/${PROJECT_PDB_NAME}" DESTINATION lib OPTIONAL)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE DESTINATION ./ OPTIONAL)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/README.md DESTINATION ./ OPTIONAL)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/examples DESTINATION ./)

if(PYEMBED_BUILD_EXAMPLE)
    add_subdirectory(examples)
endif()