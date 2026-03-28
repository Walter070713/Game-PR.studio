#include "Map.h"
#include <ctype.h>
#include <math.h>
#include <string.h>

#define RAYTMX_IMPLEMENTATION
#include "../raytmx.h"

// TMX-backed map system: collision checks, door interaction zones, and map switching.
#define MAX_OPENED_DOORS 16

static TmxMap* gTestMap = NULL;
static Rectangle gOpenedDoorZones[MAX_OPENED_DOORS] = {0};
static int gOpenedDoorCount = 0;
static Vector2 gMapOrigin = {0};
static char gActiveMapPath[260] = "TEST MAP.tmx";

static void EnsureTestMapLoaded(void);

// Return currently active TMX path for save/load persistence.
const char* GetActiveTMXMapPath(void)
{
    return gActiveMapPath;
}

// Clear runtime cache of opened door zones.
void ResetOpenedDoorZones(void)
{
    gOpenedDoorCount = 0;
    for (int i = 0; i < MAX_OPENED_DOORS; i++) gOpenedDoorZones[i] = (Rectangle){0};
}

// Number of currently stored opened door zones.
int GetOpenedDoorZoneCount(void)
{
    return gOpenedDoorCount;
}

// Copy opened door zones out for serialization.
int GetOpenedDoorZones(Rectangle outZones[], int maxCount)
{
    int count;

    if (!outZones || maxCount <= 0) return 0;

    count = (gOpenedDoorCount < maxCount) ? gOpenedDoorCount : maxCount;
    for (int i = 0; i < count; i++) outZones[i] = gOpenedDoorZones[i];
    return count;
}

// Restore opened door zones from save data.
void SetOpenedDoorZones(const Rectangle zones[], int count)
{
    int copyCount;

    ResetOpenedDoorZones();
    if (!zones || count <= 0) return;

    copyCount = (count < MAX_OPENED_DOORS) ? count : MAX_OPENED_DOORS;
    for (int i = 0; i < copyCount; i++)
    {
        if (zones[i].width <= 0.0f || zones[i].height <= 0.0f) continue;
        gOpenedDoorZones[gOpenedDoorCount++] = zones[i];
    }
}

// Case-insensitive equality helper for TMX property names.
static bool EqualsIgnoreCase(const char* a, const char* b)
{
    if (!a || !b) return false;

    while (*a && *b)
    {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) return false;
        a++;
        b++;
    }

    return *a == '\0' && *b == '\0';
}

// Read boolean property by name from TMX property array.
static bool GetBoolPropertyByName(const TmxProperty* properties, uint32_t propertyCount, const char* name, bool* outValue)
{
    if (!properties || propertyCount == 0 || !name || !outValue) return false;

    for (uint32_t i = 0; i < propertyCount; i++)
    {
        const TmxProperty* prop = &properties[i];
        if (!prop->name) continue;
        if (!EqualsIgnoreCase(prop->name, name)) continue;
        if (prop->type != PROPERTY_TYPE_BOOL) continue;

        *outValue = prop->boolValue;
        return true;
    }

    return false;
}

// Find tileset-tile metadata entry by local tile id.
static const TmxTilesetTile* FindTilesetTileByLocalId(const TmxTileset* ts, uint32_t localId)
{
    if (!ts) return NULL;

    for (uint32_t i = 0; i < ts->tilesLength; i++)
    {
        const TmxTilesetTile* tile = &ts->tiles[i];
        if (tile->id == localId) return tile;
    }

    return NULL;
}

// Check whether layer contains valid tile-layer payload.
static bool LayerSupportsTileData(const TmxLayer* layer)
{
    if (!layer || layer->type != LAYER_TYPE_TILE_LAYER) return false;
    if (!layer->exact.tileLayer.tiles) return false;
    if (layer->exact.tileLayer.width == 0 || layer->exact.tileLayer.height == 0) return false;
    return true;
}

static uint32_t NormalizeGid(uint32_t rawGid)
{
    // Strip TMX flip/rotation flags from high bits.
    return rawGid & 0x0FFFFFFF;
}

