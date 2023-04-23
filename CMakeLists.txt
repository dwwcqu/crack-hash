cmake_minimum_required(VERSION 3.2)
project(crack_hashcat VERSION 0.1
        LANGUAGES C CXX)	

# production
option(PRODUCTION "Whether Build the Production" OFF)
# Debug 或者 Release 编译
set(DEBUG 1)
set(VERSION_TAG "v6.2.6-257-g1c55ee222+")

# 使用系统上依赖库，还是项目本身自带的依赖文件
option(USE_SYSTEM_LZMA "Whether Use System LZMA Library" OFF)
option(USE_SYSTEM_ZLIB "Whether Use System ZLIB Library" OFF)
option(USE_SYSTEM_OPENCL "Whether Use System OpenCL Header" OFF)
option(USE_SYSTEM_XXHASH "Whether Use System XXHash Library" OFF)
option(USE_SYSTEM_UNRAR "Whether Use System UNRAR Library" OFF)
# 位置无关代码
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
# Enable 选项
option(ENABLE_CUBIN "Whether Enable CUBIN" ON)
option(ENABLE_BRAIN "Whether Enable Brain Model" ON)
option(ENABLE_UNRAR "Whether Enable unrar Library" ON)

# hashcat 安装路径
set(INSTALL_FOLDER 			${CMAKE_INSTALL_PREFIX}/bin)
set(SHARED_ROOT_FOLDER   	${CMAKE_INSTALL_PREFIX}/share)
set(SHARED_FOLDER        	${SHARED_ROOT_FOLDER}/hashcat)
set(DOCUMENT_FOLDER         ${SHARED_ROOT_FOLDER}/doc/hashcat)
set(LIBRARY_FOLDER          ${CMAKE_INSTALL_PREFIX}/lib)
set(LIBRARY_DEV_ROOT_FOLDER	${CMAKE_INSTALL_PREFIX}/include)
set(LIBRARY_DEV_FOLDER      ${LIBRARY_DEV_ROOT_FOLDER}/hashcat)

# 所有的库文件采用静态库
set(BUILD_SHARED_LIBS OFF)

# 设置为调试编译
set(CMAKE_BUILD_TYPE "Debug")
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Detected Operating System: ${CMAKE_HOST_SYSTEM_NAME}")
endif()

# hashcat中所有 module 接口的版本号
set(MODULE_INTERFACE_VERSION 700)

# 获取编译时的时间戳
string(TIMESTAMP COMPTIME %s)

# 链接时使用的选项
if(BUILD_SHARED_LIBS)
	set(LFLAGS ${CMAKE_SHARED_LINKER_FLAGS})
else()
	set(LFLAGS ${CMAKE_STATIC_LINKER_FLAGS})
endif()

# 编译时使用的选项
set(CFLAGS)
if(NOT PRODUCTION)
	list(APPEND CFLAGS -W -Wall -Wextra)
endif()

# 针对 clang 编译器特有的选项
if(CMAKE_C_COMPILER STREQUAL "clang")
	list(APPEND CFLAGS -Wno-typedef-redefinition)
endif()

# 针对 LZMA 的选项
if(NOT USE_SYSTEM_LZMA)
	set(CFLAGS_LZMA -D_7ZIP_ST)
	if(CMAKE_C_COMPILER STREQUAL "clang")
		list(APPEND CFLAGS_LZMA -Wno-misleading-indentation)
	endif()
endif()

# 针对 ZLIB 的选项
if(NOT USE_SYSTEM_ZLIB)
	set(CFLAGS_ZLIB -Wno-implicit-fallthrough
				    -Wno-implicit-function-declaration
				    -Wno-unused-parameter
				    -DIOAPI_NO_64
				    -DUNZ_BUFSIZE=262144)
endif()

