# INSERT SOUL

Este é um projeto de jogo 2D desenvolvido em C, utilizando a biblioteca **Raylib**.  
O jogo é modular, com engine própria, sistema de input, áudio, renderização, física e scripts para cutscenes.  

O projeto segue boas práticas de engenharia de software, com separação de responsabilidades, documentação clara e organização de pastas.

---

## 👥 Equipe

- **Engenharia de Software / Líder Técnico**: Arquitetura do motor, integração de bibliotecas e code review.  
- **Programador de Engine**: Renderização, shaders, câmera e input.  
- **Programador de Gameplay**: Player, NPCs, física, itens e níveis.  
- **Narrativa & Scripting**: Cutscenes, diálogos e triggers em Lua.  
- **Arte & Áudio**: Sprites, animações, música e efeitos sonoros.

---

## ⚙️ Instalação e Execução

// FALTA AQUI


## 📂 Estrutura do Projeto
````
CMakeLists.txt
README.md
assets/
    arcades/
    shared/
    ui/
build/
    compilados.txt
docs/
    design/
include/
    engine.h
source/
    main.c
    arcade/
        arcade.c
        arcade.h
    audio/
        audio.c
        audio.h
    core/
        engine.c
        engine.h
        logger.c
        memory.c
        memory.h
    game/
        item.c
        npc.c
        physics.c
        physics.h
        player.c
        player.h
    input/
        input.c
        input.h
    render/
        render.c
        render.h
        shaders/
            shader.c
    script/
        bindings/
        lua/
            cutscene/
                dialogues/
                    triggers/


````

## Descrição das Pastas e Arquivos

🧩 Descrição das Pastas e Arquivos
**assets/**

Contém todos os recursos do jogo: sprites, sons, fontes, shaders e scripts Lua.

**arcades/:** recursos gráficos de minijogos ou arcades internos

**shared/:** assets compartilhados (ícones, efeitos visuais)

**ui/:** elementos de interface gráfica

**build/**

Arquivos gerados durante o build do projeto.

**compilados.txt**: lista de binários ou arquivos compilados

**docs/**

Documentação do projeto.

**design/:** documentos de design do jogo, câmera, gameplay, animações e cutscenes

**include/**

Headers públicos do projeto.

**engine.h:** declara funções do motor (inicialização, render, input, áudio, física)

**source/**

Código-fonte organizado por módulos.

**main.c:** ponto de entrada do jogo (inicializa engine, loop principal e encerra subsistemas)

**arcade/:** código de minijogos

**audio/:** gerenciamento de música e efeitos sonoros

**core/:** engine principal, logger e gerenciamento de memória

**game/:** física, player, NPCs e itens

**input/:** captura de teclado, mouse e joystick

**render/:** renderização de sprites, HUD, câmeras e shaders

**script/:** integração Lua → engine, cutscenes, diálogos e triggers

## 📌 Notas ##

Cada módulo possui seu próprio README.md para documentação detalhada.

Use .gitignore para ignorar build/ e binários.

Scripts Lua, shaders e assets devem permanecer em assets/.

Mantenha alterações no core do motor coordenadas entre a equipe.


## 🧾 Licença ##

Este projeto é de uso acadêmico e experimental.
Para uso comercial, entre em contato com os autores.
