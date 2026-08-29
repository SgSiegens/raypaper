#include "config.h"
#include "state.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

void LoadDefaultConfig(void)
{
    AppTheme.bg = (Color){10, 10, 15, 255};
    AppTheme.idle = (Color){30, 30, 46, 255};
    AppTheme.hover = (Color){49, 50, 68, 255};
    AppTheme.border = (Color){203, 166, 247, 255};
    AppTheme.ripple = (Color){245, 194, 231, 255};
    AppTheme.overlay = (Color){10, 10, 15, 200};
    AppTheme.text = (Color){202, 212, 241, 255};

    g_startupEffect = EFFECT_NONE;
    g_keypressEffect = EFFECT_NONE;
    g_exitEffect = EFFECT_NONE;

    g_max_wallpapers = 512;
    g_base_thumb_size = 150;
    g_base_padding = 15;
    g_border_thickness_bloom = 3.0f;

    g_max_threads = sysconf(_SC_NPROCESSORS_ONLN) / 2;
    if (g_max_threads < 1) g_max_threads = 1; /* integer division rounds a 1-core machine down to 0 */
    g_anim_speed = 30.0f;
    g_particle_count = 50;
    g_ken_burns_duration = 15.0f;
    g_max_fps = 200;

    g_win_width = 1280;
    g_win_height = 720;

    g_cache_max_age_days = 30;
}

void ParseConfigFile(void)
{
    char config_path[1024];
    snprintf(config_path, sizeof(config_path), "%s/.config/raypaper", get_home_dir());
    mkdir(config_path, 0755);
    snprintf(config_path, sizeof(config_path), "%s/.config/raypaper/raypaper.conf", get_home_dir());

    FILE *file = fopen(config_path, "r");
    if (!file)
    {
        LogMessage(LOG_INFO, "Config file not found at '%s'. Using default settings.", config_path);
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), file))
    {
        if (line[0] == '#' || line[0] == ';' || line[0] == '\n') continue;

        char *key = strtok(line, "=");
        char *value = strtok(NULL, "\n");

        if (key && value)
        {
            key = trim_whitespace(key);
            value = trim_whitespace(value);

            if (strcmp(key, "bg") == 0) sscanf(value, "%hhu, %hhu, %hhu, %hhu", &AppTheme.bg.r, &AppTheme.bg.g, &AppTheme.bg.b, &AppTheme.bg.a);
            else if (strcmp(key, "width") == 0) g_win_width = atoi(value);
            else if (strcmp(key, "height") == 0) g_win_height = atoi(value);
            else if (strcmp(key, "idle") == 0) sscanf(value, "%hhu, %hhu, %hhu, %hhu", &AppTheme.idle.r, &AppTheme.idle.g, &AppTheme.idle.b, &AppTheme.idle.a);
            else if (strcmp(key, "hover") == 0) sscanf(value, "%hhu, %hhu, %hhu, %hhu", &AppTheme.hover.r, &AppTheme.hover.g, &AppTheme.hover.b, &AppTheme.hover.a);
            else if (strcmp(key, "border") == 0) sscanf(value, "%hhu, %hhu, %hhu, %hhu", &AppTheme.border.r, &AppTheme.border.g, &AppTheme.border.b, &AppTheme.border.a);
            else if (strcmp(key, "ripple") == 0) sscanf(value, "%hhu, %hhu, %hhu, %hhu", &AppTheme.ripple.r, &AppTheme.ripple.g, &AppTheme.ripple.b, &AppTheme.ripple.a);
            else if (strcmp(key, "overlay") == 0) sscanf(value, "%hhu, %hhu, %hhu, %hhu", &AppTheme.overlay.r, &AppTheme.overlay.g, &AppTheme.overlay.b, &AppTheme.overlay.a);
            else if (strcmp(key, "text") == 0) sscanf(value, "%hhu, %hhu, %hhu, %hhu", &AppTheme.text.r, &AppTheme.text.g, &AppTheme.text.b, &AppTheme.text.a);

            else if (strcmp(key, "max_wallpapers") == 0) g_max_wallpapers = atoi(value);
            else if (strcmp(key, "base_thumb_size") == 0) g_base_thumb_size = atoi(value);
            else if (strcmp(key, "base_padding") == 0) g_base_padding = atoi(value);
            else if (strcmp(key, "border_thickness_bloom") == 0) g_border_thickness_bloom = atof(value);
            else if (strcmp(key, "max_threads") == 0)
            {
                g_max_threads = atoi(value);
                if (g_max_threads < 1) g_max_threads = 1;
            }
            else if (strcmp(key, "anim_speed") == 0) g_anim_speed = atof(value);
            else if (strcmp(key, "particle_count") == 0) g_particle_count = atoi(value);
            else if (strcmp(key, "ken_burns_duration") == 0) g_ken_burns_duration = atof(value);
            else if (strcmp(key, "max_fps") == 0) g_max_fps = atoi(value);
            else if (strcmp(key, "cache_max_age_days") == 0) g_cache_max_age_days = atoi(value);

            else if (strcmp(key, "startup_effect") == 0) g_startupEffect = ParseEffect(value);
            else if (strcmp(key, "keypress_effect") == 0) g_keypressEffect = ParseEffect(value);
            else if (strcmp(key, "exit_effect") == 0) g_exitEffect = ParseEffect(value);
        }
    }
    fclose(file);
}
