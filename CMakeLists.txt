cmake_minimum_required(VERSION 2.8.4)
project(xorer)

add_subdirectory(getopt)
add_subdirectory(polarssl)

include_directories(".")

if (NOT MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

set(SOURCE_FILES
    common_funcs.cpp
    main.cpp
    ncch.cpp
    ncsd.cpp
    file_io.cpp
    )
set(HEADER_FILES
    ncch.h
    ncsd.h
    file_io.h
    common_funcs.h
    common_types.h
    )

add_executable(xorer ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(xorer getopt polarssl)