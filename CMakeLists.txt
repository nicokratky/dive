cmake_minimum_required(VERSION 3.6)

project(DiVe VERSION 0.2.0 DESCRIPTION "Distance Vector Routing Protocol Simulation")

set(CMAKE_CXX_STANDARD 14)  # change if necessary

add_definitions(-DASIO_STANDALONE)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR
    "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR
    "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
    add_definitions(-pedantic -Wall -Wextra -Wno-extended-offsetof)  # according to GSL
  # add_definitions(-Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
  # add_definitions(-Wl,--no-as-needed)  # needed for building on buggy configurations
else()  # currently not maintained any more:
    add_definitions(-DASIO_HAS_STD_ADDRESSOF)
    add_definitions(-DASIO_HAS_STD_ARRAY)
    add_definitions(-DASIO_HAS_CSTDINT)
    add_definitions(-DASIO_HAS_STD_SHARED_PTR)
    add_definitions(-DASIO_HAS_STD_TYPE_TRAITS)
    add_definitions(-DASIO_HAS_STD_ATOMIC)
    add_definitions(-D_WIN32_WINNT=0x0501)
    add_definitions(/Wall /EHsc)
endif()

# important: Don't forget to set the environment variables accordingly (if needed)!
# ...and uncomment all unneeded statemenets

# https://github.com/chriskohlhoff/asio
include_directories($ENV{ASIO_INCLUDE_PATH})
message(STATUS "ASIO: $ENV{ASIO_INCLUDE_PATH}")

# https://github.com/gabime/spdlog
include_directories($ENV{SPDLOG_INCLUDE_PATH})
message(STATUS "SPDLOG: $ENV{SPDLOG_INCLUDE_PATH}")

# https://github.com/fmtlib/fmt (http://fmtlib.net)
include_directories($ENV{FMT_PATH})  # should refer to the root directory of fmt
message(STATUS "FMT: $ENV{FMT_PATH}")

# https://github.com/nlohmann/json (https://nlohmann.github.io/json/)
include_directories($ENV{JSON_INCLUDE_PATH})  # should refer to the directory 'src' of json
message(STATUS "JSON: $ENV{JSON_INCLUDE_PATH}")

# https://github.com/muellan/clipp
include_directories($ENV{CLIPP_INCLUDE_PATH})
message(STATUS "CLIPP: $ENV{CLIPP_INCLUDE_PATH}")

set(FMT_LIB_PATH $ENV{FMT_PATH}/build/fmt/libfmt.a)

# only needed for protobuf
# ko:: Be aware: variable names changed in cmake 3.6 from PROTOBUF_* to Protobuf_*
find_package(Threads)
find_package(Protobuf REQUIRED)

# global include dir
include_directories("${CMAKE_SOURCE_DIR}/include/")

include_directories(${Protobuf_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

file(GLOB protos protos/*.proto)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${protos})

macro(create_targets)
    if(${ARGC} GREATER 1)
        foreach(curr_target ${ARGV})
            message(STATUS ${curr_target})

            file(GLOB sources ${CMAKE_SOURCE_DIR}/src/${curr_target}/*.cpp ${CMAKE_SOURCE_DIR}/src/*.cpp)

            include_directories(${CMAKE_SOURCE_DIR}/include/${curr_target}/)

            add_executable(${curr_target} ${sources} ${PROTO_SRCS} ${PROTO_HDRS})
            target_link_libraries(${curr_target} ${CMAKE_THREAD_LIBS_INIT} ${FMT_LIB_PATH} ${PROTOBUF_LIBRARIES})
        endforeach(curr_target)
    else()
        message(${ARGV0})
        if(EXISTS "${CMAKE_SOURCE_DIR}/include/${ARGV0}" AND IS_DIRECTORY "${CMAKE_SOURCE_DIR}/include/${ARGV0}")
            include_directories("${CMAKE_SOURCE_DIR}/include/${ARGV0}")
        endif()

        if(EXISTS "${CMAKE_SOURCE_DIR}/src/${ARGV0}" AND IS_DIRECTORY "${CMAKE_SOURCE_DIR}/src/${ARGV0}")
            file(GLOB sources ${CMAKE_SOURCE_DIR}/src/${ARGV0}/*.cpp src/*.cpp)
        else()
            file(GLOB sources ${CMAKE_SOURCE_DIR}/src/*.cpp)
        endif()

        add_executable(${ARGV0} ${sources} ${PROTO_SRCS} ${PROTO_HDRS})
        target_link_libraries(${ARGV0} ${CMAKE_THREAD_LIBS_INIT} ${FMT_LIB_PATH} ${PROTOBUF_LIBRARIES})
    endif()
endmacro(create_targets)

if (${BUILD_DOC})
    # check if Doxygen is installed
    find_package(Doxygen)

    if (DOXYGEN_FOUND)
        # set input and output files
        set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
        set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

        # request to configure the file
        configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
        # note the option ALL which allows to build the docs together with the application
        add_custom_target(doc
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM
        )
    else ()
        message("Doxygen needs to be installed to generate the doxygen documentation")
    endif ()
endif()

# replace xxx with list of executables
# like: create_targets(exe) or create_targets(exe1 exe2 exe3)
# if just one target is given -> source files are in 'src'
# else -> source files specific for each target are in respective
#   subdirectories of 'src' like in 'src/exe1', 'src/exe2', and 'src/exe3'
#   common source files are in 'src'
create_targets(router)
