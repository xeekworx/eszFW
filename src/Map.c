/**
 * @file Map.c
 * @ingroup Map
 * @defgroup Map
 * @author Michael Fitzmayer
 * @copyright "THE BEER-WARE LICENCE" (Revision 42)
 */

#include <SDL.h>
#include <SDL_image.h>
#include "Map.h"

static Uint16 _ClearGidFlags(Uint16 u16Gid)
{
    return u16Gid & TMX_FLIP_BITS_REMOVAL;
}

static void _FindObjectByName(const char *pacName, tmx_object *pstTmxObject, Object **pstObject)
{
    if (pstObject)
    {
        if (0 == SDL_strncmp(pacName, pstTmxObject->name, 20))
        {
            (*pstObject)->dPosX = pstTmxObject->x;
            (*pstObject)->dPosY = pstTmxObject->y;
        }
        else if (pstTmxObject && pstTmxObject->next)
        {
            _FindObjectByName(pacName, pstTmxObject->next, pstObject);
        }
    }
}

static void _GetObjectCount(tmx_object *pstObject, Uint16 **pu16ObjectCount)
{
    if (pstObject)
    {
        (**pu16ObjectCount)++;
    }

    if (pstObject && pstObject->next)
    {
        _GetObjectCount(pstObject->next, pu16ObjectCount);
    }
}

static void _GetGravitation(tmx_property *pProperty, void *dGravitation)
{
    if (0 == SDL_strncmp(pProperty->name, "Gravitation", 11))
    {
        if (PT_FLOAT == pProperty->type)
        {
            *((double*)dGravitation) = pProperty->value.decimal;
        }
    }
}

/**
 * @brief Draw Map.
 * @param u16Index         the texture index.  The total amount of layers per map is defined by MAP_TEXTURES.
 * @param bRenderAnimTiles if set to 1, all animated tiles will be rendered in this call.
 * @param bRenderBgColour  determine if the map's background colour should be rendered
 * @param pacLayerName     substring of the layer(s) to render.
 * @param dCameraPosX      camera position along the x-axis.
 * @param dCameraPosY      camera position along the y-axis.
 * @param dDeltaTime       time since last frame in seconds.
 * @param pstMap           pointer to Map structure.
 * @param pstRenderer      pointer to SDL rendering context.
 * @return  0 on success, -1 on failure.
 * @ingroup Map
 */
