cmake_minimum_required(VERSION 2.8.4)
project(xorer)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    main.cpp
    file_io.cpp
    )
set(HEADER_FILES
    ncch_header.h
    file_io.h
    common_types.h
    )

add_executable(xorer ${SOURCE_FILES} ${HEADER_FILES})