cmake_minimum_required(VERSION 3.10)
project(main_c)

set(CMAKE_C_STANDARD 11)

include_directories(/usr/local/include)

# 使用 Ninja 生成器
set(CMAKE_GENERATOR ninja)

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

# 添加各层级目录为 include 搜索路径
target_include_directories(main_c PRIVATE src/core)
target_include_directories(main_c PRIVATE src/modules)
target_include_directories(main_c PRIVATE src/ui)

target_link_libraries(main_c PRIVATE raylib)
add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)