Sint8 DrawMap(
    const Uint16   u16Index,
    const SDL_bool bRenderAnimTiles,
    const SDL_bool bRenderBgColour,
    const char    *pacLayerName,
    const double   dCameraPosX,
    const double   dCameraPosY,
    const double   dDeltaTime,
    Map           *pstMap,
    SDL_Renderer  *pstRenderer)
{
    tmx_layer *pstLayer = pstMap->pstTmxMap->ly_head;

    // Load tileset image once.
    if (! pstMap->pstTileset)
    {
        pstMap->pstTileset = IMG_LoadTexture(pstRenderer, pstMap->acTilesetImage);
        if (! pstMap->pstTileset)
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", IMG_GetError());
            return -1;
        }
    }

    // Update and render animated tiles.
    pstMap->dAnimDelay += dDeltaTime;

    if (0 < pstMap->u16AnimTileSize && pstMap->dAnimDelay > 1.f / pstMap->dAnimSpeed && bRenderAnimTiles)
    {
        pstMap->pstAnimTexture = SDL_CreateTexture(
            pstRenderer,
            SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_TARGET,
            pstMap->pstTmxMap->width  * pstMap->pstTmxMap->tile_width,
            pstMap->pstTmxMap->height * pstMap->pstTmxMap->tile_height);

        if (! pstMap->pstAnimTexture)
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
            return -1;
        }

        if (0 != SDL_SetRenderTarget(pstRenderer, pstMap->pstAnimTexture))
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
            return -1;
        }

        for (Uint16 u16Index = 0; u16Index < pstMap->u16AnimTileSize; u16Index++)
        {
            Uint16       u16Gid        = pstMap->acAnimTile[u16Index].u16Gid;
            Uint16       u16TileId     = pstMap->acAnimTile[u16Index].u16TileId + 1;
            Uint16       u16NextTileId = 0;
            SDL_Rect     stDst;
            SDL_Rect     stSrc;
            tmx_tileset *pstTS;

            pstTS    = pstMap->pstTmxMap->tiles[1]->tileset;
            stSrc.x  = pstMap->pstTmxMap->tiles[u16TileId]->ul_x;
            stSrc.y  = pstMap->pstTmxMap->tiles[u16TileId]->ul_y;
            stSrc.w  = stDst.w = pstTS->tile_width;
            stSrc.h  = stDst.h = pstTS->tile_height;
            stDst.x  = pstMap->acAnimTile[u16Index].s16DstX;
            stDst.y  = pstMap->acAnimTile[u16Index].s16DstY;
            SDL_RenderCopy(pstRenderer, pstMap->pstTileset, &stSrc, &stDst);

            pstMap->acAnimTile[u16Index].u8FrameCount++;
            if (pstMap->acAnimTile[u16Index].u8FrameCount >= pstMap->acAnimTile[u16Index].u8AnimLen)
            {
                pstMap->acAnimTile[u16Index].u8FrameCount = 0;
            }

            u16NextTileId = pstMap->pstTmxMap->tiles[u16Gid]->animation[pstMap->acAnimTile[u16Index].u8FrameCount].tile_id;
            pstMap->acAnimTile[u16Index].u16TileId = u16NextTileId;
        }

        if (pstMap->dAnimDelay > 1.f / pstMap->dAnimSpeed)
        {
            pstMap->dAnimDelay = 0.f;
        }

        // Switch back to default render target.
        if (0 != SDL_SetRenderTarget(pstRenderer, NULL))
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        return -1;
        }

        if (0 != SDL_SetTextureBlendMode(pstMap->pstAnimTexture, SDL_BLENDMODE_BLEND))
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
            return -1;
        }
    }

    // The texture has already been rendered and can be drawn.
    if (pstMap->pstTexture[u16Index])
    {
        double   dRenderPosX = pstMap->dPosX - dCameraPosX;
        double   dRenderPosY = pstMap->dPosY - dCameraPosY;
        SDL_Rect stDst =
        {
            dRenderPosX,
            dRenderPosY,
            pstMap->pstTmxMap->width  * pstMap->pstTmxMap->tile_width,
            pstMap->pstTmxMap->height * pstMap->pstTmxMap->tile_height
        };

        if (-1 == SDL_RenderCopyEx(
                pstRenderer, pstMap->pstTexture[u16Index],
                NULL, &stDst, 0, NULL, SDL_FLIP_NONE))
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
            return -1;
        }

        if (bRenderAnimTiles)
        {
            if (pstMap->pstAnimTexture)
            {
                if (-1 == SDL_RenderCopyEx(
                        pstRenderer, pstMap->pstAnimTexture,
                        NULL, &stDst, 0, NULL, SDL_FLIP_NONE))
                {
                    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
                    return -1;
                }
            }
        }

        return 0;
    }

    // Otherwise render the texture once.
    pstMap->pstTexture[u16Index] = SDL_CreateTexture(
        pstRenderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_TARGET,
        pstMap->pstTmxMap->width  * pstMap->pstTmxMap->tile_width,
        pstMap->pstTmxMap->height * pstMap->pstTmxMap->tile_height);

    if (! pstMap->pstTexture[u16Index])
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        return -1;
    }

    if (0 != SDL_SetRenderTarget(pstRenderer, pstMap->pstTexture[u16Index]))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        return -1;
    }

    if (bRenderBgColour)
    {
        SDL_SetRenderDrawColor(
            pstRenderer,
            (pstMap->pstTmxMap->backgroundcolor >> 16) & 0xFF,
            (pstMap->pstTmxMap->backgroundcolor >>  8) & 0xFF,
            (pstMap->pstTmxMap->backgroundcolor)       & 0xFF,
            255);
    }

    pstLayer = pstMap->pstTmxMap->ly_head;
    while(pstLayer)
    {
        SDL_bool     bRenderLayer = 1;
        Uint16       u16Gid;
        SDL_Rect     stDst;
        SDL_Rect     stSrc;
        tmx_tileset *pstTS;

        if (L_LAYER == pstLayer->type)
        {
            if (pacLayerName)
            {
                if (! SDL_strstr(pstLayer->name, pacLayerName))
                {
                    bRenderLayer = 0;
                }
            }
            if (pstLayer->visible && bRenderLayer)
            {
                for (Uint16 u16IndexH = 0; u16IndexH < pstMap->pstTmxMap->height; u16IndexH++)
                {
                    for (Uint32 u16IndexW = 0; u16IndexW < pstMap->pstTmxMap->width; u16IndexW++)
                    {
                        u16Gid = _ClearGidFlags(pstLayer->content.gids[(u16IndexH * pstMap->pstTmxMap->width) + u16IndexW]);
                        if (pstMap->pstTmxMap->tiles[u16Gid])
                        {
                            pstTS    = pstMap->pstTmxMap->tiles[1]->tileset;
                            stSrc.x  = pstMap->pstTmxMap->tiles[u16Gid]->ul_x;
                            stSrc.y  = pstMap->pstTmxMap->tiles[u16Gid]->ul_y;
                            stSrc.w  = stDst.w   = pstTS->tile_width;
                            stSrc.h  = stDst.h   = pstTS->tile_height;
                            stDst.x  = u16IndexW * pstTS->tile_width;
                            stDst.y  = u16IndexH * pstTS->tile_height;
                            SDL_RenderCopy(pstRenderer, pstMap->pstTileset, &stSrc, &stDst);

                            if (bRenderAnimTiles && pstMap->pstTmxMap->tiles[u16Gid]->animation)
                            {
                                Uint8  u8AnimLen;
                                Uint16 u16TileId;
                                u8AnimLen = pstMap->pstTmxMap->tiles[u16Gid]->animation_len;
                                u16TileId = pstMap->pstTmxMap->tiles[u16Gid]->animation[0].tile_id;
                                pstMap->acAnimTile[pstMap->u16AnimTileSize].u16Gid       = u16Gid;
                                pstMap->acAnimTile[pstMap->u16AnimTileSize].u16TileId    = u16TileId;
                                pstMap->acAnimTile[pstMap->u16AnimTileSize].s16DstX      = stDst.x;
                                pstMap->acAnimTile[pstMap->u16AnimTileSize].s16DstY      = stDst.y;
                                pstMap->acAnimTile[pstMap->u16AnimTileSize].u8FrameCount = 0;
                                pstMap->acAnimTile[pstMap->u16AnimTileSize].u8AnimLen    = u8AnimLen;
                                pstMap->u16AnimTileSize++;

                                // Prevent buffer overflow.
                                if (pstMap->u16AnimTileSize >= ANIM_TILE_MAX)
                                {
                                    pstMap->u16AnimTileSize = ANIM_TILE_MAX;
                                }
                            }
                        }
                    }
                }
                SDL_Log("Render TMX map layer: %s\n", pstLayer->name);
            }
        }
        pstLayer = pstLayer->next;
    }
    // Switch back to default render target.
    if (0 != SDL_SetRenderTarget(pstRenderer, NULL))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        return -1;
    }

    if (0 != SDL_SetTextureBlendMode(pstMap->pstTexture[u16Index], SDL_BLENDMODE_BLEND))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        return -1;
    }

    return 0;
}

