cmake_minimum_required(VERSION 3.10)
project(main_c)

set(CMAKE_C_STANDARD 11)

include_directories(/usr/local/include)

# 使用 Ninja 生成器
set(CMAKE_GENERATOR ninja)

add_executable(main_c main.c
        app_state.c
        app_state.h
        config.h
        account.c
        account.h
        plan.c
        plan.h
        words.c
        words.h
        fonts.c
        fonts.h
        raylib_word_ui.c
        raylib_word_ui.h
        tree_menu.c
        tree_menu.h
        menu_callbacks.c
        menu_callbacks.h)

target_link_libraries(main_c PRIVATE raylib)
add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)