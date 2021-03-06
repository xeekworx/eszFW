// SPDX-License-Identifier: Beerware
/**
 * @file      Video.c
 * @ingroup   Video
 * @defgroup  Video Video/Graphics handler
 * @author    Michael Fitzmayer
 * @copyright "THE BEER-WARE LICENCE" (Revision 42)
 */

#include <SDL.h>
#include <SDL_image.h>
#include "Video.h"
#include "Constants.h"

/**
 * @brief   Free video
 * @details Frees up allocated memory and destroys window
 * @param   pstVideo
 *          Pointer to video handle
 */
void Video_Free(Video* pstVideo)
{
    IMG_Quit();
    if (pstVideo)
    {
        if (pstVideo->pstRenderer)
        {
            SDL_DestroyRenderer(pstVideo->pstRenderer);
        }
        if (pstVideo->pstWindow)
        {
            SDL_DestroyWindow(pstVideo->pstWindow);
        }
        SDL_free(pstVideo);
        SDL_Log("Terminate window.\n");
    }
}

/**
 * @brief   Initialise video
 * @details Initialises video and creates window
 * @param   pacWindowTitle
 *          Window title
 * @param   s32WindowWidth
 *          Window width in pixel
 * @param   s32WindowHeight
 *          Window height in pixel
 * @param   s32LogicalWindowWidth
 *          Logical window width in pixel
 * @param   s32LogicalWindowHeight
 *          Logical window height in pixel
 * @param   bFullscreen
 *          Initial fullscreen state
 * @param   pstVideo
 *          Pointer to video handle
 * @return  Error code
 * @retval   0: OK
 * @retval  -1: Error
 */
Sint8 Video_Init(
    const char*    pacWindowTitle,
    const Sint32   s32WindowWidth,
    const Sint32   s32WindowHeight,
    const Sint32   s32LogicalWindowWidth,
    const Sint32   s32LogicalWindowHeight,
    const SDL_bool bFullscreen,
    Video**        pstVideo)
{
    SDL_DisplayMode stDisplayMode;
    Uint32          u32Flags = 0;

    *pstVideo = SDL_calloc(sizeof(struct Video_t), sizeof(Sint8));
    if (!*pstVideo)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "InitVideo(): error allocating memory.\n");
        return -1;
    }

    (*pstVideo)->s32WindowHeight        = s32WindowHeight;
    (*pstVideo)->s32WindowWidth         = s32WindowWidth;
    (*pstVideo)->s32LogicalWindowWidth  = s32LogicalWindowWidth;
    (*pstVideo)->s32LogicalWindowHeight = s32LogicalWindowHeight;
    (*pstVideo)->dTimeA                 = SDL_GetTicks();
    (*pstVideo)->dTimeB                 = SDL_GetTicks();
    (*pstVideo)->dDeltaTime             = ((*pstVideo)->dTimeB - (*pstVideo)->dTimeA) / 1000.f;

    if (0 > SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        return -1;
    }

    if (IMG_INIT_PNG != IMG_Init(IMG_INIT_PNG))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", IMG_GetError());
        return -1;
    }

    if (bFullscreen)
    {
        u32Flags = u32Flags | SDL_WINDOW_FULLSCREEN_DESKTOP;
    }

    if (0 == SDL_GetCurrentDisplayMode(0, &stDisplayMode))
    {
        (*pstVideo)->u8RefreshRate = stDisplayMode.refresh_rate;
    }
    if ((*pstVideo)->u8RefreshRate < 60)
    {
        (*pstVideo)->u8RefreshRate = 60;
    }
    #ifdef __ANDROID__
    u32Flags                     = 0;
    (*pstVideo)->s32WindowWidth  = stDisplayMode.w;
    (*pstVideo)->s32WindowHeight = stDisplayMode.h;
    #endif

    (*pstVideo)->pstWindow = SDL_CreateWindow(
        pacWindowTitle,
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        (*pstVideo)->s32WindowWidth,
        (*pstVideo)->s32WindowHeight,
        u32Flags);

    if (!(*pstVideo)->pstWindow)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        return -1;
    }

    if (0 > SDL_ShowCursor(SDL_DISABLE))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        return -1;
    }

    SDL_GetWindowSize(
        (*pstVideo)->pstWindow, &(*pstVideo)->s32WindowWidth, &(*pstVideo)->s32WindowHeight);

    (*pstVideo)->dZoomLevel = (double)(*pstVideo)->s32WindowHeight / (double)s32LogicalWindowHeight;
    (*pstVideo)->dInitialZoomLevel = (*pstVideo)->dZoomLevel;

    (*pstVideo)->pstRenderer = SDL_CreateRenderer(
        (*pstVideo)->pstWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);

    if (!(*pstVideo)->pstRenderer)
    {
        SDL_DestroyWindow((*pstVideo)->pstWindow);
        return -1;
    }

    SDL_Log(
        "Setting up window at resolution %dx%d @ %d FPS.\n",
        (*pstVideo)->s32WindowWidth,
        (*pstVideo)->s32WindowHeight,
        (*pstVideo)->u8RefreshRate);

    Video_SetZoomLevel((*pstVideo)->dZoomLevel, *pstVideo);
    SDL_Log("Set initial zoom-level to factor %f.\n", (*pstVideo)->dZoomLevel);

    return 0;
}