# 针对 UNRAR 的选项
if(ENABLE_UNRAR)
	if(NOT USE_SYSTEM_UNRAR)
		set(CFLAGS_UNRAR)
		if(CMAKE_C_COMPILER STREQUAL "clang")
			list(APPEND CFLAGS_UNRAR -Wno-class-memaccess
									 -Wno-misleading-indentation
									 -Wno-format-overflow)
		else()
			list(APPEND CFLAGS_UNRAR -Wno-missing-braces
									 -Wno-unused-variable
									 -Wno-unused-parameter
									 -Wno-unused-function
									 -Wno-sign-compare
									 -Wno-dangling-else
									 -Wno-switch
									 -Wno-parentheses
									 -Wno-implicit-fallthrough
									 -Wno-extra
									 -Wno-unknown-pragmas
									 -Wno-unused-but-set-variable
									 -Wno-deprecated-declarations
									 -Wno-misleading-indentation)
		endif()
	endif()
endif()

# 针对 Debug 或者 Release 的一些编译和链接选项
if(DEBUG EQUAL 0)
	list(APPEND CFLAGS -O2)
	if(NOT(CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin"))
		list(APPEND LFLAGS -s)
	endif()
elseif(DEBUG EQUAL 1)
	if(NOT(CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin"))
		list(APPEND CFLAGS -DDEBUG -Og -ggdb)
	else()
		list(APPEND CFLAGS -DDEBUG -O0 -ggdb)
	endif()
elseif(DEBUG EQUAL 2)
	if(NOT(CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin"))
		list(APPEND CFLAGS -DDEBUG -Og -ggdb)
	else()
		list(APPEND CFLAGS -DDEBUG -O0 -ggdb)
	endif()
	list(APPEND CFLAGS -fsanitize=address -fno-omit-frame-pointer)
endif()

if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
	execute_process(COMMAND -q Raspberry /proc/cpuinfo && echo 1 || echo 0
		RESULTE_VARIABLE IS_RPI)
	if(IS_RPI EQUAL 1)
		list(APPEND CFLAGS -mcpu=cortex-a72)
	endif()
endif()

list(APPEND CFLAGS -pipe)

# include 头文件路径
include_directories(include/ 
					OpenCL/)

# LZMA 库的头文件
if(NOT USE_SYSTEM_LZMA)
	include_directories(deps/LZMA-SDK/C/)
elseif(LibLZMA-FOUND)
	include_directories(${LibLZMA_INCLUDE_DIRS})
else()
	message(ERROR "LibLZMA Not Found In System")
endif()

# ZLIB 库的头文件
if(NOT USE_SYSTEM_ZLIB)
	include_directories(deps/zlib/ deps/zlib/contrib/)
elseif(ZLIB-FOUND)
	include_directories(${ZLIB_INCLUDE_DIRS})
else()
	message(ERROR "ZLIB Not Found In System")
endif()

# OpenCL 库的头文件
if(NOT USE_SYSTEM_OPENCL)
	include_directories(deps/OpenCL-Headers/)
elseif(OpenCL-FOUND)
	include_directories(${OpenCL_INCLUDE_DIRS})
else()
	message(ERROR "OpenCL Not Found In System")
endif()

# xxHash 库的头文件
if(ENABLE_BRAIN)
	list(APPEND CFLAGS -DWITH_BRAIN)
	if(NOT USE_SYSTEM_XXHASH)
		include_directories(deps/xxHash/)
	elseif(XXHash-Found)
		include_directories(${XXHash_INCLUDE_DIRS})
	else()
		message(ERROR "xxHash Not Found In System")
	endif()
endif()

# CUDA Binary Cache
if(ENABLE_CUBIN)
	list(APPEND CFLAGS -DWITH_CUBIN)
endif()

# unrar 库的头文件
if(ENABLE_UNRAR)
	include_directories(deps/unrar/)
	list(APPEND LFLAGS -lstdc++)
endif()

# 针对 Linux 本地编译的编译和链接特有选项
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
	set(CFLAGS_NATIVE ${CFLAGS})
	list(APPEND CFLAGS_NATIVE -DWITH_HWMON)
	set(LFLAGS_NATIVE ${LFLAGS})
	list(APPEND LFLAGS_NATIVE -lpthread -ldl -lrt -lm)
endif()

# 针对 FreeBSD 本地编译的编译和链接特有选项
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "FreeBSD")
	set(CFLAGS_NATIVE ${CFLAGS})
	set(LFLAGS_NATIVE ${LFLAGS})
	list(APPEND LFLAGS_NATIVE -lpthread -lm -liconv)
endif()

# 针对 NetBSD 本地编译的编译和链接特有选项
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "NetBSD")
	set(CFLAGS_NATIVE ${CFLAGS})
	set(LFLAGS_NATIVE ${LFLAGS})
	list(APPEND LFLAGS_NATIVE -lpthread)
endif()

# 针对 Darwin 本地编译的编译和链接特有选项
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
	execute_process(COMMAND export MACOSX_DEPLOYMENT_TARGET=10.15)
	execute_process(COMMAND uname -r | cut -d. -f1
			RESULTE_VARIABLE DARWIN_VERSION)
	execute_process(COMMAND lipo /bin/zsh -verify_arch arm64e && echo 1 || echo 0
			RESULTE_VARIABLE IS_APPLE_SILICON)
	set(CFLAGS_NATIVE ${CFLAGS})
	list(APPEND CFLAGS_NATIVE -DWITH_HWMON)
	execute_process(COMMAND test $(DARWIN_VERSION) -le 15; echo $$?
			RESULTE_VARIABLE Darwin_RES)
	if(Darwin_RES EQUAL 0)
		list(APPEND CFLAGS_NATIVE -DMISSING_CLOCK_GETTIME)
	endif()
	set(LFLAGS_NATIVE ${LFLAGS})
	list(APPEND LFLAGS_NATIVE -framework CoreFoundation
							  -framework CoreGraphics
							  -framework Foundation
							  -framework IOKit
							  -framework Metal
							  -lpthread
							  -liconv)
	if(IS_APPLE_SILICON EQUAL 1)
		list(APPEND CFLAGS_NATIVE -arch arm64 -arch x86_64)
	endif()
	if(BUILD_SHARED_LIBS)
		list(APPEND LFLAGS_NATIVE -arch arm64 -arch x86_64)
	endif()
endif()

# 针对 CYGWIN 本地编译的编译和链接特有选项
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "CYGWIN")
	set(CFLAGS_NATIVE ${CFLAGS})
	list(APPEND CFLAGS_NATIVE -DWITH_HWMON)
	set(LFLAGS_NATIVE ${LFLAGS})
	list(APPEND LFLAGS_NATIVE -Wl,--dynamicbase -Wl,--nxcompat -lpsapi -liconv)
endif()

# 针对 MSYS2 本地编译的编译和链接特有选项
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "MSYS2")
	set(CFLAGS_NATIVE ${CFLAGS})
	list(APPEND CFLAGS_NATIVE -DWITH_HWMON)
	set(LFLAGS_NATIVE ${LFLAGS})
	list(APPEND LFLAGS_NATIVE -Wl,--dynamicbase -Wl,--nxcompat -lpsapi -liconv -lws2_32 -lpowrprof)
endif()

set(CCFLAGS -std=gnu99)
set(CXXFLAGS)

# LibLZMA
if(USE_SYSTEM_LZMA)
	find_package(LibLZMA REQUIRED)
else()
	add_subdirectory(deps/LZMA-SDK)
endif()

# LibZLIB
if(USE_SYSTEM_ZLIB)
	find_package(ZLIB REQUIRED)
else()
	add_subdirectory(deps/zlib)
endif()

# OpenCL Headers
if(USE_SYSTEM_OPENCL)
	find_package(OpenCL REQUIRED)
else()
	add_subdirectory(deps/OpenCL-Headers)
endif()

# LibXXHash
if(USE_SYSTEM_XXHASH)
	find_package(XXHash REQUIRED)
else()
	add_subdirectory(deps/xxHash)
endif()

# LibUNRAR
if(ENABLE_UNRAR)
	if(USE_SYSTEM_UNRAR)
		find_package(UNRAR REQUIRED)
	else()
		add_subdirectory(deps/unrar)
	endif()
endif()

# GLFW3 库
add_subdirectory(deps/glfw3)
# hashcat 源文件的编译，需要与依赖的库进行链接
add_subdirectory(src)
