#
# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.
#

cmake_minimum_required(VERSION 3.25 FATAL_ERROR)

project(amqp-value-parser C)

find_package(Proton 0.37 QUIET)
if (NOT TARGET Proton::core)
    find_package(PkgConfig)
    pkg_check_modules(QPID_PROTON QUIET libqpid-proton-core)
    find_path(QPID_PROTON_INCLUDE_DIRS HINTS /usr/include NAMES proton/codec.h)
    find_library(QPID_PROTON_LIBRARIES NAMES qpid-proton-core)
    message(STATUS "QPID_PROTON_INCLUDE_DIRS: ${QPID_PROTON_INCLUDE_DIRS}")
    message(STATUS "QPID_PROTON_LIBRARIES: ${QPID_PROTON_LIBRARIES}")
    add_library(Proton::core INTERFACE IMPORTED)
    if (NOT QPID_PROTON_INCLUDE_DIRS)
        target_include_directories(Proton::core INTERFACE "${QPID_PROTON_INCLUDE_DIRS}")
    endif()
    if (QPID_PROTON_LIBRARIES)
        target_link_libraries(Proton::core INTERFACE "${QPID_PROTON_LIBRARIES}")
    endif()

    if (NOT QPID_PROTON_INCLUDE_DIRS OR NOT QPID_PROTON_LIBRARIES)
        message(STATUS "Unable to find Proton library or headers")
    endif()
endif()

add_compile_options(-Wall)

add_custom_command (OUTPUT amqp-value.c amqp-value.h amqp-value.out
                    DEPENDS amqp-value.lemon amqp-performatives.h amqp-types.h amqp-value.re.c
                    COMMAND lemon -d${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR}/amqp-value.lemon)

add_custom_command (OUTPUT amqp-performatives.h
                    DEPENDS performatives.gperf
                    COMMAND gperf --output-file amqp-performatives.h ${CMAKE_SOURCE_DIR}/performatives.gperf)

add_custom_command (OUTPUT amqp-types.h
                    DEPENDS types.gperf
                    COMMAND gperf --output-file amqp-types.h ${CMAKE_SOURCE_DIR}/types.gperf)

add_custom_command (OUTPUT amqp-value.re.c
                    DEPENDS amqp-value.re
                    COMMAND re2c -o amqp-value.re.c ${CMAKE_SOURCE_DIR}/amqp-value.re)

add_library (amqp-value STATIC
             amqp-value.c)
target_link_libraries (amqp-value PRIVATE Proton::core)
target_include_directories(amqp-value PRIVATE ${CMAKE_SOURCE_DIR})

add_executable (amqp-value-test main.c)
target_link_libraries (amqp-value-test PRIVATE amqp-value Proton::core)

add_executable (frame-dump-tester frame_dump_tester.c)
target_link_libraries (frame-dump-tester PRIVATE amqp-value Proton::core readline)

install (TARGETS amqp-value-test
         RUNTIME DESTINATION "bin"
         LIBRARY DESTINATION "lib"
         ARCHIVE DESTINATION "lib")
