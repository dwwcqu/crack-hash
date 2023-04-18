add_subdirectory(hashcat)
add_subdirectory(modules)
find_package(OpenGL REQUIRED)
# 构建 crack-hash 可执行程序
add_executable(crack-hash main.cpp)
target_compile_definitions(crack-hash   PRIVATE -DCOMPTIME=${COMPTIME} 
                                        PRIVATE -DVERSION_TAG=\"${VERSION_TAG}\"
                                        PRIVATE -DINSTALL_FOLDER=\"${INSTALL_FOLDER}\"
                                        PRIVATE -DSHARED_FOLDER=\"${SHARED_FOLDER}\"
                                        PRIVATE -DDOCUMENT_FOLDER=\"${DOCUMENT_FOLDER}\")
set_target_properties(crack-hash
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}"
    )
target_link_libraries(crack-hash PUBLIC hashcat ${LFLAGS_NATIVE})