# INSERT SOUL 🎮💀

This is a 2D game project developed in **C**, using the **Raylib** library.  
The game is modular, with its own engine, input system, audio, rendering, physics, and scripting for cutscenes.  

The project follows good software engineering practices, with clear separation of responsibilities, documentation, and organized folder structure.

---

## Developer 🧑‍💻
<p align="center">
<table align="center">
  <tr>
    <td align="center">
      <a href="https://github.com/gustavocharamba">
        <img src="https://avatars.githubusercontent.com/gustavocharamba" width="100px;" alt="Gustavo Charamba"/><br />
        <sub><b>Gustavo Charamba</b></sub>
      </a>
    </td>
    <td align="center">
      <a href="https://github.com/lgss0">
        <img src="https://avatars.githubusercontent.com/lgss0" width="100px;" alt="lgss0"/><br />
        <sub><b>Luiz Gabriel</b></sub>
      </a>
    </td>
    <td align="center">
      <a href="https://github.com/rafael-smoura">
        <img src="https://avatars.githubusercontent.com/rafael-smoura" width="100px;" alt="rafael-smoura"/><br />
        <sub><b>Rafael</b></sub>
      </a>
    </td>
    <td align="center">
      <a href="https://github.com/lebb8">
        <img src="https://avatars.githubusercontent.com/lebb8" width="100px;" alt="lebb8"/><br />
        <sub><b>Eduardo</b></sub>
      </a>
    </td>
    <td align="center">
      <a href="https://github.com/luizmiguelbarbosa">
        <img src="https://avatars.githubusercontent.com/luizmiguelbarbosa" width="100px;" alt="Luiz Miguel Barbosa"/><br />
        <sub><b>Luiz Miguel Barbosa</b></sub>
      </a>
    </td>
    <td align="center">
      <a href="https://github.com/miqueias-santos">
        <img src="https://avatars.githubusercontent.com/miqueias-santos" width="100px;" alt="Miqueias Santos"/><br />
        <sub><b>Miqueias Santos</b></sub>
      </a>
  </tr>
</table>
</p>

---

## Installing the Game ⚙️🛠️

Clone the repository:

```bash
git clone https://github.com/luizmiguelbarbosa/insert_soul.git
```
Create a build directory and compile using CMake:
```bash
mkdir build
cd build
cmake ..
cmake --build .
```
Run the generated executable inside the build folder.

## Folder Structure 📂
# Project Folder Architecture
```bash
CMakeLists.txt
README.md
assets/
    arcades/
    shared/
    ui/
build/
    compiled.txt
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
```
## Libraries Used 📚
```bash
raylib
stdio.h
stdlib.h
stdbool.h
math.h
```
<div align="center">
  <h1>🌌 Project Architecture & Team</h1>
  <p><i>A deep dive into low-level engine development using C and Lua integration.</i></p>
</div>

<hr />

## 👥 Project Task Distribution

<table align="center" width="100%">
  <thead>
    <tr>
      <th width="30%">Developer</th>
      <th width="70%">Core Responsibilities</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td><a href="https://github.com/gustavocharamba"><b>Gustavo Charamba</b></a></td>
      <td>Engine architecture, integration of libraries, and code review.</td>
    </tr>
    <tr>
      <td><a href="https://github.com/lgss0"><b>Luiz Gabriel</b></a></td>
      <td>Rendering system, shaders, and camera controls.</td>
    </tr>
    <tr>
      <td><a href="https://github.com/rafael-smoura"><b>Rafael</b></a></td>
      <td>Gameplay systems: Player, NPCs, physics, items, and level design.</td>
    </tr>
    <tr>
      <td><a href="https://github.com/lebb8"><b>Eduardo</b></a></td>
      <td>Lua scripting for cutscenes, dialogues, and triggers.</td>
    </tr>
    <tr>
      <td><a href="https://github.com/luizmiguelbarbosa"><b>Luiz Miguel Barbosa</b></a></td>
      <td>Input handling, audio system, integration, and main game loop.</td>
    </tr>
    <tr>
      <td><a href="https://github.com/miqueias-santos"><b>Miqueias Santos</b></a></td>
      <td>Art, animations, UI, and audio effects.</td>
    </tr>
  </tbody>
</table>

---

## 📖 Concepts Applied

O projeto aplica conceitos fundamentais e avançados de engenharia de software utilizando a linguagem **C**:

* **Engine Design:** Arquitetura modular e implementação de *Main Game Loop*.
* **Data Modeling:** Modelagem de entidades baseada em `structs`.
* **Low-level Management:** Gerenciamento manual de memória e separação rigorosa entre *headers* e *sources*.
* **Graphics Pipeline:** Controle de pipeline de renderização e implementação de shaders.
* **Systems Integration:** * Input handling (Teclado, Mouse, Joystick).
    * Física e detecção de colisão.
    * Integração de scripting via **Lua** para automação de diálogos e cutscenes.

---

## ⚠️ Challenges & Lessons Learned

O desenvolvimento de uma engine modular em C trouxe desafios técnicos que exigiram soluções robustas:

> [!IMPORTANT]
> **Memory Management:** Vigilância constante para evitar *memory leaks* e acessos inválidos através de ferramentas de debug e revisões de código.

1.  **Arquitetura Multi-módulo:** A necessidade de manter dependências claras entre módulos para evitar inclusões cíclicas.
2.  **Interoperabilidade C/Lua:** O desafio de lidar com a pilha (*stack*) do Lua para garantir a passagem correta de dados entre o motor e os scripts.
3.  **Cross-platform Build:** Configuração do **CMake** para garantir que a compilação e o link de bibliotecas externas funcionem perfeitamente em diferentes ambientes.

---

<div align="center">
  <p>Este projeto consolidou habilidades em programação de baixo nível, design de engines modulares e práticas profissionais de engenharia de software em equipe.</p>
</div>


