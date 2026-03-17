# Weapon System Documentation

## Overview
A scalable, modular weapon system with reload mechanics and ammo management. Designed for easy extension with new weapon types.

## Architecture

### Core Components

#### 1. **Weapon.h / Weapon.c**
- `Weapon` struct: Contains all weapon properties
- `WeaponType` enum: Define weapon types here for new weapons
- `weaponPresets[]`: Configure weapon properties (speed, ammo, fire rate, reload time)

#### 2. **Bullet.c** (Refactored)
- **Removed:** Firing logic and mouse input handling
- **Kept:** Bullet physics, movement, and rendering
- New functions:
  - `FireBullet()`: Spawn a bullet with custom properties
  - `UpdateBulletPhysics()`: Handle bullet movement and lifetime
  - Bullets now support custom colors for weapon differentiation

#### 3. **Player.c** (Updated)
- Now includes `Weapon weapon;` member
- Initialized with default weapon (WEAPON_PISTOL)
- Weapon state managed with `UpdateWeapon()`

#### 4. **main.c** (Refactored)
- Weapon update loop: `UpdateWeapon(&plyr.weapon)`
- Firing logic: `FireWeapon()` on mouse click
- Bullet spawning: `FireBullet()` with weapon properties
- New HUD: Shows ammo, magazine, reload status

## How to Add a New Weapon

### Step 1: Add to WeaponType enum (Weapon.h)
```c
typedef enum {
    WEAPON_PISTOL = 0,
    WEAPON_RIFLE = 1,
    WEAPON_SHOTGUN = 2,
    WEAPON_SNIPER = 3      // New weapon
} WeaponType;
```

### Step 2: Add preset config (Weapon.c)
```c
static const struct {
    // ... existing presets ...
    // WEAPON_SNIPER
    {
        .bulletSpeed = 4000.0f,    // Very fast
        .bulletSize = 8.0f,         // Small
        .maxMagazine = 5,           // Low ammo per mag
        .totalAmmo = 25,            // Limited total
        .fireRate = 2.0f,           // Slow fire rate
        .reloadTime = 3.0f,         // Long reload
        .bulletColor = CYAN         // Unique color
    }
} weaponPresets[4];  // Update array size!
```

### Step 3: That's it!
The weapon is ready to use:
```c
Weapon sniper = InitWeapon(WEAPON_SNIPER);
```

## Weapon Properties Explained

- **bulletSpeed**: How fast bullets travel (pixels/second)
- **bulletSize**: Visual bullet radius
- **maxMagazine**: Max rounds per magazine
- **totalAmmo**: Total ammunition reserves
- **fireRate**: Delay between shots (seconds)
- **reloadTime**: Time to reload (seconds)
- **bulletColor**: Visual/debugging color

## Usage in Code

### Initialize player with default weapon
```c
InitPlayer(&plyr, window_center); // Starts with PISTOL
```

### Update weapon timers and state
```c
UpdateWeapon(&plyr.weapon); // Call every frame
```

### Fire the weapon
```c
if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
{
    if (FireWeapon(&plyr.weapon))  // Returns true if successful
    {
        FireBullet(bulletpool, capacity, plyr.pos, mouse.dir,
                  plyr.weapon.bulletSpeed, plyr.weapon.bulletSize, 
                  plyr.weapon.bulletColor);
    }
}
```

### Get weapon info for HUD
```c
WeaponInfo info = GetWeaponInfo(&plyr.weapon);
DrawText(TextFormat("Ammo: %d/%d", info.magazine, plyr.weapon.maxMagazine), 10, 100, 30, YELLOW);
```

## HUD Controls

- **Left Click**: Fire weapon (if ready and has ammo)
- **R Key**: Manual reload (if not reloading and ammo available)
- **Auto-reload**: Automatic when magazine empties (if ammo reserves available)

## Reload System

1. Magazine count and total ammo are tracked separately
2. Reload takes `reloadTime` seconds
3. If no ammo to reload, weapon stays empty
4. Auto-reloads when magazine empties (if ammo available)
5. Manual reload via R key

## Future Enhancements

- Switch weapons: Add weapon selection (number keys or mouse wheel)
- Weapon drops: Enemies drop different weapon types
- Upgrades: Modify weapon properties dynamically
- Special effects: Knockback, piercing, area damage
- Weapon skins: Different visuals for same weapon type

## File Structure

```
Weapon/
  ├── Weapon.h         # Weapon interface & types
  └── Weapon.c         # Weapon implementation
Bullet/
  ├── Bullet.h         # Bullet physics only
  └── Bullet.c         # Bullet movement & rendering
Player/
  ├── Player.h         # Player struct with Weapon member
  └── Player.c         # Player with weapon init
main.c                 # Weapon integration in game loop
```