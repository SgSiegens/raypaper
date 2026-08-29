#include "state.h"

Theme AppTheme;

int g_base_thumb_size;
int g_max_wallpapers;
int g_particle_count;
int g_base_padding;
int g_max_threads;
int g_max_fps;
float g_anim_speed;
float g_ken_burns_duration;
float g_border_thickness_bloom;

int g_win_width;
int g_win_height;

int g_cache_max_age_days;

Wallpaper wallpapers[MAX_POSSIBLE_WALLPAPERS];
int wallpaper_count = 0;

atomic_int next_load_index = 0;
atomic_bool loader_running = true;
Image pendingImages[MAX_POSSIBLE_WALLPAPERS];
atomic_bool imagePending[MAX_POSSIBLE_WALLPAPERS];

int preview_index = -1;
float previewAnim = 0.0f;
int closing_preview_index = -1;
Texture2D fullPreviewTexture;
atomic_bool isFullTextureReady = false;
Image pendingFullImage;
atomic_bool fullImagePending = false;
Rectangle g_previewStartRect;

float masterScale = 1.0f;

char searchBuffer[256] = {0};
int searchBufferCount = 0;
bool isSearching = false;

EffectType g_startupEffect = EFFECT_NONE;
EffectType g_keypressEffect = EFFECT_NONE;
EffectType g_exitEffect = EFFECT_NONE;
float g_effectIntensity = 0.0f;
float g_effectTimer = 0.0f;
float g_effectDuration = 0.0f;
EffectType g_activeEffect = EFFECT_NONE;

LayoutMode g_currentMode = MODE_GRID;
LayoutMode g_targetMode = MODE_GRID;
float g_modeTransitionTimer = 0.0f;

Vector2 g_scroll = {0, 0};
float g_smoothScrollY = 0.0f;
float g_smoothScrollX = 0.0f;

int g_hoveredIndex = -1;
