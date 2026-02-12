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
# Libraries Used 📚
```bash
raylib
stdio.h
stdlib.h
stdbool.h
math.h
```
<!DOCTYPE html>
<html lang="pt-br">
<head>
  <meta charset="UTF-8">
</head>
<body>

  <div align="center">
    <h1>🌌 Project Architecture & Team</h1>
    <p><i>A deep dive into low-level engine development using C and Lua integration.</i></p>
  </div>

  <hr />

  <section>
    <h2 align="left">👥 Project Task Distribution</h2>
    <table align="center" width="100%">
      <thead>
        <tr style="background-color: #161b22;">
          <th width="30%" align="left">Developer</th>
          <th width="70%" align="left">Core Responsibilities</th>
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
          <td><a href="https://github.com/luizmiguelbarbosa"><b>Luiz Miguel Barbosa</b></a></td>
          <td>Input handling, audio system, integration, and main game loop.</td>
        </tr>
        <tr>
          <td><a href="https://github.com/miqueias-santos"><b>Miqueias Santos</b></a></td>
          <td>Art, animations, UI, and audio effects.</td>
        </tr>
      </tbody>
    </table>
  </section>

  <br />

  <section>
    <h2 align="left">📖 Concepts Applied</h2>
    <p>The project applies both fundamental and advanced software engineering concepts using <b>C</b>:</p>
    <ul>
      <li><b>Engine Design:</b> Modular architecture and main game loop implementation.</li>
      <li><b>Data Modeling:</b> Entity modeling using <code>structs</code>.</li>
      <li><b>Low-level Management:</b> Manual memory management and strict header/source separation.</li>
      <li><b>Graphics Pipeline:</b> Rendering pipeline control and shader implementation.</li>
      <li><b>Systems Integration:</b> Input handling (keyboard, mouse, joystick), physics, collision detection, and Lua scripting integration for dialogues and cutscenes.</li>
    </ul>
  </section>

  <br />

  <section>
    <h2 align="left">⚠️ Challenges & Lessons Learned</h2>
    <div style="background-color: #1f2328; border-left: 5px solid #d1242f; padding: 15px; border-radius: 6px;">
      <p style="color: #f85149; font-weight: bold; margin-bottom: 10px;">IMPORTANT</p>
      <p>Developing a modular engine in C brought several technical challenges:</p>
      <ul>
        <li><b>Memory Management:</b> Continuous vigilance to avoid memory leaks or invalid accesses through debugging tools and code reviews.</li>
        <li><b>Multi-module Architecture:</b> Needed clear dependencies between modules to prevent cyclic inclusions.</li>
        <li><b>C/Lua Interoperability:</b> Managing the Lua stack to ensure proper data passing between engine and scripts.</li>
        <li><b>Cross-platform Build:</b> Configuring CMake to ensure compilation and linking of external libraries across platforms.</li>
      </ul>
    </div>
  </section>

  <br />

  <div align="center">
    <hr />
    <p><i>This project strengthened low-level programming skills, modular engine design, and professional software engineering practices in a team environment.</i></p>
  </div>

</body>
</html>


