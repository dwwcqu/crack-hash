# 构建共享库
set(BUILD_SHARED_LIBS ON)
set(UNIX_NAMES Linux FreeBSD Darwin NetBSD)
foreach(SYS_NAME IN LISTS UNIX_NAMES)
    if(CMAKE_HOST_SYSTEM_NAME STREQUAL ${SYS_NAME})
        SET(CMAKE_SHARED_LIBRARY_PREFIX "")
        break()
    endif()
endforeach()

# 设置每个模块的输出目录为项目根目录的 modules 文件夹下
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/modules)
add_compile_options(-DMODULE_INTERFACE_VERSION_CURRENT=${MODULE_INTERFACE_VERSION})

# hashcat 的各个模块的编译，目前的 crack-hash 只能破解以下算法的哈希值
# MD5 
add_library(module_00000 module_00000.c)
# SHA1
add_library(module_00100 module_00100.c)
# SHA2-224
add_library(module_01300 module_01300.c)
# SHA2-256
add_library(module_01400 module_01400.c)
# SHA2-384
add_library(module_10800 module_10800.c)
# SHA2-512
add_library(module_01700 module_01700.c)
# SHA3-224
add_library(module_17300 module_17300.c)
# SHA3-256
add_library(module_17400 module_17400.c)
# SHA3-384
add_library(module_17500 module_17500.c)
# SHA3-512
add_library(module_17600 module_17600.c)