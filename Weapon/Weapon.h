#ifndef WEAPON_H
#define WEAPON_H
#include "raylib.h"

// Supported weapon presets.
typedef enum {
    WEAPON_PISTOL = 0,
    WEAPON_RIFLE,
    WEAPON_SHOTGUN,
    WEAPON_COUNT
} WeaponType;

typedef struct {
    WeaponType type;
    
    // Bullet properties
    float bulletSpeed;
    float bulletSize;
    Color bulletColor;

    // Pool capacity -- how many bullets can exist at once for this weapon
    int bulletPoolSize;
    
    // Magazine & ammo
    int magazine;          // Current ammo in magazine
    int maxMagazine;       // Max ammo per magazine
    int totalAmmo;         // Total ammo reserves
    
    // Timing
    float fireRate;        // Delay between shots (seconds)
    float reloadTime;      // Time to reload (seconds)
    float lastFireTime;    // Time elapsed since last shot
    float reloadTimer;     // Current reload progress
    bool isReloading;      // Currently reloading?
} Weapon;

// Get the max bullet pool size for the given weapon (used by the bullet system)
int GetWeaponBulletPoolSize(const Weapon* weapon);

// Create a weapon from preset values.
Weapon InitWeapon(WeaponType type);

// Attempt one shot; returns true only when fire gate allows it.
bool FireWeapon(Weapon* weapon);

// Enter reload state.
void ReloadWeapon(Weapon* weapon);

// Advance timers and handle manual/auto reload progression.
void UpdateWeapon(Weapon* weapon);

// Read-only HUD payload extracted from weapon state.
typedef struct {
    int magazine;
    int totalAmmo;
    bool isReloading;
    float reloadProgress; // 0.0 to 1.0
} WeaponInfo;

// Snapshot weapon data for UI rendering.
WeaponInfo GetWeaponInfo(const Weapon* weapon);

// Draw reload indicator/hint widget.
void DrawReload(const WeaponInfo* winfo);

#endif