static const TmxTileset* FindTilesetForGid(const TmxMap* map, uint32_t gid)
{
    if (!map || gid == 0) return NULL;

    for (uint32_t i = 0; i < map->tilesetsLength; i++)
    {
        const TmxTileset* ts = &map->tilesets[i];
        if (gid >= ts->firstGid && gid <= ts->lastGid) return ts;
    }

    return NULL;
}

// Resolve collision/door classification by combining layer, tileset, and tile bool properties.
static void ClassifyTileByBoolProperties(const TmxLayer* layer, uint32_t rawGid, bool* outCollision, bool* outDoor)
{
    uint32_t gid = NormalizeGid(rawGid);
    const TmxTileset* ts = NULL;
    bool layerCollision = false;
    bool layerDoor = false;
    bool tilesetCollision = false;
    bool tilesetDoor = false;
    bool tileCollision = false;
    bool tileDoor = false;

    if (outCollision) *outCollision = false;
    if (outDoor) *outDoor = false;

    if (!gTestMap || !layer || gid == 0) return;

    ts = FindTilesetForGid(gTestMap, gid);
    if (!ts) return;

    (void)GetBoolPropertyByName(layer->properties, layer->propertiesLength, "collision", &layerCollision);
    (void)GetBoolPropertyByName(layer->properties, layer->propertiesLength, "door", &layerDoor);

    (void)GetBoolPropertyByName(ts->properties, ts->propertiesLength, "collision", &tilesetCollision);
    (void)GetBoolPropertyByName(ts->properties, ts->propertiesLength, "door", &tilesetDoor);

    {
        uint32_t localId = gid - ts->firstGid;
        const TmxTilesetTile* tile = FindTilesetTileByLocalId(ts, localId);
        if (tile)
        {
            (void)GetBoolPropertyByName(tile->properties, tile->propertiesLength, "collision", &tileCollision);
            (void)GetBoolPropertyByName(tile->properties, tile->propertiesLength, "door", &tileDoor);
        }
    }

    if (outDoor) *outDoor = layerDoor || tilesetDoor || tileDoor;
    if (outCollision)
    {
        bool isDoor = layerDoor || tilesetDoor || tileDoor;
        *outCollision = isDoor || layerCollision || tilesetCollision || tileCollision;
    }
}

// True when tile should block movement/projectiles.
static bool IsTileCollision(const TmxLayer* layer, uint32_t rawGid)
{
    bool collision = false;
    ClassifyTileByBoolProperties(layer, rawGid, &collision, NULL);
    return collision;
}

// True when tile represents an interactable/openable door.
static bool IsTileDoor(const TmxLayer* layer, uint32_t rawGid)
{
    bool door = false;
    ClassifyTileByBoolProperties(layer, rawGid, NULL, &door);
    return door;
}

// Distance from point to rectangle edge (0 when inside rectangle).
static float DistancePointToRect(Vector2 p, Rectangle r)
{
    float dx = 0.0f;
    float dy = 0.0f;

    if (p.x < r.x) dx = r.x - p.x;
    else if (p.x > r.x + r.width) dx = p.x - (r.x + r.width);

    if (p.y < r.y) dy = r.y - p.y;
    else if (p.y > r.y + r.height) dy = p.y - (r.y + r.height);

    return sqrtf(dx * dx + dy * dy);
}

// Fuzzy rectangle equality for door zone comparisons.
static bool IsSameDoorZone(Rectangle a, Rectangle b)
{
    float eps = 2.0f;
    return fabsf(a.x - b.x) <= eps && fabsf(a.y - b.y) <= eps &&
        fabsf(a.width - b.width) <= eps && fabsf(a.height - b.height) <= eps;
}

// Skip collisions on doors already opened in runtime state.
static bool IsTileInsideAnyOpenedDoor(Rectangle tileRect)
{
    for (int i = 0; i < gOpenedDoorCount; i++)
    {
        if (CheckCollisionRecs(tileRect, gOpenedDoorZones[i])) return true;
    }

    return false;
}

