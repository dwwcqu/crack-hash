add_subdirectory(hashcat)
add_subdirectory(modules)
find_package(OpenGL REQUIRED)
# 哈希算法源文件
set(hash_algos_sources md5.cpp sha1.cpp sha3.cpp sha256.cpp)
# IMGUI 源文件
set(imgui_sources imgui.cpp imgui_demo.cpp imgui_draw.cpp imgui_impl_glfw.cpp imgui_impl_opengl3.cpp
                  imgui_tables.cpp imgui_widgets.cpp)

# 构建 crack-hash 可执行程序
add_executable(crack-hash main.cpp crack.cpp crack_gui.cpp glad.c ${hash_algos_sources} ${imgui_sources})

target_include_directories(crack-hash 
            PRIVATE ${PROJECT_SOURCE_DIR}/deps/glfw3/include
            PRIVATE ${PROJECT_SOURCE_DIR}/include/hash
            PRIVATE ${PROJECT_SOURCE_DIR}/include/imgui)
target_compile_definitions(crack-hash   PRIVATE -DCOMPTIME=${COMPTIME} 
                                        PRIVATE -DVERSION_TAG=\"${VERSION_TAG}\"
                                        PRIVATE -DINSTALL_FOLDER=\"${INSTALL_FOLDER}\"
                                        PRIVATE -DSHARED_FOLDER=\"${SHARED_FOLDER}\"
                                        PRIVATE -DDOCUMENT_FOLDER=\"${DOCUMENT_FOLDER}\")
set_target_properties(crack-hash
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}"
    )
target_link_libraries(crack-hash PRIVATE hashcat glfw GL ${LFLAGS_NATIVE})