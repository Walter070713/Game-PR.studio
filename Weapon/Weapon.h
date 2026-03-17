#ifndef WEAPON_H
#define WEAPON_H
#include "raylib.h"
#include "raymath.h"

typedef enum {
    WEAPON_PISTOL = 0,
    WEAPON_RIFLE = 1,
    WEAPON_SHOTGUN = 2
} WeaponType;

typedef struct {
    WeaponType type;
    
    // Bullet properties
    float bulletSpeed;
    float bulletSize;
    Color bulletColor;
    
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

// Initialize a weapon with a specific type
Weapon InitWeapon(WeaponType type);

// Try to fire the weapon, returns true if successful
bool FireWeapon(Weapon* weapon);

// Reload the weapon
void ReloadWeapon(Weapon* weapon);

// Update weapon state (timers, etc)
void UpdateWeapon(Weapon* weapon);

// Get weapon info for HUD display
typedef struct {
    int magazine;
    int totalAmmo;
    bool isReloading;
    float reloadProgress; // 0.0 to 1.0
} WeaponInfo;

WeaponInfo GetWeaponInfo(const Weapon* weapon);

#endif
