cmake_minimum_required(VERSION 4.0)
project(insert_soul C)

set(CMAKE_C_STANDARD 99)

# --- 1. DEFINIÇÃO DOS DIRETÓRIOS DE HEADER (.h) ---
include_directories(include)
include_directories(src/arcade)
include_directories(src/audio)
include_directories(src/guitar_hero)
include_directories(src/byte2/include)
include_directories(src/byte2)

# --- 2. CONFIGURAÇÃO DA RAYLIB ---
set(RAYLIB_INCLUDE_PATH "C:/raylib/raylib/src")
set(RAYLIB_LIB_PATH "C:/raylib/raylib/src")

include_directories(${RAYLIB_INCLUDE_PATH})
link_directories(${RAYLIB_LIB_PATH})

# --- 3. LISTA DE ARQUIVOS FONTE (.c) ---
set(SOURCES
        # --- SISTEMA PRINCIPAL (LOBBY) ---
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
        src/arcade/arcade.c

        # --- JOGO 1: GUITAR HERO ---
        src/guitar_hero/guitar_hero.c

        # --- JOGO 2: BYTE SPACE ---
        src/byte2/byte2.c
        src/byte2/b2_audio.c
        src/byte2/b2_bullet.c
        src/byte2/b2_cutscene.c
        src/byte2/b2_enemy.c
        src/byte2/b2_hud.c
        src/byte2/b2_player.c
        src/byte2/b2_shop.c
        src/byte2/b2_star.c
)

# --- 4. CRIAÇÃO DO EXECUTÁVEL E MOVIDO PARA A RAIZ ---
add_executable(insert_soul ${SOURCES} assets/icon.rc)

# NOVO COMANDO: Define que o diretório de saída do executável é o diretório raiz do projeto.
set_target_properties(insert_soul PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}"
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}"
)

# --- 5. LINKAGEM DAS BIBLIOTECAS (Windows) ---
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

# --- 6. COPIA A PASTA ASSETS ---
# ATENÇÃO: O comando agora copia os assets para a raiz, ao lado do .exe
add_custom_command(
        TARGET insert_soul POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        ${CMAKE_SOURCE_DIR}/assets
)