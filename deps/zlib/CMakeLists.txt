set(ZLIB_SOURCES adler32.c crc32.c deflate.c inflate.c inffast.c
                 inftrees.c trees.c gzread.c gzwrite.c gzclose.c
                 zutil.c gzlib.c contrib/minizip/unzip.c contrib/minizip/ioapi.c)
                 
add_compile_options(${CXXFLAGS} ${CFLAGS_NATIVE} ${CFLAGS_ZLIB})
add_library(zlib ${ZLIB_SOURCES})