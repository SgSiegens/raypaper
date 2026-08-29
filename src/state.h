#ifndef RAYPAPER_STATE_H
#define RAYPAPER_STATE_H

/*
 * Single source of truth for every global that's read or written from more
 * than one module. Definitions (with initial values) live in state.c.
 *
 * Anything only ever touched inside one .c file (e.g. main.c's isExiting,
 * effects.c's particles[]) stays a local/static there instead of being
 * dragged in here - keep this file to genuinely cross-cutting state.
 */

#include "types.h"
#include <stdatomic.h>
#include <stdbool.h>

/* ---- theme / config (set by config.c, read everywhere) ---- */
extern Theme AppTheme;

extern int g_base_thumb_size;
extern int g_max_wallpapers;
extern int g_particle_count;
extern int g_base_padding;
extern int g_max_threads;
extern int g_max_fps;
extern float g_anim_speed;
extern float g_ken_burns_duration;
extern float g_border_thickness_bloom;

extern int g_win_width;
extern int g_win_height;

/* how long a cached thumbnail is kept before being auto-pruned; <= 0 disables age-based pruning */
extern int g_cache_max_age_days;

/* ---- wallpaper collection + async thumbnail loading ---- */
extern Wallpaper wallpapers[MAX_POSSIBLE_WALLPAPERS];
extern int wallpaper_count;

extern atomic_int next_load_index;
extern atomic_bool loader_running;
extern Image pendingImages[MAX_POSSIBLE_WALLPAPERS];
extern atomic_bool imagePending[MAX_POSSIBLE_WALLPAPERS];

/* ---- full-screen preview state ---- */
extern int preview_index;
extern float previewAnim;
extern int closing_preview_index;
extern Texture2D fullPreviewTexture;
extern atomic_bool isFullTextureReady;
extern Image pendingFullImage;
extern atomic_bool fullImagePending;
extern Rectangle g_previewStartRect;

/* ---- zoom ---- */
extern float masterScale;

/* ---- search ---- */
extern char searchBuffer[256];
extern int searchBufferCount;
extern bool isSearching;

/* ---- visual effects ---- */
extern EffectType g_startupEffect;
extern EffectType g_keypressEffect;
extern EffectType g_exitEffect;
extern float g_effectIntensity;
extern float g_effectTimer;
extern float g_effectDuration;
extern EffectType g_activeEffect;

/* ---- layout mode ---- */
extern LayoutMode g_currentMode;
extern LayoutMode g_targetMode;
extern float g_modeTransitionTimer;
#define MODE_TRANSITION_DURATION 1.0f

/* ---- scroll ---- */
extern Vector2 g_scroll;
extern float g_smoothScrollY;
extern float g_smoothScrollX;

/* ---- selection ---- */
extern int g_hoveredIndex;

#endif // RAYPAPER_STATE_H
