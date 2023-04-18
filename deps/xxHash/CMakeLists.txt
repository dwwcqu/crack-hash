if(ENABLE_BRAIN)
    set(XXHash_SOURCES xxhash.c)
endif()

if(ENABLE_BRAIN)
    add_compile_options(${CXXFLAGS} ${CFLAGS_NATIVE})
    add_library(xxhash ${XXHash_SOURCES})
endif()