void FreeMap(Map *pstMap)
{
    if (pstMap)
    {
        if (! pstMap->pstTmxMap)
        {
            tmx_map_free(pstMap->pstTmxMap);
        }

        if (pstMap->pstTileset)
        {
            SDL_DestroyTexture(pstMap->pstTileset);
        }

        for (Uint8 u8Index = 0; u8Index < MAP_TEXTURES; u8Index++)
        {
            if (pstMap->pstTexture[u8Index])
            {
                SDL_DestroyTexture(pstMap->pstTexture[u8Index]);
            }
        }

        if (pstMap->pstAnimTexture)
        {
            SDL_DestroyTexture(pstMap->pstAnimTexture);
        }

        SDL_free(pstMap);
        SDL_Log("Unload TMX map.\n");
    }
}

void FreeObject(Object *pstObject)
{
    SDL_free(pstObject);
}

void GetSingleObjectByName(const char *pacName, const Map *pstMap, Object **pstObject)
{
    tmx_layer *pstLayer;

    pstLayer = pstMap->pstTmxMap->ly_head;
    while(pstLayer)
    {
        if (L_OBJGR == pstLayer->type)
        {
            _FindObjectByName(pacName, pstLayer->content.objgr->head, pstObject);
        }

        pstLayer = pstLayer->next;
    }
}

Uint16 GetObjectCount(const Map *pstMap)
{
    Uint16     u16ObjectCount  = 0;
    Uint16    *pu16ObjectCount = &u16ObjectCount;
    tmx_layer *pstLayer;

    pstLayer = pstMap->pstTmxMap->ly_head;
    while(pstLayer)
    {
        if (L_OBJGR == pstLayer->type)
        {
            _GetObjectCount(pstLayer->content.objgr->head, &pu16ObjectCount);
        }

        pstLayer = pstLayer->next;

    }

    return u16ObjectCount;
}

