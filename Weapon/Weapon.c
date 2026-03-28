#include "Weapon.h"

// Keep weapon type inside valid enum range before indexing preset table.
static WeaponType SanitizeWeaponType(WeaponType type)
{
    if ((int)type < 0 || (int)type >= WEAPON_COUNT) return WEAPON_PISTOL;
    return type;
}

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
} weaponPresets[WEAPON_COUNT] = {
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
    WeaponType safeType = SanitizeWeaponType(type);
    Weapon w = {0};
    w.type = safeType;
    
    // Load preset values
    w.bulletSpeed = weaponPresets[safeType].bulletSpeed;
    w.bulletSize = weaponPresets[safeType].bulletSize;
    w.bulletPoolSize = weaponPresets[safeType].bulletPoolSize;
    w.maxMagazine = weaponPresets[safeType].maxMagazine;
    w.totalAmmo = weaponPresets[safeType].totalAmmo;
    w.fireRate = weaponPresets[safeType].fireRate;
    w.reloadTime = weaponPresets[safeType].reloadTime;
    w.bulletColor = weaponPresets[safeType].bulletColor;
    
    // Start with full magazine
    w.magazine = w.maxMagazine;
    w.lastFireTime = 0.0f;
    w.reloadTimer = 0.0f;
    w.isReloading = false;
    
    return w;
}

// Fire-gate check + ammo consumption for one shot attempt.
bool FireWeapon(Weapon* weapon)
{
    if (!weapon) return false;

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

// Begin reload sequence if state allows it.
void ReloadWeapon(Weapon* weapon)
{
    if (!weapon) return;

    // Already reloading or no ammo to reload
    if (weapon->isReloading || weapon->totalAmmo <= 0) {
        return;
    }
    
    weapon->isReloading = true;
    weapon->reloadTimer = 0.0f;
}

// Update fire/reload timers and process manual reload input.
void UpdateWeapon(Weapon* weapon)
{
    float dt;

    if (!weapon) return;

    dt = GetFrameTime();

    // Update fire rate timer
    if (weapon->lastFireTime < weapon->fireRate) {
        weapon->lastFireTime += dt;
    }
    
    // Update reload timer
    if (weapon->isReloading) {
        weapon->reloadTimer += dt;
        
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

// Extract read-only values for HUD rendering.
WeaponInfo GetWeaponInfo(const Weapon* weapon)
{
    WeaponInfo info = {0};
    if (!weapon) return info;

    info.magazine = weapon->magazine;
    info.totalAmmo = weapon->totalAmmo;
    info.isReloading = weapon->isReloading;
    
    if (weapon->isReloading) 
    {
        if (weapon->reloadTime > 0.0f)
        {
            info.reloadProgress = weapon->reloadTimer / weapon->reloadTime;
        }
        else
        {
            info.reloadProgress = 1.0f;
        }
    } 
    else 
    {
        info.reloadProgress = 0.0f;
    }

    if (info.reloadProgress < 0.0f) info.reloadProgress = 0.0f;
    if (info.reloadProgress > 1.0f) info.reloadProgress = 1.0f;
    
    return info;
}

// Draw reload bar/hints near HUD ammo readout.
void DrawReload(const WeaponInfo* winfo)
{
    if (!winfo) return;

    if (winfo->isReloading)
    {
        DrawText("RELOADING", 10, 230, 30, ORANGE);
        DrawRectangle(10, 270, (int)(200 * winfo->reloadProgress), 20, ORANGE);
        DrawRectangleLines(10, 270, 200, 20, WHITE);
    }
    else if (winfo->magazine == 0 && winfo->totalAmmo > 0)
    {
        DrawText("PRESS R TO RELOAD", 10, 230, 25, RED);
    }
}

// Return configured max in-flight projectile slots for current weapon.
int GetWeaponBulletPoolSize(const Weapon* weapon)
{
    if (!weapon) return 0;
    return weapon->bulletPoolSize;
}
