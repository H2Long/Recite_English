cmake_minimum_required(VERSION 3.10)
project(main_c)

set(CMAKE_C_STANDARD 11)

# ============================================================================
# raylib 查找策略：
# 1. 优先静态链接 — 生成独立可执行文件，无需额外 .dll/.so
# 2. 没有静态库时回退到动态链接
# ============================================================================

# 先尝试找静态库
set(saved_suffixes ${CMAKE_FIND_LIBRARY_SUFFIXES})
set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
find_library(raylib_STATIC_LIB raylib)
set(CMAKE_FIND_LIBRARY_SUFFIXES ${saved_suffixes})

if(raylib_STATIC_LIB)
    message(STATUS "raylib: 静态链接 → ${raylib_STATIC_LIB}")
    set(raylib_LIBRARIES ${raylib_STATIC_LIB})
    set(RAYLIB_IS_STATIC TRUE)
else()
    # 回退到动态链接
    find_library(raylib_LIBRARY raylib)
    if(raylib_LIBRARY)
        message(STATUS "raylib: 动态链接 → ${raylib_LIBRARY}")
        set(raylib_LIBRARIES ${raylib_LIBRARY})
        set(RAYLIB_IS_STATIC FALSE)
    else()
        message(FATAL_ERROR "找不到 raylib！安装:\n"
                "  Ubuntu/Debian: sudo apt install libraylib-dev\n"
                "  macOS:          brew install raylib\n"
                "  Windows:        vcpkg install raylib")
    endif()
endif()

# 查找 raylib 头文件
find_path(raylib_INCLUDE_DIR raylib.h
    PATHS /usr/local/include /usr/include /opt/homebrew/include)
if(NOT raylib_INCLUDE_DIR)
    message(FATAL_ERROR "找不到 raylib.h 头文件！")
endif()
include_directories(${raylib_INCLUDE_DIR})

# ============================================================================
# 可执行文件
# ============================================================================

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
        src/ui/menu_callbacks.c
        src/ui/pages/pages.h
        src/ui/pages/page_home.c
        src/ui/pages/page_learn.c
        src/ui/pages/page_review_root.c
        src/ui/pages/page_card_review.c
        src/ui/pages/page_select_word.c
        src/ui/pages/page_test.c
        src/ui/pages/page_search.c
        src/ui/pages/page_plan.c
        src/ui/pages/page_settings.c
        src/ui/pages/page_word_manager.c
        src/ui/pages/page_account.c)

target_include_directories(main_c PRIVATE src/core src/modules src/ui src/ui/pages)

# 跨平台编译选项
if(MSVC)
    target_compile_options(main_c PRIVATE /utf-8)
    target_compile_definitions(main_c PRIVATE _CRT_NONSTDC_NO_DEPRECATE)
else()
    target_compile_options(main_c PRIVATE -finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()

target_link_libraries(main_c PRIVATE ${raylib_LIBRARIES})

# 静态链接时需要链接 raylib 的系统依赖
if(RAYLIB_IS_STATIC AND NOT MSVC)
    target_link_libraries(main_c PRIVATE m pthread dl)
    # X11 相关（Linux）
    target_link_libraries(main_c PRIVATE X11 Xrandr Xinerama Xcursor Xi Xxf86vm GL)
endif()
