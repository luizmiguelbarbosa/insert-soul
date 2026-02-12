# INSERT SOUL 🎮💀

This is a 2D game project developed in **C**, using the **Raylib** library.  
The game is modular, with its own engine, input system, audio, rendering, physics, and scripting for cutscenes.  

The project follows good software engineering practices, with clear separation of responsibilities, documentation, and organized folder structure.

---

## Team 🧑‍💻
<p align="center">
<table align="center">
  <tr>
    <td align="center">
      <a href="https://github.com/gustavocharamba">
        <img src="https://avatars.githubusercontent.com/gustavocharamba" width="100px;" alt="Gustavo Charamba"/><br />
        <sub><b>Gustavo Charamba</b></sub>
      </a>
    </td>
    </td>
    <td align="center">
      <a href="https://github.com/rafael-smoura">
        <img src="https://avatars.githubusercontent.com/rafael-smoura" width="100px;" alt="rafael-smoura"/><br />
        <sub><b>Rafael</b></sub>
      </a>
    </td>
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

# Folder Structure 📂
## Project Folder Architecture
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
  <p><i>A deep dive into low-level engine development using C.</i></p>
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
      <td><a href="https://github.com/rafael-smoura"><b>Rafael</b></a></td>
      <td>Gameplay systems: Player, NPCs, physics, items, and level design.</td>
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

The project applies both fundamental and advanced software engineering concepts using the **C** language:

* **Engine Design:** Modular architecture and implementation of the *Main Game Loop*.
* **Data Modeling:** Entity modeling based on `structs`.
* **Low-level Management:** Manual memory management and strict separation between *headers* and *sources*.
* **Graphics Pipeline:** Control of the rendering pipeline and shader implementation.
* **Systems Integration:** 
    * Input handling (keyboard, mouse, joystick).
    * Physics and collision detection.
    * Lua scripting integration for automating dialogues and cutscenes.

---

## ⚠️ Challenges & Lessons Learned

Developing a modular engine in C brought technical challenges that required robust solutions:

> [!IMPORTANT]
> **Memory Management:** Continuous monitoring to prevent *memory leaks* and invalid accesses using debugging tools and code reviews.

1.  **Multi-module Architecture:** The need to maintain clear dependencies between modules to avoid cyclic inclusions.
2.  **C/Lua Interoperability:** The challenge of managing the Lua stack to ensure correct data passing between the engine and scripts.
3.  **Cross-platform Build:** Configuring **CMake** to ensure compilation and linking of external libraries work smoothly across different environments.

---

<div align="center">
  <p>This project strengthened skills in low-level programming, modular engine design, and professional software engineering practices in a team environment.</p>
</div>
