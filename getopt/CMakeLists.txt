set(GETOPT_SOURCE_FILES
    getopt.c
    getopt1.c
    )
set(GETOPT_HEADER_FILES
    getopt.h
    )

add_library(getopt ${GETOPT_SOURCE_FILES} ${GETOPT_HEADER_FILES})
