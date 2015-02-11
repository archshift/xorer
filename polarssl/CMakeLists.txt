set(POLARSSL_SOURCE_FILES
    sha256.c
    )
set(POLARSSL_HEADER_FILES
    check_config.h
    config.h
    platform.h
    sha256.h
    )

add_library(polarssl ${POLARSSL_SOURCE_FILES} ${POLARSSL_HEADER_FILES})
