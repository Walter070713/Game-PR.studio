#include "Map.h"
#include <ctype.h>
#include <math.h>
#include <string.h>

#define RAYTMX_IMPLEMENTATION
#include "../raytmx.h"

#define MAX_OPENED_DOORS 16

static TmxMap* gTestMap = NULL;
static Rectangle gOpenedDoorZones[MAX_OPENED_DOORS] = {0};
static int gOpenedDoorCount = 0;
static Vector2 gMapOrigin = {0};
static char gActiveMapPath[260] = "TEST MAP.tmx";

static void EnsureTestMapLoaded(void);

static bool ContainsIgnoreCase(const char* text, const char* needle)
{
    if (!text || !needle) return false;

    {
        size_t nlen = strlen(needle);
        size_t tlen = strlen(text);
        if (nlen == 0 || nlen > tlen) return false;

        for (size_t i = 0; i + nlen <= tlen; i++)
        {
            bool matched = true;
            for (size_t j = 0; j < nlen; j++)
            {
                if (tolower((unsigned char)text[i + j]) != tolower((unsigned char)needle[j]))
                {
                    matched = false;
                    break;
                }
            }
            if (matched) return true;
        }
    }

    return false;
}

static bool IsTilesetSolidByName(const char* name)
{
    if (!name) return false;

    if (ContainsIgnoreCase(name, "floor") || ContainsIgnoreCase(name, "carpet")) return false;

    return ContainsIgnoreCase(name, "wall") ||
        ContainsIgnoreCase(name, "door") ||
        ContainsIgnoreCase(name, "decor") ||
        ContainsIgnoreCase(name, "tree") ||
        ContainsIgnoreCase(name, "furniture") ||
        ContainsIgnoreCase(name, "chair") ||
        ContainsIgnoreCase(name, "bed") ||
        ContainsIgnoreCase(name, "table") ||
        ContainsIgnoreCase(name, "window");
}

static bool IsDoorTileset(const TmxTileset* ts)
{
    if (!ts) return false;

    if (ContainsIgnoreCase(ts->name, "door")) return true;
    if (ts->hasImage && ts->image.source && ContainsIgnoreCase(ts->image.source, "door")) return true;

    return false;
}

static bool IsLayerCollisionCandidate(const TmxLayer* layer)
{
    if (!layer || layer->type != LAYER_TYPE_TILE_LAYER || !layer->visible) return false;

    // Exclude decorative overlays explicitly.
    if (ContainsIgnoreCase(layer->name, "spider") || ContainsIgnoreCase(layer->name, "web") ||
        ContainsIgnoreCase(layer->name, "tale") || ContainsIgnoreCase(layer->name, "carpet")) return false;

    // Near-invisible layers should not participate in collision.
    if (layer->opacity <= 0.05) return false;

    return ContainsIgnoreCase(layer->name, "wall") ||
        ContainsIgnoreCase(layer->name, "door") ||
        ContainsIgnoreCase(layer->name, "decor") ||
        ContainsIgnoreCase(layer->name, "furniture") ||
        ContainsIgnoreCase(layer->name, "tree") ||
        ContainsIgnoreCase(layer->name, "object");
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

static bool IsSolidRawGid(const TmxMap* map, uint32_t rawGid)
{
    uint32_t gid = NormalizeGid(rawGid);
    const TmxTileset* ts = NULL;

    if (gid == 0) return false;

    ts = FindTilesetForGid(map, gid);
    if (!ts) return false;

    return IsTilesetSolidByName(ts->name);
}

static bool IsDoorRawGid(const TmxMap* map, uint32_t rawGid)
{
    uint32_t gid = NormalizeGid(rawGid);
    const TmxTileset* ts = NULL;

    if (gid == 0) return false;

    ts = FindTilesetForGid(map, gid);
    if (!ts) return false;

    return IsDoorTileset(ts);
}

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

static bool IsSameDoorZone(Rectangle a, Rectangle b)
{
    float eps = 2.0f;
    return fabsf(a.x - b.x) <= eps && fabsf(a.y - b.y) <= eps &&
        fabsf(a.width - b.width) <= eps && fabsf(a.height - b.height) <= eps;
}

static bool IsTileInsideAnyOpenedDoor(Rectangle tileRect)
{
    for (int i = 0; i < gOpenedDoorCount; i++)
    {
        if (CheckCollisionRecs(tileRect, gOpenedDoorZones[i])) return true;
    }

    return false;
}

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
            if (!layer || layer->type != LAYER_TYPE_TILE_LAYER || !layer->visible) continue;

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
                        if (!IsDoorRawGid(gTestMap, rawGid)) continue;

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
            if (!IsLayerCollisionCandidate(layer)) continue;

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
                        if (!IsSolidRawGid(gTestMap, rawGid)) continue;

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

static void EnsureTestMapLoaded(void)
{
    if (gTestMap) return;
    gTestMap = LoadTMX(gActiveMapPath);
}

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

        gOpenedDoorCount = 0;
        for (int i = 0; i < MAX_OPENED_DOORS; i++) gOpenedDoorZones[i] = (Rectangle){0};
    }

    return true;
}

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

