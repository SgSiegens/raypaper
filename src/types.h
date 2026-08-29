#ifndef RAYPAPER_TYPES_H
#define RAYPAPER_TYPES_H

#include "raylib.h"
#include <stdatomic.h>

#define MAX_POSSIBLE_WALLPAPERS 2048
#define MAX_POSSIBLE_PARTICLES 256
#define MAX_TEXTURES_TO_LOAD_PER_FRAME 4
#define NUM_MODES 4

typedef struct
{
    Color bg;
    Color idle;
    Color hover;
    Color border;
    Color ripple;
    Color overlay;
    Color text;
} Theme;

typedef enum
{
    MODE_GRID = 0,
    MODE_RIVER_H,
    MODE_RIVER_V,
    MODE_WAVE
} LayoutMode;

typedef struct
{
    float hoverAnim;
    atomic_bool loaded;
    char *path, *filename;

    Texture2D texture;
    Vector2 animPos, animSize;
} Wallpaper;

typedef struct
{
    Vector2 pos;
    Vector2 vel;
    Color color;
    float life;
} Particle;

typedef enum
{
    EFFECT_NONE,
    EFFECT_GLITCH,
    EFFECT_BLUR,
    EFFECT_PIXELATE,
    EFFECT_REVEAL,
    EFFECT_SHAKE,
    EFFECT_COLLAPSE
} EffectType;

#endif // RAYPAPER_TYPES_H
