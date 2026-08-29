#include "render.h"
#include "state.h"
#include "shaders.h"
#include "scene.h"

#include "raylib.h"
#include "rlgl.h"

#include <stdio.h>
#include <string.h>
#include <math.h>

static const float bloomDownscale = 4.0f;

static Shader postShader;
static Shader blurShader;
static RenderTexture2D mainTarget;
static RenderTexture2D bloomMask;
static RenderTexture2D blurPingPong;
static RenderTexture2D bloomMaskHiRes;

static int timeLoc, resLoc, glitchLoc, blurLoc, pixelLoc, mirrorLoc, shakeLoc, collapseLoc, scanlineLoc;

void InitRenderPipeline(int width, int height)
{
    postShader = LoadShaderFromMemory(NULL, postProcessingFs);
    timeLoc = GetShaderLocation(postShader, "time");
    resLoc = GetShaderLocation(postShader, "resolution");
    glitchLoc = GetShaderLocation(postShader, "glitchIntensity");
    blurLoc = GetShaderLocation(postShader, "blurIntensity");
    pixelLoc = GetShaderLocation(postShader, "pixelSize");
    mirrorLoc = GetShaderLocation(postShader, "mirrorMode");
    shakeLoc = GetShaderLocation(postShader, "shakeIntensity");
    collapseLoc = GetShaderLocation(postShader, "collapseIntensity");
    scanlineLoc = GetShaderLocation(postShader, "scanlineIntensity");

    blurShader = LoadShaderFromMemory(NULL, blurFs);
    mainTarget = LoadRenderTexture(width, height);
    bloomMask = LoadRenderTexture(width / bloomDownscale, height / bloomDownscale);
    blurPingPong = LoadRenderTexture(width / bloomDownscale, height / bloomDownscale);
    bloomMaskHiRes = LoadRenderTexture(width, height);
}

void HandleRenderResize(void)
{
    if (!IsWindowResized()) return;

    int sw = GetScreenWidth();
    int sh = GetScreenHeight();

    UnloadRenderTexture(mainTarget);
    UnloadRenderTexture(bloomMask);
    UnloadRenderTexture(blurPingPong);
    UnloadRenderTexture(bloomMaskHiRes);
    mainTarget = LoadRenderTexture(sw, sh);
    bloomMask = LoadRenderTexture(sw / bloomDownscale, sh / bloomDownscale);
    blurPingPong = LoadRenderTexture(sw / bloomDownscale, sh / bloomDownscale);
    bloomMaskHiRes = LoadRenderTexture(sw, sh);
}