// Adds one wall safely; returns false when MAX_WALLS is reached.
bool AddMapWall(GameMap* map, Rectangle wall)
{
    if (!map || map->WallCount >= MAX_WALLS) return false;
    map->walls[map->WallCount++] = wall;
    return true;
}

GameMap InitRoom(void)
{
    return InitMapFromTMXAt((Vector2){1000.0f, 1000.0f}, DARKGRAY, GRAY);
}

void SyncMapToWindow(GameMap* map)
{
    if (!map) return;

    EnsureTestMapLoaded();
    if (!gTestMap) return;

    // Map world size stays in TMX units; window fitting is handled by camera zoom in main.
    map->bounds.width = (float)(gTestMap->width * gTestMap->tileWidth);
    map->bounds.height = (float)(gTestMap->height * gTestMap->tileHeight);
}

float GetMapTileSize(void)
{
    EnsureTestMapLoaded();

    if (!gTestMap) return 16.0f;
    return (float)gTestMap->tileWidth;
}

bool IsMapCircleBlocked(Vector2 center, float radius, Rectangle mapBounds)
{
    EnsureTestMapLoaded();
    return CheckCircleAgainstSolidTiles(center, radius, mapBounds);
}

bool IsMapPointBlocked(Vector2 point, Rectangle mapBounds)
{
    EnsureTestMapLoaded();
    return CheckCircleAgainstSolidTiles(point, 0.1f, mapBounds);
}

bool FindPrimaryDoorInteractZone(Rectangle mapBounds, Rectangle* outDoor)
{
    if (!outDoor) return false;
    EnsureTestMapLoaded();
    if (!gTestMap) return false;

    {
        float tileW = (float)gTestMap->tileWidth;
        float tileH = (float)gTestMap->tileHeight;
        float bestX = -1.0f;
        bool found = false;
        Rectangle bestRect = (Rectangle){0};

        for (uint32_t li = 0; li < gTestMap->layersLength; li++)
        {
            const TmxLayer* layer = &gTestMap->layers[li];
            if (!layer || layer->type != LAYER_TYPE_TILE_LAYER || !layer->visible) continue;
            if (!ContainsIgnoreCase(layer->name, "door")) continue;

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
                        uint32_t gid = NormalizeGid(rawGid);
                        const TmxTileset* ts = FindTilesetForGid(gTestMap, gid);
                        if (gid == 0 || !ts || !IsDoorTileset(ts)) continue;

                        {
                            Rectangle r = {ox + (float)col * tileW, oy + (float)row * tileH, tileW, tileH};
                            if (r.x > bestX)
                            {
                                bestX = r.x;
                                bestRect = r;
                                found = true;
                            }
                        }
                    }
                }
            }
        }

        if (!found) return false;

        *outDoor = (Rectangle){
            bestRect.x - tileW * 0.35f,
            bestRect.y - tileH * 0.35f,
            bestRect.width + tileW * 0.7f,
            bestRect.height + tileH * 0.7f
        };
        return true;
    }
}

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
            if (!layer || layer->type != LAYER_TYPE_TILE_LAYER || !layer->visible) continue;

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
                        uint32_t gid = NormalizeGid(rawGid);
                        const TmxTileset* ts = FindTilesetForGid(gTestMap, gid);
                        if (gid == 0 || !ts || !IsDoorTileset(ts)) continue;

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

bool HasOpenedDoor(void)
{
    return gOpenedDoorCount > 0;
}

bool IsDoorInteractZoneOpened(Rectangle doorZone)
{
    for (int i = 0; i < gOpenedDoorCount; i++)
    {
        if (IsSameDoorZone(gOpenedDoorZones[i], doorZone) || CheckCollisionRecs(gOpenedDoorZones[i], doorZone)) return true;
    }

    return false;
}

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

void ShutdownMapSystem(void)
{
    if (!gTestMap) return;
    UnloadTMX(gTestMap);
    gTestMap = NULL;
    gOpenedDoorCount = 0;
    for (int i = 0; i < MAX_OPENED_DOORS; i++) gOpenedDoorZones[i] = (Rectangle){0};
}