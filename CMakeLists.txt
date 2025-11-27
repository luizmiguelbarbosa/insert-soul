cmake_minimum_required(VERSION 3.5)
project(insert_soul C)

set(CMAKE_C_STANDARD 99)

# --- Include dos seus headers ---
include_directories(include)
include_directories(src)
include_directories(src/arcade)
include_directories(src/audio)
include_directories(src/core)
include_directories(src/game)
include_directories(src/input)
include_directories(src/render)
include_directories(src/guitar_hero)

# --- Fonte principais ---
set(SOURCES
        src/main.c
        src/system.c
        src/system_config.c
        src/intro.c
        src/menu.c
        src/game.c
        src/video_player.c
        src/transition.c
        src/credits.c
        src/dialog.c

        src/audio/audio.c
        src/core/engine.c
        src/core/logger.c
        src/core/memory.c
        src/render/render.c
        src/render/shaders/shader.c
        src/input/input.c

        src/game/player.c
        src/game/physics.c
        src/game/npc.c
        src/game/item.c

        src/arcade/arcade.c
        src/guitar_hero/guitar_hero.c
)

add_executable(insert_soul ${SOURCES})

# ===========================================================
#              🔥  SUPORTE UNIVERSAL À RAYLIB  🔥
# ===========================================================
find_package(raylib 4.0 QUIET)

if (NOT raylib_FOUND)
    message(STATUS "Raylib não encontrado — baixando automaticamente…")

    include(FetchContent)
    FetchContent_Declare(
            raylib
            GIT_REPOSITORY https://github.com/raysan5/raylib.git
            GIT_TAG 4.5.0
    )
    FetchContent_MakeAvailable(raylib)
endif()

target_link_libraries(insert_soul raylib)

# ===========================================================
#        🔥  LINKAGEM ESPECÍFICA PARA CADA SISTEMA  🔥
# ===========================================================

if (APPLE)
    message(STATUS "Compilando para macOS")

    # libs do macOS
    target_link_libraries(insert_soul
            "-framework Cocoa"
            "-framework IOKit"
            "-framework OpenGL"
            "-framework CoreAudio"
            "-framework CoreVideo"
            "-framework Security"
    )

elseif (WIN32)
    message(STATUS "Compilando para Windows")

    target_link_libraries(insert_soul
            gdi32 winmm opengl32 user32 kernel32 shell32
    )

elseif (UNIX)
    message(STATUS "Compilando para Linux")

    target_link_libraries(insert_soul
            m pthread dl X11 Xi Xrandr Xcursor GL
    )
endif()

# ===========================================================
#        Copia os assets para a pasta do executável
# ===========================================================
add_custom_command(
        TARGET insert_soul POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:insert_soul>/assets
)