void RenderFrame(int filteredCount, int *filteredIndices, float delta, bool isPreviewing, bool isSearching)
{
    int sw = GetScreenWidth();
    int sh = GetScreenHeight();

    BeginTextureMode(mainTarget);
    {
        ClearBackground(AppTheme.bg);
        UpdateAndDrawScene(filteredCount, filteredIndices, delta, isPreviewing, isSearching);
        if (isSearching || searchBufferCount > 0)
        {
            DrawRectangle(0, 0, sw, 40, AppTheme.overlay);
            DrawRectangleLines(0, 0, sw, 40, AppTheme.border);
            char s[512];
            snprintf(s, sizeof(s), "Search: %s", searchBuffer);
            if (isSearching && fmod(GetTime(), 1.0) > 0.5) strcat(s, "|");
            DrawText(s, 10, 10, 20, AppTheme.text);
        }
    }
    EndTextureMode();

    BeginTextureMode(bloomMaskHiRes);
    {
        ClearBackground(BLANK);
        if (g_hoveredIndex != -1 && wallpapers[g_hoveredIndex].hoverAnim > 0.01f)
        {
            Wallpaper w = wallpapers[g_hoveredIndex];
            Rectangle r = {w.animPos.x, w.animPos.y, w.animSize.x, w.animSize.y};
            if (!isPreviewing)
                DrawRectangleRoundedLinesEx(r, 0.1f, 8, g_border_thickness_bloom * 2.f, Fade(AppTheme.border, w.hoverAnim));
        }
    }
    EndTextureMode();

    bool h = true;
    Vector2 rs = {(float)bloomMask.texture.width, (float)bloomMask.texture.height};
    BeginShaderMode(blurShader);
    SetShaderValue(blurShader, GetShaderLocation(blurShader, "renderSize"), &rs, SHADER_UNIFORM_VEC2);
    SetShaderValue(blurShader, GetShaderLocation(blurShader, "horizontal"), &h, SHADER_UNIFORM_INT);
    BeginTextureMode(blurPingPong);
    ClearBackground(BLANK);
    DrawTextureRec(bloomMask.texture, (Rectangle){0, 0, rs.x, -rs.y}, (Vector2){0, 0}, WHITE);
    EndTextureMode();
    h = false;
    SetShaderValue(blurShader, GetShaderLocation(blurShader, "horizontal"), &h, SHADER_UNIFORM_INT);
    BeginTextureMode(bloomMask);
    ClearBackground(BLANK);
    DrawTextureRec(blurPingPong.texture, (Rectangle){0, 0, rs.x, -rs.y}, (Vector2){0, 0}, WHITE);
    EndTextureMode();
    EndShaderMode();

    BeginDrawing();
    {
        ClearBackground(AppTheme.bg);
        BeginShaderMode(postShader);
        float tt = GetTime();
        SetShaderValue(postShader, timeLoc, &tt, SHADER_UNIFORM_FLOAT);
        Vector2 res = {(float)sw, (float)sh};
        SetShaderValue(postShader, resLoc, &res, SHADER_UNIFORM_VEC2);
        float z = 0.f, pv = 0.f;
        int m = 0;
        float sl = 0.0f;
        SetShaderValue(postShader, glitchLoc, (g_activeEffect == EFFECT_GLITCH) ? &g_effectIntensity : &z, SHADER_UNIFORM_FLOAT);
        SetShaderValue(postShader, blurLoc, (g_activeEffect == EFFECT_BLUR) ? &g_effectIntensity : &z, SHADER_UNIFORM_FLOAT);
        if (g_activeEffect == EFFECT_PIXELATE)
        {
            pv = 256.f * (1.f - g_effectIntensity);
            if (pv < 10.f) pv = 10.f;
        }
        SetShaderValue(postShader, pixelLoc, &pv, SHADER_UNIFORM_FLOAT);
        if (g_activeEffect == EFFECT_REVEAL) m = 3;
        SetShaderValue(postShader, mirrorLoc, &m, SHADER_UNIFORM_INT);
        SetShaderValue(postShader, shakeLoc, (g_activeEffect == EFFECT_SHAKE) ? &g_effectIntensity : &z, SHADER_UNIFORM_FLOAT);
        SetShaderValue(postShader, collapseLoc, (g_activeEffect == EFFECT_REVEAL || g_activeEffect == EFFECT_COLLAPSE) ? &g_effectIntensity : &z, SHADER_UNIFORM_FLOAT);
        SetShaderValue(postShader, scanlineLoc, &sl, SHADER_UNIFORM_FLOAT);

        rlActiveTextureSlot(1);
        rlEnableTexture(bloomMask.texture.id);
        SetShaderValue(postShader, GetShaderLocation(postShader, "bloomTexture"), (int[]){1}, SHADER_UNIFORM_INT);
        rlActiveTextureSlot(2);
        rlEnableTexture(bloomMaskHiRes.texture.id);
        SetShaderValue(postShader, GetShaderLocation(postShader, "bloomTextureHiRes"), (int[]){2}, SHADER_UNIFORM_INT);

        rlActiveTextureSlot(0);
        DrawTextureRec(mainTarget.texture, (Rectangle){0, 0, (float)sw, (float)-sh}, (Vector2){0, 0}, WHITE);
        EndShaderMode();

        DrawText("Modes: 1-4 | Nav: HJKL/Arrows | Zoom: Ctrl+Scroll | Search: / | Preview: L-Shift/RMB | Select: Enter/LMB", 10, sh - 20, 10, AppTheme.text);
    }
    EndDrawing();
}

void ShutdownRenderPipeline(void)
{
    UnloadShader(postShader);
    UnloadShader(blurShader);
    UnloadRenderTexture(mainTarget);
    UnloadRenderTexture(bloomMask);
    UnloadRenderTexture(blurPingPong);
    UnloadRenderTexture(bloomMaskHiRes);
}