/**
 * @brief   Render scene
 * @details Render/draw current scene
 * @param   pstVideo
 *          Pointer to video handle
 */
void Video_RenderScene(Video* pstVideo)
{
    double dTime = (double)APPROX_TIME_PER_FRAME / (double)TIME_FACTOR;

    pstVideo->dTimeB     = SDL_GetTicks();
    pstVideo->dDeltaTime = (pstVideo->dTimeB - pstVideo->dTimeA) / 1000.f;
    pstVideo->dTimeA     = pstVideo->dTimeB;

    if (pstVideo->dDeltaTime >= dTime)
    {
        pstVideo->dDeltaTime = dTime;
    }

    SDL_RenderPresent(pstVideo->pstRenderer);
    SDL_Delay(1000.f / (double)pstVideo->u8RefreshRate - pstVideo->dDeltaTime);
    SDL_RenderClear(pstVideo->pstRenderer);
}

/**
 * @brief   Set zoom-level
 * @details Sets zoom-level
 * @param   dZoomLevel
 *          Zoom-level
 * @param   pstVideo
 *          Pointer to video handle
 * @return  Error code
 * @retval   0: OK
 * @retval  -1: Error
 */
Sint8 Video_SetZoomLevel(const double dZoomLevel, Video* pstVideo)
{
    pstVideo->dZoomLevel             = dZoomLevel;
    pstVideo->s32LogicalWindowWidth  = pstVideo->s32WindowWidth / dZoomLevel;
    pstVideo->s32LogicalWindowHeight = pstVideo->s32WindowHeight / dZoomLevel;

    if (0 != SDL_RenderSetLogicalSize(
                 pstVideo->pstRenderer,
                 pstVideo->s32LogicalWindowWidth,
                 pstVideo->s32LogicalWindowHeight))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        return -1;
    }

    #ifdef DEBUG
    if (dZoomLevel != pstVideo->dZoomLevel)
    {
        SDL_Log("Set zoom-level to factor %f.\n", dZoomLevel);
    }
    #endif

    return 0;
}

/**
 * @brief   Toggle fullscreen
 * @details Togggles fullscreen state
 * @param   pstVideo
 *          Pointer to video handle
 * @return  Error code
 * @retval  0: OK
 * @retval  Negative error code: failure
 */
Sint8 Video_ToggleFullscreen(Video* pstVideo)
{
    Sint8  s8ReturnValue;
    Uint32 u32WindowFlags;

    u32WindowFlags = SDL_GetWindowFlags(pstVideo->pstWindow);

    if (u32WindowFlags & SDL_WINDOW_FULLSCREEN_DESKTOP)
    {
        SDL_SetWindowPosition(pstVideo->pstWindow, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
        s8ReturnValue = SDL_SetWindowFullscreen(pstVideo->pstWindow, 0);
        SDL_Log("Set window to windowed mode.\n");
    }
    else
    {
        s8ReturnValue = SDL_SetWindowFullscreen(pstVideo->pstWindow, SDL_WINDOW_FULLSCREEN_DESKTOP);
        SDL_Log("Set window to fullscreen mode.\n");
    }

    if (0 != s8ReturnValue)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
    }

    return s8ReturnValue;
}
