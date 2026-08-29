#include "scene.h"
#include "state.h"

#include "raylib.h"
#include "raymath.h"

#include <math.h>
#include <stdatomic.h>

void UpdateAndDrawScene(int filteredCount, int *filteredIndices, float delta, bool isPreviewing, bool isSearching)
{
    int sw = GetScreenWidth();
    int sh = GetScreenHeight();
    Vector2 mouse = GetMousePosition();

    if (g_modeTransitionTimer > 0)
    {
        g_modeTransitionTimer -= delta;
        if (g_modeTransitionTimer <= 0)
        {
            g_currentMode = g_targetMode;
        }
    }

    for (int j = 0; j < filteredCount; j++)
    {
        int i = filteredIndices[j];
        Vector2 targetPos2D = {0}, targetSize2D = {0};
        switch (g_targetMode)
        {
            case MODE_GRID:
            {
                float ts = g_base_thumb_size * masterScale, p = g_base_padding * masterScale;
                int c = sw / (ts + p);
                if (c < 1) c = 1;
                targetPos2D = (Vector2){(j % c) * (ts + p) + p, (j / c) * (ts + p) + p - g_smoothScrollY};
                targetSize2D = (Vector2){ts, ts};
                break;
            }
            case MODE_RIVER_H:
            {
                float x = j * (g_base_thumb_size * 0.7f * masterScale) - g_smoothScrollX;
                float d = fabs(x - (sw / 2.f - g_base_thumb_size / 2.f));
                float s = Remap(d, 0, sw / 2.f, 1.5f, 0.5f);
                s = Clamp(s, 0.5, 1.5) * masterScale;
                targetPos2D = (Vector2){x, sh / 2.f - (g_base_thumb_size * s) / 2.f};
                targetSize2D = (Vector2){g_base_thumb_size * s, g_base_thumb_size * s};
                break;
            }
            case MODE_RIVER_V:
            {
                float y = j * (g_base_thumb_size * 0.7f * masterScale) - g_smoothScrollY;
                float d = fabs(y - (sh / 2.f - g_base_thumb_size / 2.f));
                float s = Remap(d, 0, sh / 2.f, 1.5f, 0.5f);
                s = Clamp(s, 0.5, 1.5) * masterScale;
                targetPos2D = (Vector2){sw / 2.f - (g_base_thumb_size * s) / 2.f, y};
                targetSize2D = (Vector2){g_base_thumb_size * s, g_base_thumb_size * s};
                break;
            }
            case MODE_WAVE:
            {
                float x = j * (g_base_thumb_size * 0.8f * masterScale) - g_smoothScrollX;
                float yOff = sinf(j * 0.2f + GetTime() * 2.f) * 80.f * masterScale;
                targetPos2D = (Vector2){x, sh / 2.f + yOff - g_base_thumb_size / 2.f};
                targetSize2D = (Vector2){g_base_thumb_size * masterScale, g_base_thumb_size * masterScale};
                break;
            }
        }
        wallpapers[i].animPos = Vector2Lerp(wallpapers[i].animPos, targetPos2D, delta * g_anim_speed);
        wallpapers[i].animSize = Vector2Lerp(wallpapers[i].animSize, targetSize2D, delta * g_anim_speed);
    }

    if (Vector2LengthSqr(GetMouseDelta()) > 0.1f)
    {
        g_hoveredIndex = -1;
        for (int j = filteredCount - 1; j >= 0; j--)
        {
            int i = filteredIndices[j];
            Rectangle rect = {wallpapers[i].animPos.x, wallpapers[i].animPos.y, wallpapers[i].animSize.x, wallpapers[i].animSize.y};
            if (CheckCollisionPointRec(mouse, rect) && !isPreviewing && !isSearching)
            {
                g_hoveredIndex = i;
                break;
            }
        }
    }

    int highlighted_j = -1;
    for (int j = 0; j < filteredCount; j++)
    {
        int i = filteredIndices[j];
        wallpapers[i].hoverAnim = Lerp(wallpapers[i].hoverAnim, (g_hoveredIndex == i) ? 1.f : 0.f, delta * g_anim_speed * 2.0f);
        if (g_hoveredIndex == i)
        {
            highlighted_j = j;
        }
    }

    for (int j = 0; j < filteredCount; j++)
    {
        if (j == highlighted_j) continue;
        int i = filteredIndices[j];
        Rectangle rect = {wallpapers[i].animPos.x, wallpapers[i].animPos.y, wallpapers[i].animSize.x, wallpapers[i].animSize.y};
        DrawRectangleRounded(rect, 0.1f, 8, ColorLerp(AppTheme.idle, AppTheme.hover, wallpapers[i].hoverAnim));
        if (atomic_load(&wallpapers[i].loaded))
        {
            DrawTexturePro(wallpapers[i].texture, (Rectangle){0, 0, g_base_thumb_size, g_base_thumb_size}, rect, (Vector2){0, 0}, 0, WHITE);
        }
        else
        {
            DrawCircleSectorLines(Vector2Add((Vector2){rect.x, rect.y}, (Vector2){rect.width / 2, rect.height / 2}), rect.width / 3, fmodf(GetTime() * 360, 360), fmodf(GetTime() * 360, 360) + 90, 30, Fade(AppTheme.border, 0.6f));
        }
    }

    if (highlighted_j != -1)
    {
        int i = filteredIndices[highlighted_j];
        Rectangle rect = {wallpapers[i].animPos.x, wallpapers[i].animPos.y, wallpapers[i].animSize.x, wallpapers[i].animSize.y};
        if (!isPreviewing)
            DrawRectangleRounded(rect, 0.1f, 8, ColorLerp(AppTheme.idle, AppTheme.hover, wallpapers[i].hoverAnim));
        if (atomic_load(&wallpapers[i].loaded))
        {
            DrawTexturePro(wallpapers[i].texture, (Rectangle){0, 0, g_base_thumb_size, g_base_thumb_size}, rect, (Vector2){0, 0}, 0, WHITE);
        }
        else
        {
            DrawCircleSectorLines(Vector2Add((Vector2){rect.x, rect.y}, (Vector2){rect.width / 2, rect.height / 2}), rect.width / 3, fmodf(GetTime() * 360, 360), fmodf(GetTime() * 360, 360) + 90, 30, Fade(AppTheme.border, 0.6f));
        }
        const char *text = wallpapers[i].filename;
        float fontSize = 14.f * Clamp(masterScale, 0.8f, 1.5f);
        int textWidth = MeasureText(text, fontSize);
        DrawText(text, rect.x + (rect.width - textWidth) / 2, rect.y + rect.height + 5, fontSize, AppTheme.text);
    }

    if (previewAnim > 0.001f)
    {
        DrawRectangle(0, 0, sw, sh, Fade(AppTheme.overlay, previewAnim * 0.9f));

        int current_preview_idx = (preview_index != -1) ? preview_index : closing_preview_index;
        if (current_preview_idx == -1)
        {
            return;
        }

        Texture2D *tex = (atomic_load(&isFullTextureReady) && preview_index != -1) ? &fullPreviewTexture : &wallpapers[current_preview_idx].texture;
        Rectangle texRect = {0, 0, (float)tex->width, (float)tex->height};
        float aspect = (float)texRect.width / (float)texRect.height;
        float tH = sh * 0.9f, tW = tH * aspect;
        if (tW > sw * 0.9f)
        {
            tW = sw * 0.9f;
            tH = tW / aspect;
        }

        Rectangle endRect = {(sw - tW) / 2, (sh - tH) / 2, tW, tH};
        Rectangle startRect = g_previewStartRect;

        if (preview_index == -1)
        {
            startRect = (Rectangle){wallpapers[closing_preview_index].animPos.x, wallpapers[closing_preview_index].animPos.y, wallpapers[closing_preview_index].animSize.x, wallpapers[closing_preview_index].animSize.y};
        }

        Rectangle cur = {
            Lerp(startRect.x, endRect.x, previewAnim),
            Lerp(startRect.y, endRect.y, previewAnim),
            Lerp(startRect.width, endRect.width, previewAnim),
            Lerp(startRect.height, endRect.height, previewAnim)
        };

        DrawTexturePro(*tex, texRect, cur, (Vector2){0, 0}, 0.f, WHITE);
    }
}