// Expand a single touched door tile into a merged nearby door cluster zone.
static bool BuildOpenedDoorZone(Rectangle triggerZone, Rectangle* outZone)
{
    if (!outZone || !gTestMap) return false;

    {
        float tileW = (float)gTestMap->tileWidth;
        float tileH = (float)gTestMap->tileHeight;
        Rectangle search = {
            triggerZone.x - tileW * 1.25f,
            triggerZone.y - tileH * 2.0f,
            triggerZone.width + tileW * 2.5f,
            triggerZone.height + tileH * 4.0f
        };
        bool found = false;
        Rectangle bounds = {0};

        for (uint32_t li = 0; li < gTestMap->layersLength; li++)
        {
            const TmxLayer* layer = &gTestMap->layers[li];
            if (!LayerSupportsTileData(layer)) continue;

            {
                uint32_t w = layer->exact.tileLayer.width;
                uint32_t h = layer->exact.tileLayer.height;
                const uint32_t* tiles = layer->exact.tileLayer.tiles;
                float ox = (float)layer->offsetX;
                float oy = (float)layer->offsetY;

                if (!tiles || w == 0 || h == 0) continue;

                for (uint32_t row = 0; row < h; row++)
                {
                    for (uint32_t col = 0; col < w; col++)
                    {
                        uint32_t rawGid = tiles[row * w + col];
                        if (!IsTileDoor(layer, rawGid)) continue;

                        {
                            Rectangle tileRect = {
                                gMapOrigin.x + ox + (float)col * tileW,
                                gMapOrigin.y + oy + (float)row * tileH,
                                tileW,
                                tileH
                            };

                            // Note: triggerZone is in world space; tileRect gets shifted below by map origin at call site.
                            if (!CheckCollisionRecs(tileRect, search)) continue;

                            if (!found)
                            {
                                bounds = tileRect;
                                found = true;
                            }
                            else
                            {
                                float minX = (bounds.x < tileRect.x) ? bounds.x : tileRect.x;
                                float minY = (bounds.y < tileRect.y) ? bounds.y : tileRect.y;
                                float maxX = ((bounds.x + bounds.width) > (tileRect.x + tileRect.width)) ?
                                    (bounds.x + bounds.width) : (tileRect.x + tileRect.width);
                                float maxY = ((bounds.y + bounds.height) > (tileRect.y + tileRect.height)) ?
                                    (bounds.y + bounds.height) : (tileRect.y + tileRect.height);

                                bounds.x = minX;
                                bounds.y = minY;
                                bounds.width = maxX - minX;
                                bounds.height = maxY - minY;
                            }
                        }
                    }
                }
            }
        }

        if (!found) return false;

        *outZone = (Rectangle){
            bounds.x - tileW * 0.2f,
            bounds.y - tileH * 0.2f,
            bounds.width + tileW * 0.4f,
            bounds.height + tileH * 0.4f
        };
        return true;
    }
}


// Broadphase + tile scan collision test against active TMX solid tiles.
static bool CheckCircleAgainstSolidTiles(Vector2 center, float radius, Rectangle mapBounds)
{
    if (!gTestMap) return false;

    {
        float tileW = (float)gTestMap->tileWidth;
        float tileH = (float)gTestMap->tileHeight;

        if (tileW <= 0.0f || tileH <= 0.0f) return false;

        Rectangle aabb = {
            center.x - radius,
            center.y - radius,
            radius * 2.0f,
            radius * 2.0f
        };

        for (uint32_t li = 0; li < gTestMap->layersLength; li++)
        {
            const TmxLayer* layer = &gTestMap->layers[li];
            if (!LayerSupportsTileData(layer)) continue;

            {
                float layerOriginX = mapBounds.x + (float)layer->offsetX;
                float layerOriginY = mapBounds.y + (float)layer->offsetY;
                uint32_t layerW = layer->exact.tileLayer.width;
                uint32_t layerH = layer->exact.tileLayer.height;
                const uint32_t* tiles = layer->exact.tileLayer.tiles;

                if (!tiles || layerW == 0 || layerH == 0) continue;

                int minCol = (int)floorf((aabb.x - layerOriginX) / tileW);
                int maxCol = (int)floorf((aabb.x + aabb.width - layerOriginX) / tileW);
                int minRow = (int)floorf((aabb.y - layerOriginY) / tileH);
                int maxRow = (int)floorf((aabb.y + aabb.height - layerOriginY) / tileH);

                if (minCol < 0) minCol = 0;
                if (minRow < 0) minRow = 0;
                if (maxCol >= (int)layerW) maxCol = (int)layerW - 1;
                if (maxRow >= (int)layerH) maxRow = (int)layerH - 1;

                if (minCol > maxCol || minRow > maxRow) continue;

                for (int row = minRow; row <= maxRow; row++)
                {
                    for (int col = minCol; col <= maxCol; col++)
                    {
                        uint32_t rawGid = tiles[(uint32_t)row * layerW + (uint32_t)col];
                        if (!IsTileCollision(layer, rawGid)) continue;

                        {
                            Rectangle tileRect = {
                                layerOriginX + (float)col * tileW,
                                layerOriginY + (float)row * tileH,
                                tileW,
                                tileH
                            };

                            if (gOpenedDoorCount > 0 && IsTileInsideAnyOpenedDoor(tileRect))
                            {
                                continue;
                            }

                            if (CheckCollisionCircleRec(center, radius, tileRect)) return true;
                        }
                    }
                }
            }
        }
    }

    return false;
}