Sint8 InitMap(const char *pacFileName, const char *pacTilesetImage, const Uint8 u8MeterInPixel, Map **pstMap)
{
    *pstMap = SDL_malloc(sizeof(struct Map_t) + TS_IMG_PATH_LEN);
    if (! *pstMap)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "InitMap(): error allocating memory.\n");
        return -1;
    }

    (*pstMap)->pstTmxMap = tmx_load(pacFileName);
    if (! (*pstMap)->pstTmxMap)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", tmx_strerr());
        return -1;
    }

    (*pstMap)->u16Height         = (*pstMap)->pstTmxMap->height * (*pstMap)->pstTmxMap->tile_height;
    (*pstMap)->u16Width          = (*pstMap)->pstTmxMap->width  * (*pstMap)->pstTmxMap->tile_width;
    (*pstMap)->dPosX             = 0.f;
    (*pstMap)->dPosY             = 0.f;
    (*pstMap)->dGravitation      = 0.f;
    (*pstMap)->u8MeterInPixel    = u8MeterInPixel;
    (*pstMap)->acTilesetImage[0] = '\0';
    (*pstMap)->u16AnimTileSize   = 0;
    (*pstMap)->dAnimSpeed        = 6.25f;
    (*pstMap)->dAnimDelay        = 0.f;

    strncat((*pstMap)->acTilesetImage, pacTilesetImage, TS_IMG_PATH_LEN - 1);

    for (Uint8 u8Index = 0; u8Index < MAP_TEXTURES; u8Index++)
    {
        (*pstMap)->pstTexture[u8Index] = NULL;
    }
    (*pstMap)->pstAnimTexture = NULL;
    (*pstMap)->pstTileset     = NULL;

    SDL_Log("Load TMX map file: %s.\n", pacFileName);
    SetGravitation(0, 1, *pstMap);

    return 0;
}

Sint8 InitObject(Object **pstObject)
{
    *pstObject = SDL_malloc(sizeof(struct Object_t));
    if (! *pstObject)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "InitObject(): error allocating memory.\n");
        return -1;
    }

    (*pstObject)->dPosX = 0.f;
    (*pstObject)->dPosY = 0.f;

    return 0;
}

SDL_bool IsMapCoordOfType(
    const char *pacType,
    const Map  *pstMap,
    double      dPosX,
    double      dPosY)
{
    tmx_layer *pstLayers;
    dPosX /= (double)pstMap->pstTmxMap->tile_width;
    dPosY /= (double)pstMap->pstTmxMap->tile_height;

    // Set boundaries to prevent segfault.
    if ( (dPosX < 0.0) ||
         (dPosY < 0.0) ||
         (dPosX > (double)pstMap->pstTmxMap->width) ||
         (dPosY > (double)pstMap->pstTmxMap->height) )
    {
        return SDL_FALSE;
    }

    pstLayers = pstMap->pstTmxMap->ly_head;
    while(pstLayers)
    {
        Uint16 u16TsTileCount = pstMap->pstTmxMap->tiles[1]->tileset->tilecount;
        Uint16 u16Gid         = _ClearGidFlags(
            pstLayers->content.gids[((int32_t)dPosY * pstMap->pstTmxMap->width) + (int32_t)dPosX]);

        if (u16Gid > u16TsTileCount)
        {
            return SDL_FALSE;
        }

        if (pstMap->pstTmxMap->tiles[u16Gid])
        {
            if (pstMap->pstTmxMap->tiles[u16Gid]->type)
            {
                if (0 == strncmp(pacType, pstMap->pstTmxMap->tiles[u16Gid]->type, 20))
                {
                    return SDL_TRUE;
                }
            }
        }

        pstLayers = pstLayers->next;
    }

    return SDL_FALSE;
}

SDL_bool IsOnTileOfType(
    const char  *pacType,
    const double dPosX,
    const double dPosY,
    const Uint8  u8EntityHeight,
    const Map   *pstMap)
{
    return IsMapCoordOfType(pacType, pstMap, dPosX, dPosY + (double)(u8EntityHeight / 2.f));
}

void SetGravitation(const double  dGravitation, const SDL_bool bUseTmxConstant, Map *pstMap)
{
    if (bUseTmxConstant)
    {
        double       *dTmxConstant = NULL;
        tmx_property *pProperty;

        dTmxConstant = &pstMap->dGravitation;
        pProperty    = pstMap->pstTmxMap->properties;
        tmx_property_foreach(pProperty, _GetGravitation, (void *)dTmxConstant);
        pstMap->dGravitation = *dTmxConstant;
    }
    else
    {
        pstMap->dGravitation = dGravitation;
    }

    SDL_Log(
        "Set gravitational constant to %f (g*%dpx/s^2).\n",
        pstMap->dGravitation,
        pstMap->u8MeterInPixel);
}

void SetTileAnimationSpeed(const double dAnimSpeed, Map *pstMap)
{
    pstMap->dAnimSpeed = dAnimSpeed;
}
