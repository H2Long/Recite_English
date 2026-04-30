cmake_minimum_required(VERSION 3.10)
project(main_c)

set(CMAKE_C_STANDARD 11)

# 跨平台查找 raylib
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(raylib QUIET raylib)
endif()

if(raylib_FOUND)
    include_directories(${raylib_INCLUDE_DIRS})
    link_directories(${raylib_LIBRARY_DIRS})
else()
    # 备选：直接找头文件和库
    find_path(raylib_INCLUDE_DIR raylib.h)
    find_library(raylib_LIBRARY raylib)
    if(raylib_INCLUDE_DIR)
        include_directories(${raylib_INCLUDE_DIR})
    endif()
    if(raylib_LIBRARY)
        set(raylib_LIBRARIES ${raylib_LIBRARY})
    else()
        # 如果都没找到，fallback 到系统路径
        include_directories(/usr/local/include /usr/include)
        set(raylib_LIBRARIES raylib)
    endif()
endif()

add_executable(main_c
        src/main.c
        src/core/config.h
        src/core/app_state.c
        src/core/tree_menu.c
        src/modules/account.c
        src/modules/plan.c
        src/modules/words.c
        src/modules/fonts.c
        src/ui/raylib_word_ui.c
        src/ui/menu_callbacks.c)

target_include_directories(main_c PRIVATE src/core src/modules src/ui)

# 跨平台编译选项
if(MSVC)
    # MSVC: 启用 UTF-8 源码编码，禁用 strdup 警告
    target_compile_options(main_c PRIVATE /utf-8)
    target_compile_definitions(main_c PRIVATE _CRT_NONSTDC_NO_DEPRECATE)
else()
    # GCC/Clang: 指定 UTF-8 源码和执行编码
    target_compile_options(main_c PRIVATE -finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()

target_link_libraries(main_c PRIVATE ${raylib_LIBRARIES})