// Lazy-load active TMX map if not already loaded.
static void EnsureTestMapLoaded(void)
{
    if (gTestMap) return;
    gTestMap = LoadTMX(gActiveMapPath);
}

// Switch active TMX map and reset map-specific runtime door state.
bool SetActiveTMXMap(const char* mapPath)
{
    if (!mapPath || mapPath[0] == '\0') return false;

    if (gTestMap && strcmp(gActiveMapPath, mapPath) == 0) return true;

    {
        TmxMap* newMap = LoadTMX(mapPath);
        if (!newMap) return false;

        if (gTestMap) UnloadTMX(gTestMap);
        gTestMap = newMap;

        strncpy(gActiveMapPath, mapPath, sizeof(gActiveMapPath) - 1);
        gActiveMapPath[sizeof(gActiveMapPath) - 1] = '\0';

        ResetOpenedDoorZones();
    }

    return true;
}

// Initialize runtime GameMap bounds from currently active TMX metadata.
static GameMap InitMapFromTMXAt(Vector2 origin, Color fallbackFloor, Color fallbackWall)
{
    EnsureTestMapLoaded();
    gMapOrigin = origin;

    if (gTestMap)
    {
        return CreateMapLayout(
            (Rectangle){
                origin.x,
                origin.y,
                (float)(gTestMap->width * gTestMap->tileWidth),
                (float)(gTestMap->height * gTestMap->tileHeight)
            },
            fallbackFloor,
            fallbackWall
        );
    }

    // Fallback bounds if TMX load fails; keeps game running with visible warning room.
    return CreateMapLayout((Rectangle){origin.x, origin.y, 1200.0f, 800.0f}, fallbackFloor, fallbackWall);
}

// Generic map constructor used by all chapter/level room presets.
GameMap CreateMapLayout(Rectangle bounds, Color floorColor, Color wallColor)
{
    GameMap map = {0};
    map.bounds = bounds;
    map.FloorColor = floorColor;
    map.WallColor = wallColor;
    map.WallCount = 0;
    return map;
}

// Build the active room from current TMX map at default world origin.
GameMap InitRoom(void)
{
    return InitMapFromTMXAt((Vector2){1000.0f, 1000.0f}, DARKGRAY, GRAY);
}

// Keep bounds synchronized with loaded TMX dimensions.
void SyncMapToWindow(GameMap* map)
{
    if (!map) return;

    EnsureTestMapLoaded();
    if (!gTestMap) return;

    // Map world size stays in TMX units; window fitting is handled by camera zoom in main.
    map->bounds.width = (float)(gTestMap->width * gTestMap->tileWidth);
    map->bounds.height = (float)(gTestMap->height * gTestMap->tileHeight);
}

// Return tile width of active TMX map; fallback to 16.
float GetMapTileSize(void)
{
    EnsureTestMapLoaded();

    if (!gTestMap) return 16.0f;
    return (float)gTestMap->tileWidth;
}

// Circle collision query against active TMX solids.
bool IsMapCircleBlocked(Vector2 center, float radius, Rectangle mapBounds)
{
    EnsureTestMapLoaded();
    return CheckCircleAgainstSolidTiles(center, radius, mapBounds);
}

