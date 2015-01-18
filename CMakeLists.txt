cmake_minimum_required(VERSION 2.8.4)
project(xorer)

include_directories(".")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    common_funcs.cpp
    main.cpp
    ncch.cpp
    ncsd.cpp
    file_io.cpp
    polarssl/sha256.c
    )
set(HEADER_FILES
    ncch.h
    ncsd.h
    file_io.h
    common_funcs.h
    common_types.h
    polarssl/check_config.h
    polarssl/config.h
    polarssl/platform.h
    polarssl/sha256.h
    )

add_executable(xorer ${SOURCE_FILES} ${HEADER_FILES})
