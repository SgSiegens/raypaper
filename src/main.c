#include "raylib.h"
#include "raymath.h"

#include "types.h"
#include "state.h"
#include "util.h"
#include "config.h"
#include "cli.h"
#include "wallpaper.h"
#include "cache.h"
#include "effects.h"
#include "render.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdatomic.h>
#include <math.h>

int main(int argc, char **argv)
{
    CheckForHelpFlag(argc, argv); /* exits immediately if --help was passed */
    bool clear_cache_requested = CheckForClearCacheFlag(argc, argv);

    bool print_filename_only = false;

    LoadDefaultConfig();
    ParseConfigFile();
    InitThumbnailCache(clear_cache_requested);
    SetTraceLogLevel(LOG_ERROR);

    const char *cli_path = ParseCommandLineArgs(argc, argv, &print_filename_only);

    char default_path[1024];
    const char *wallpaper_path = cli_path;
    if (!wallpaper_path)
    {
        snprintf(default_path, sizeof(default_path), "%s/Pictures", get_home_dir());
        wallpaper_path = default_path;
    }
    LoadWallpapers(wallpaper_path);

    InitWindow(g_win_width, g_win_height, "Raypaper");
    SetExitKey(KEY_NULL);
    SetTargetFPS(g_max_fps);

    InitRenderPipeline(g_win_width, g_win_height);

    pthread_t loader_threads[g_max_threads];
    for (int t = 0; t < g_max_threads; t++)
    {
        pthread_create(&loader_threads[t], NULL, LoaderThread, NULL);
    }
    TriggerEffect(g_startupEffect, 1.0f);

    bool isExiting = false;
    float kenBurnsTimer = 0.0f;
    float keyRepeatTimer = 0.0f;
    const float KEY_REPEAT_DELAY = 0.1f;

    while (!WindowShouldClose())
    {
        float delta = GetFrameTime();
        int sw = GetScreenWidth();
        int sh = GetScreenHeight();

        if (isExiting && g_effectTimer <= 0)
        {
            break;
        }

        HandleRenderResize();

        if (g_effectTimer > 0)
        {
            g_effectTimer -= delta;
            float p = 1.f - (g_effectTimer / g_effectDuration);
            if (g_activeEffect == EFFECT_COLLAPSE || g_activeEffect == EFFECT_REVEAL)
            {
                g_effectIntensity = p;
            }
            else
            {
                g_effectIntensity = sinf(p * PI);
            }
        }
        else
        {
            g_effectIntensity = 0.f;
            g_activeEffect = EFFECT_NONE;
        }

        bool isPreviewing = (preview_index != -1);

        int filteredIndices[g_max_wallpapers];
        int filteredCount = 0;
        for (int i = 0; i < wallpaper_count; i++)
        {
            if (searchBufferCount == 0 || stristr(wallpapers[i].filename, searchBuffer) != NULL)
            {
                filteredIndices[filteredCount++] = i;
            }
        }

        float maxScrollY = 0, maxScrollX = 0;
        switch (g_currentMode)
        {
            case MODE_GRID:
            {
                float ts = g_base_thumb_size * masterScale, p = g_base_padding * masterScale;
                int c = sw / (ts + p);
                if (c < 1) c = 1;
                int r = (filteredCount + c - 1) / c;
                if (r > 0) maxScrollY = r * (ts + p) - sh + p;
                break;
            }
            case MODE_RIVER_V:
            {
                maxScrollY = filteredCount * (g_base_thumb_size * 0.7f * masterScale) - sh + (g_base_thumb_size * 1.5f * masterScale);
                break;
            }
            case MODE_RIVER_H:
            case MODE_WAVE:
            {
                maxScrollX = filteredCount * (g_base_thumb_size * 0.8f * masterScale) - sw + (g_base_thumb_size * 1.5f * masterScale);
                break;
            }
        }
        if (maxScrollY < 0) maxScrollY = 0;
        if (maxScrollX < 0) maxScrollX = 0;

        bool blockActions = isExiting || isPreviewing;
        if (!blockActions)
        {
            int key = GetKeyPressed();
            if (key != 0 && !isSearching)
            {
                TriggerEffect(g_keypressEffect, 0.4f);
            }
            if (key >= KEY_ONE && key < KEY_ONE + NUM_MODES)
            {
                g_targetMode = key - KEY_ONE;
                g_modeTransitionTimer = MODE_TRANSITION_DURATION;
            }
        }

        bool ateEscKey = false;
        if (isSearching)
        {
            int key = GetCharPressed();
            while (key > 0)
            {
                if ((key >= 32) && (key <= 125) && (searchBufferCount < 255))
                {
                    searchBuffer[searchBufferCount++] = (char)key;
                }
                key = GetCharPressed();
            }
            if (IsKeyPressed(KEY_BACKSPACE)) { if (searchBufferCount > 0) searchBuffer[--searchBufferCount] = '\0'; }
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_ESCAPE)) { isSearching = false; if (IsKeyPressed(KEY_ESCAPE)) ateEscKey = true; }
        }
        else
        {
            if (!blockActions && IsKeyPressed(KEY_SLASH)) { isSearching = true; searchBufferCount = 0; searchBuffer[0] = '\0'; }
        }

        if (!blockActions && !isSearching && filteredCount > 0)
        {
            int direction = 0;
            keyRepeatTimer -= delta;

            bool navKeyPressed = IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_L) || IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_H) ||
                                  IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_J) || IsKeyDown(KEY_UP) || IsKeyDown(KEY_K);

            if (navKeyPressed && keyRepeatTimer <= 0.0f)
            {
                keyRepeatTimer = KEY_REPEAT_DELAY;

                if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_L)) direction = 1;
                if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_H)) direction = -1;
                if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_J))
                {
                    if (g_currentMode == MODE_GRID)
                    {
                        int cols = (GetScreenWidth() / (g_base_thumb_size * masterScale + g_base_padding * masterScale));
                        if (cols < 1) cols = 1;
                        direction = cols;
                    }
                    else direction = 1;
                }
                if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_K))
                {
                    if (g_currentMode == MODE_GRID)
                    {
                        int cols = (GetScreenWidth() / (g_base_thumb_size * masterScale + g_base_padding * masterScale));
                        if (cols < 1) cols = 1;
                        direction = -cols;
                    }
                    else direction = -1;
                }

                if (direction != 0)
                {
                    int currentFilteredIdx = -1;
                    for (int i = 0; i < filteredCount; i++)
                    {
                        if (filteredIndices[i] == g_hoveredIndex)
                        {
                            currentFilteredIdx = i;
                            break;
                        }
                    }

                    int nextFilteredIdx = (currentFilteredIdx == -1) ? ((direction > 0) ? 0 : filteredCount - 1) : (currentFilteredIdx + direction);
                    nextFilteredIdx = Clamp(nextFilteredIdx, 0, filteredCount - 1);
                    g_hoveredIndex = filteredIndices[nextFilteredIdx];

                    Rectangle itemRect = {wallpapers[g_hoveredIndex].animPos.x, wallpapers[g_hoveredIndex].animPos.y, wallpapers[g_hoveredIndex].animSize.x, wallpapers[g_hoveredIndex].animSize.y};
                    if (g_currentMode == MODE_GRID || g_currentMode == MODE_RIVER_V)
                    {
                        if (itemRect.y < g_smoothScrollY) g_scroll.y = itemRect.y - g_base_padding;
                        if (itemRect.y + itemRect.height > g_smoothScrollY + sh) g_scroll.y = itemRect.y + itemRect.height - sh + g_base_padding;
                    }
                    else
                    {
                        if (itemRect.x < g_smoothScrollX) g_scroll.x = itemRect.x - g_base_padding;
                        if (itemRect.x + itemRect.width > g_smoothScrollX + sw) g_scroll.x = itemRect.x + itemRect.width - sw + g_base_padding;
                    }
                }
            }
        }

        float wheel = GetMouseWheelMove();
        if (IsKeyDown(KEY_LEFT_CONTROL))
        {
            masterScale += wheel * 0.05f;
            masterScale = Clamp(masterScale, 0.2f, 5.0f);
        }
        else if (!isPreviewing)
        {
            if (g_currentMode == MODE_RIVER_H || g_currentMode == MODE_WAVE) g_scroll.x -= wheel * 100.f;
            else g_scroll.y -= wheel * 100.f;
        }

        g_scroll.y = Clamp(g_scroll.y, 0, maxScrollY);
        g_scroll.x = Clamp(g_scroll.x, 0, maxScrollX);

        if (!isPreviewing && !isExiting && !isSearching && g_hoveredIndex != -1)
        {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || IsKeyPressed(KEY_ENTER))
            {
                isExiting = true;
                TriggerParticleBurst(GetMousePosition());
                printf("%s\n", print_filename_only ? wallpapers[g_hoveredIndex].filename : wallpapers[g_hoveredIndex].path);
                fflush(stdout);
                TriggerEffect(g_exitEffect, 1.5f);
            }
            if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) || IsKeyPressed(KEY_LEFT_SHIFT))
            {
                preview_index = g_hoveredIndex;
                closing_preview_index = g_hoveredIndex;
                atomic_store(&isFullTextureReady, false);
                g_previewStartRect = (Rectangle){wallpapers[g_hoveredIndex].animPos.x, wallpapers[g_hoveredIndex].animPos.y, wallpapers[g_hoveredIndex].animSize.x, wallpapers[g_hoveredIndex].animSize.y};
                pthread_t pt;
                pthread_create(&pt, NULL, FullPreviewLoaderThread, wallpapers[g_hoveredIndex].path);
                pthread_detach(pt);
            }
        }

        if (isPreviewing && (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || IsKeyPressed(KEY_ESCAPE) || IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)))
        {
            if (IsKeyPressed(KEY_ESCAPE)) ateEscKey = true;
            if (atomic_load(&isFullTextureReady)) UnloadTexture(fullPreviewTexture);
            closing_preview_index = preview_index;
            g_previewStartRect = (Rectangle){wallpapers[closing_preview_index].animPos.x, wallpapers[closing_preview_index].animPos.y, wallpapers[closing_preview_index].animSize.x, wallpapers[closing_preview_index].animSize.y};
            preview_index = -1;
        }

        if (IsKeyPressed(KEY_ESCAPE) && !ateEscKey)
        {
            break;
        }

        g_smoothScrollY = Lerp(g_smoothScrollY, g_scroll.y, delta * g_anim_speed);
        g_smoothScrollX = Lerp(g_smoothScrollX, g_scroll.x, delta * g_anim_speed);

        int textures_loaded_this_frame = 0;
        for (int i = 0; i < wallpaper_count; i++)
        {
            if (textures_loaded_this_frame >= MAX_TEXTURES_TO_LOAD_PER_FRAME) break;
            if (atomic_load(&imagePending[i]))
            {
                wallpapers[i].texture = LoadTextureFromImage(pendingImages[i]);
                UnloadImage(pendingImages[i]);
                atomic_store(&wallpapers[i].loaded, true);
                atomic_store(&imagePending[i], false);
                textures_loaded_this_frame++;
            }
        }

        if (atomic_load(&fullImagePending))
        {
            fullPreviewTexture = LoadTextureFromImage(pendingFullImage);
            UnloadImage(pendingFullImage);
            atomic_store(&isFullTextureReady, true);
            atomic_store(&fullImagePending, false);
        }
        previewAnim = Lerp(previewAnim, isPreviewing ? 1.f : 0.f, delta * g_anim_speed * 0.8f);
        if (isPreviewing) { if (kenBurnsTimer < g_ken_burns_duration) kenBurnsTimer += delta; }

        RenderFrame(filteredCount, filteredIndices, delta, isPreviewing, isSearching);
    }

    atomic_store(&loader_running, false);
    for (int t = 0; t < g_max_threads; t++)
    {
        pthread_join(loader_threads[t], NULL);
    }

    ShutdownRenderPipeline();

    for (int i = 0; i < wallpaper_count; i++)
    {
        if (atomic_load(&wallpapers[i].loaded))
        {
            UnloadTexture(wallpapers[i].texture);
        }
        free(wallpapers[i].path);
        free(wallpapers[i].filename);
    }
    CloseWindow();
    return 0;
}