// Point collision query against active TMX solids.
bool IsMapPointBlocked(Vector2 point, Rectangle mapBounds)
{
    EnsureTestMapLoaded();
    return CheckCircleAgainstSolidTiles(point, 0.1f, mapBounds);
}

// Find nearest door interaction zone around a position within max distance.
bool FindNearbyDoorInteractZone(Rectangle mapBounds, Vector2 fromPos, float maxDistance, Rectangle* outDoor)
{
    if (!outDoor) return false;
    EnsureTestMapLoaded();
    if (!gTestMap) return false;

    {
        float tileW = (float)gTestMap->tileWidth;
        float tileH = (float)gTestMap->tileHeight;
        float bestDist = maxDistance;
        bool found = false;
        Rectangle bestRect = (Rectangle){0};

        for (uint32_t li = 0; li < gTestMap->layersLength; li++)
        {
            const TmxLayer* layer = &gTestMap->layers[li];
            if (!LayerSupportsTileData(layer)) continue;

            {
                uint32_t w = layer->exact.tileLayer.width;
                uint32_t h = layer->exact.tileLayer.height;
                const uint32_t* tiles = layer->exact.tileLayer.tiles;
                float ox = mapBounds.x + (float)layer->offsetX;
                float oy = mapBounds.y + (float)layer->offsetY;

                if (!tiles || w == 0 || h == 0) continue;

                for (uint32_t row = 0; row < h; row++)
                {
                    for (uint32_t col = 0; col < w; col++)
                    {
                        uint32_t rawGid = tiles[row * w + col];
                        if (!IsTileDoor(layer, rawGid)) continue;

                        {
                            Rectangle zone = {
                                ox + (float)col * tileW - tileW * 0.35f,
                                oy + (float)row * tileH - tileH * 0.35f,
                                tileW * 1.7f,
                                tileH * 1.7f
                            };
                            float dist = DistancePointToRect(fromPos, zone);

                            if (dist <= bestDist)
                            {
                                bestDist = dist;
                                bestRect = zone;
                                found = true;
                            }
                        }
                    }
                }
            }
        }

        if (!found) return false;
        *outDoor = bestRect;
        return true;
    }
}

// Mark a door area opened so its tiles stop blocking movement.
void OpenDoorInteractZone(Rectangle doorZone)
{
    Rectangle zoneToStore = doorZone;

    if (doorZone.width <= 0.0f || doorZone.height <= 0.0f) return;

    // Expand opening from touched tile to the full nearby door cluster.
    BuildOpenedDoorZone(doorZone, &zoneToStore);

    for (int i = 0; i < gOpenedDoorCount; i++)
    {
        if (IsSameDoorZone(gOpenedDoorZones[i], zoneToStore) || CheckCollisionRecs(gOpenedDoorZones[i], zoneToStore)) return;
    }

    if (gOpenedDoorCount >= MAX_OPENED_DOORS) return;
    gOpenedDoorZones[gOpenedDoorCount++] = zoneToStore;
}

// Query whether a door zone overlaps any opened door zone.
bool IsDoorInteractZoneOpened(Rectangle doorZone)
{
    for (int i = 0; i < gOpenedDoorCount; i++)
    {
        if (IsSameDoorZone(gOpenedDoorZones[i], doorZone) || CheckCollisionRecs(gOpenedDoorZones[i], doorZone)) return true;
    }

    return false;
}

// Draw active map using TMX renderer (or fallback room if map missing).
void DrawMap(GameMap map)
{
    EnsureTestMapLoaded();

    if (gTestMap)
    {
        // TMX draws tiles at pixel-per-tile size. Animate and render with current camera.
        AnimateTMX(gTestMap);
        DrawTMX(gTestMap, NULL, NULL, (int)map.bounds.x, (int)map.bounds.y, WHITE);
        return;
    }

    // Fallback draw if TMX cannot be loaded.
    DrawRectangleRec(map.bounds, map.FloorColor);
    DrawRectangleLinesEx(map.bounds, 8.0f, MAROON);
}

// Release TMX resources and clear opened-door runtime cache.
void ShutdownMapSystem(void)
{
    if (!gTestMap) return;
    UnloadTMX(gTestMap);
    gTestMap = NULL;
    ResetOpenedDoorZones();
}