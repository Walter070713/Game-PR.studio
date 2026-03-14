# Game-PR.studio

A small top-down shooter demo built with **raylib** in C. Move the player around a room, avoid enemies, and shoot them with a mouse-aimed weapon.

---

## 🎮 Gameplay

- **Move:** `W`, `A`, `S`, `D`
- **Aim:** Move the mouse cursor
- **Shoot:** Left mouse button
- Enemies spawn periodically and chase the player.
- Enemies flash red when hit and die after enough hits.
- The player takes damage when enemies touch you.

---

## 🧩 Project Structure

```
Game-PR.studio/
├── main.c                # Game loop and core flow
├── Basic Settings/       # Shared configuration (window size)
├── Player/               # Player movement & rendering
├── Enemy/                # Enemy AI, state, drawing
├── Bullet/               # Bullet pooling + firing logic
├── Map/                  # Room boundaries + wall generation
├── Collision/            # Game collision and response
├── Spawn/                # Enemy spawn logic
├── Mouse/                # Mouse aiming helper
├── include/              # raylib headers
├── lib/                  # raylib library files (linked on build)
└── game.exe              # Built executable (Windows)
```

---

## 🧱 Dependencies

This project uses **raylib** and is built for **Windows**.

### Required tools

- A C compiler (e.g., `gcc` from MinGW/MSYS2)
- raylib headers and libraries (already included in `include/` and `lib/`)

If you don’t have raylib installed, you can get it from https://www.raylib.com/ and put the headers in `include/` and the `.a`/`.lib` files in `lib/`, or install it through a package manager (MSYS2: `pacman -S mingw-w64-x86_64-raylib`).

---

## 🛠️ Build & Run

### Option A: Use the provided VS Code build task
1. Open the workspace in VS Code.
2. Run the **`build raylib game`** task (Terminal > Run Task > `build raylib game`).

### Option B: Build manually from terminal
From the project root, run:

```bash
gcc main.c Player/Player.c Camera/CameraSet.c Mouse/MouseAim.c Bullet/Bullet.c Enemy/Enemy.c Collision/Collision.c Spawn/Spawn.c Map/Map.c -o game.exe -Iinclude -IPlayer -ICamera -IMouse -IBullet -I"Basic Settings" -IEnemy -ICollision -ISpawn -IMap -Llib -lraylib -lopengl32 -lgdi32 -lwinmm
```

Then run:

```bash
./game.exe
```

> ⚠️ If you use a compiler other than `gcc`, adjust the flags accordingly.

---

## 🧠 How the Game Works (High-Level)

- **main.c**: Initializes the window, game objects, and enters the main loop.
- **Player/**: Handles player movement and rendering.
- **Mouse/**: Calculates a direction vector from the player to the cursor.
- **Bullet/**: Uses a fixed-size pool. Bullets fire from the player toward the mouse and deactivate when they travel too far.
- **Enemy/**: Enemies chase the player, flash when hit, and die after their health is reduced to 0.
- **Collision/**: Detects and resolves collisions between player/enemies/bullets and map walls.
- **Spawn/**: Periodically reactivates dead enemies at safe spawn points.
- **Map/**: Defines the room boundaries and internal walls.

---

## 🧪 Extending the Game

Some ideas to build on this prototype:

- Add a scoring system and display it on screen.
- Add different weapon types (spread shots, charged shots, etc.).
- Add enemy variety (different speeds, health, behaviors).
- Add a UI with health/shield bars and a “Game Over” screen.
- Add sound effects and music using raylib’s audio module.

---

## 📝 Notes / Known Quirks

- Currently, enemy boundary clamping has a small bug where an enemy hitting the right boundary will sometimes use the player position for correction (it still works but could be fixed).
- The game was designed for a 2560×1600 window, but it will run at other resolutions; the room itself is 2400×1800.

---

Enjoy building and expanding this raylib shooter! 🚀