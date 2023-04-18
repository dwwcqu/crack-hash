set(EMU_SOURCES emu_general.c emu_inc_common.c emu_inc_platform.c emu_inc_simd.c)
list(APPEND EMU_SOURCES emu_inc_rp.c emu_inc_rp_optimized.c)
list(APPEND EMU_SOURCES emu_inc_hash_md4.c emu_inc_hash_md5.c emu_inc_hash_ripemd160.c
                        emu_inc_hash_sha1.c emu_inc_hash_sha256.c
                        emu_inc_hash_sha384.c emu_inc_hash_sha512.c
                        emu_inc_hash_streebog256.c emu_inc_hash_streebog512.c
                        emu_inc_ecc_secp256k1.c emu_inc_bignum_operations.c)
list(APPEND EMU_SOURCES emu_inc_cipher_aes.c emu_inc_cipher_camellia.c emu_inc_cipher_des.c
                        emu_inc_cipher_kuznyechik.c emu_inc_cipher_serpent.c emu_inc_cipher_twofish.c)
list(APPEND EMU_SOURCES emu_inc_hash_base58.c)

set(SOURCES affinity.c autotune.c backend.c benchmark.c bitmap.c
            bitops.c combinator.c common.c convert.c cpt.c
            cpu_crc32.c debugfile.c dictstat.c dispatch.c dynloader.c
            event.c ext_ADL.c ext_cuda.c ext_hip.c ext_nvapi.c
            ext_nvml.c ext_nvrtc.c ext_hiprtc.c ext_OpenCL.c ext_sysfs_amdgpu.c
            ext_sysfs_cpu.c ext_iokit.c ext_lzma.c filehandling.c folder.c
            hashcat.c hashes.c hlfmt.c hwmon.c induct.c
            interface.c keyboard_layout.c locking.c logfile.c loopback.c
            memory.c monitor.c mpsp.c outfile_check.c outfile.c 
            pidfile.c potfile.c restore.c rp.c rp_cpu.c
            selftest.c slow_candidates.c shared.c status.c stdout.c
            straight.c terminal.c thread.c timer.c tuningdb.c
            usage.c user_options.c wordlist.c hashcat_main.c
            ${EMU_SOURCES})
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
    list(APPEND SOURCES ext_metal.m)
endif()

if(ENABLE_BRAIN)
    list(APPEND SOURCES brain.c)
endif()
add_compile_options(${CCFLAGS} ${CFLAGS_NATIVE})
# 构建共享库
set(BUILD_SHARED_LIBS ON)
# 编译源文件
add_library(hashcat ${SOURCES})
target_compile_definitions(hashcat   PRIVATE -DCOMPTIME=${COMPTIME} 
                                        PRIVATE -DVERSION_TAG=\"${VERSION_TAG}\"
                                        PRIVATE -DINSTALL_FOLDER=\"${INSTALL_FOLDER}\"
                                        PRIVATE -DSHARED_FOLDER=\"${SHARED_FOLDER}\"
                                        PRIVATE -DDOCUMENT_FOLDER=\"${DOCUMENT_FOLDER}\")
# 确定要链接的依赖库
set(HASHCAT_LINK_LIBRARIES)
# 链接 LZMA 库
if(NOT USE_SYSTEM_LZMA)
    list(APPEND HASHCAT_LINK_LIBRARIES PRIVATE lzma)
else()
    list(APPEND HASHCAT_LINK_LIBRARIES PRIVATE LibLZMA)
endif()

# 链接 UNRAR 库
if(NOT USE_SYSTEM_UNRAR)
    list(APPEND HASHCAT_LINK_LIBRARIES PRIVATE unrar)
else()
    list(APPEND HASHCAT_LINK_LIBRARIES PRIVATE UNRAR)
endif()

# 链接 xxHash 库
if(NOT USE_SYSTEM_XXHASH)
    list(APPEND HASHCAT_LINK_LIBRARIES PRIVATE xxhash)
else()
    list(APPEND HASHCAT_LINK_LIBRARIES PRIVATE XXHash)
endif()

# 链接 ZLIB 库
if(NOT USE_SYSTEM_ZLIB)
    list(APPEND HASHCAT_LINK_LIBRARIES PRIVATE zlib)
else()
    list(APPEND HASHCAT_LINK_LIBRARIES PRIVATE ZLIB)
endif()
# 链接 hashcat 和其所有依赖的库构成一个共享库
target_link_libraries(hashcat ${HASHCAT_LINK_LIBRARIES})