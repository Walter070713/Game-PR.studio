#include "Weapon.h"

// Weapon preset configurations - easy to modify or add new weapons
static const struct {
    float bulletSpeed;
    float bulletSize;
    int bulletPoolSize;
    int maxMagazine;
    int totalAmmo;
    float fireRate;
    float reloadTime;
    Color bulletColor;
} weaponPresets[3] = {
    // WEAPON_PISTOL
    {
        .bulletSpeed = 1500.0f,
        .bulletSize = 12.0f,
        .bulletPoolSize = 30, // Enough for ~2 magazines worth of bullets in flight
        .maxMagazine = 15,
        .totalAmmo = 90,
        .fireRate = 0.1f,      // 10 shots per second
        .reloadTime = 1.5f,
        .bulletColor = YELLOW
    },
    // WEAPON_RIFLE
    {
        .bulletSpeed = 2500.0f,
        .bulletSize = 10.0f,
        .bulletPoolSize = 60, // Enough for ~2 magazines worth of bullets in flight
        .maxMagazine = 30,
        .totalAmmo = 120,
        .fireRate = 0.08f,     // 12.5 shots per second (faster)
        .reloadTime = 2.0f,
        .bulletColor = ORANGE
    },
    // WEAPON_SHOTGUN
    {
        .bulletSpeed = 1200.0f,
        .bulletSize = 18.0f,
        .bulletPoolSize = 32, // Some extra for spread shards or rapid firing
        .maxMagazine = 8,
        .totalAmmo = 40,
        .fireRate = 1.0f,      // 1 shot per second (slower)
        .reloadTime = 2.5f,
        .bulletColor = RED
    }
};

Weapon InitWeapon(WeaponType type)
{
    Weapon w = {0};
    w.type = type;
    
    // Load preset values
    w.bulletSpeed = weaponPresets[type].bulletSpeed;
    w.bulletSize = weaponPresets[type].bulletSize;
    w.bulletPoolSize = weaponPresets[type].bulletPoolSize;
    w.maxMagazine = weaponPresets[type].maxMagazine;
    w.totalAmmo = weaponPresets[type].totalAmmo;
    w.fireRate = weaponPresets[type].fireRate;
    w.reloadTime = weaponPresets[type].reloadTime;
    w.bulletColor = weaponPresets[type].bulletColor;
    
    // Start with full magazine
    w.magazine = w.maxMagazine;
    w.lastFireTime = 0.0f;
    w.reloadTimer = 0.0f;
    w.isReloading = false;
    
    return w;
}

bool FireWeapon(Weapon* weapon)
{
    // Can't fire if reloading, no ammo, or fire rate not met
    if (weapon->isReloading || weapon->magazine <= 0) {
        return false;
    }
    
    if (weapon->lastFireTime < weapon->fireRate) {
        return false;
    }
    
    // Fire the weapon
    weapon->magazine--;
    weapon->lastFireTime = 0.0f;
    
    // Auto-reload if magazine empty
    if (weapon->magazine == 0 && weapon->totalAmmo > 0) {
        ReloadWeapon(weapon);
    }
    
    return true;
}

void ReloadWeapon(Weapon* weapon)
{
    // Already reloading or no ammo to reload
    if (weapon->isReloading || weapon->totalAmmo <= 0) {
        return;
    }
    
    weapon->isReloading = true;
    weapon->reloadTimer = 0.0f;
}

void UpdateWeapon(Weapon* weapon)
{
    // Update fire rate timer
    if (weapon->lastFireTime < weapon->fireRate) {
        weapon->lastFireTime += GetFrameTime();
    }
    
    // Update reload timer
    if (weapon->isReloading) {
        weapon->reloadTimer += GetFrameTime();
        
        if (weapon->reloadTimer >= weapon->reloadTime) {
            // Reload complete
            int ammoNeeded = weapon->maxMagazine - weapon->magazine;
            int ammoToUse = (weapon->totalAmmo < ammoNeeded) ? weapon->totalAmmo : ammoNeeded;
            
            weapon->magazine += ammoToUse;
            weapon->totalAmmo -= ammoToUse;
            
            weapon->isReloading = false;
            weapon->reloadTimer = 0.0f;
        }
    }
    
    // Manual reload request via R key
    if (IsKeyPressed(KEY_R) && !weapon->isReloading && weapon->magazine < weapon->maxMagazine && weapon->totalAmmo > 0) {
        ReloadWeapon(weapon);
    }
}

// Get weapon information
WeaponInfo GetWeaponInfo(const Weapon* weapon)
{
    WeaponInfo info;
    info.magazine = weapon->magazine;
    info.totalAmmo = weapon->totalAmmo;
    info.isReloading = weapon->isReloading;
    
    if (weapon->isReloading) 
    {
        info.reloadProgress = weapon->reloadTimer / weapon->reloadTime;
    } 
    else 
    {
        info.reloadProgress = 0.0f;
    }
    
    return info;
}

// Reloading animation
void DrawReload(WeaponInfo *winfo)
{
    if (winfo->isReloading)
    {
        DrawText("RELOADING", 10, 230, 30, ORANGE);
        DrawRectangle(10, 270, (int)(200 * winfo->reloadProgress), 20, ORANGE);
        DrawRectangleLines(10, 270, 200, 20, WHITE);
    }
    else if (winfo->magazine == 0)
    {
        DrawText("PRESS R TO RELOAD", 10, 230, 25, RED);
    }
}

int GetWeaponBulletPoolSize(const Weapon* weapon)
{
    return weapon->bulletPoolSize;
}
