cmake_minimum_required(VERSION 4.0)
project(insert_soul C)

set(CMAKE_C_STANDARD 99)

# Define onde estão os headers (.h)
include_directories(include)
include_directories(src/arcade)
include_directories(src/audio)
include_directories(src/core)
include_directories(src/game)
include_directories(src/input)
include_directories(src/render)
include_directories(src/guitar_hero) # Importante para o jogo funcionar
include_directories(src/byte2)       # <<-- ADICIONADO: headers do byte2

# Configura a Raylib (ajuste o caminho se necessário)
set(RAYLIB_INCLUDE_PATH "C:/raylib/raylib/src")
set(RAYLIB_LIB_PATH "C:/raylib/raylib/src")

include_directories(${RAYLIB_INCLUDE_PATH})
link_directories(${RAYLIB_LIB_PATH})

# Lista APENAS os arquivos de código fonte (.c)
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

        # Módulos
        src/audio/audio.c
        src/core/engine.c
        src/core/logger.c
        src/core/memory.c
        src/render/render.c
        src/render/shaders/shader.c
        src/input/input.c

        # Gameplay specific
        src/game/player.c
        src/game/physics.c
        src/game/npc.c
        src/game/item.c

        # Arcade Games
        src/arcade/arcade.c
        src/guitar_hero/guitar_hero.c  # Guitar Hero

        # BYTE2 minigame (novo)
        src/byte2/byte2.c
)

# Cria o executável apenas com código fonte
add_executable(insert_soul ${SOURCES} assets/icon.rc)

# Linka as bibliotecas do Windows necessárias para Raylib
target_link_libraries(insert_soul
        PRIVATE
        raylib
        gdi32
        winmm
        opengl32
        kernel32
        user32
        shell32
        ole32
        oleaut32
        uuid
        comdlg32
        advapi32
)

# Copia a pasta assets (incluindo byte_assets) para o diretório do executável após compilar
add_custom_command(
        TARGET insert_soul POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:insert_soul>/assets
)

# (Opcional) se colocou os assets do byte dentro assets/byte_assets, mantenha essa pasta lá.
