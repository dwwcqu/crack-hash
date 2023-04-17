set(LZMA_SOURCES 7zCrc.c 7zCrcOpt.c 7zFile.c 7zStream.c Alloc.c
                 Bra.c Bra86.c BraIA64.c CpuArch.c Delta.c
                 LzmaDec.c Lzma2Dec.c Sha256.c Sha256Opt.c Xz.c
                 XzCrc64.c XzCrc64Opt.c XzDec.c XzIn.c)
add_library(lzma ${LZMA_SOURCES})
add_compile_options($(CCFLAGS) $(CFLAGS_NATIVE) ${CFLAGS_LZMA})