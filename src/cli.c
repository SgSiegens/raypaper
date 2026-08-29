#include "cli.h"
#include "state.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_help(void)
{
    printf("Raypaper - wallpaper picker for Linux.\n\n");
    printf("USAGE:\n");
    printf("  raypaper [OPTIONS] [PATH]\n\n");
    printf("ARGUMENTS:\n");
    printf("  [PATH]              Optional path to the directory containing wallpapers.\n");
    printf("                      Defaults to '~/Pictures/'.\n\n");
    printf("OPTIONS:\n");
    printf("  --help              Show this help message and exit.\n");
    printf("  --filename          Print only the filename of the selected wallpaper to stdout.\n");
    printf("  --clear-cache       Wipe the on-disk thumbnail cache before starting.\n");
    printf("  --width <pixels>    Set the initial window width.\n");
    printf("  --height <pixels>   Set the initial window height.\n");
    printf("  --startup-effect <effect>\n");
    printf("  --keypress-effect <effect>\n");
    printf("  --exit-effect <effect>\n");
    printf("                      Override the configured visual effects on certain events.\n");
    printf("                      Available effects: none, glitch, blur, pixelate, shake, collapse, reveal\n\n");
    printf("KEYBINDINGS:\n");
    printf("  NAVIGATION:\n");
    printf("    h, j, k, l / Arrows Move selection. Keys repeat when held.\n");
    printf("    Mouse Wheel       Scroll through wallpapers.\n");
    printf("    Ctrl + Mouse Wheel  Zoom thumbnail scaling.\n\n");
    printf("  ACTIONS:\n");
    printf("    Enter / LMB         Select the highlighted wallpaper and exit.\n");
    printf("    L-Shift / RMB       Show a full-screen preview of the highlighted wallpaper.\n");
    printf("    /                   Enter search mode. Type to filter wallpapers by name.\n");
    printf("    ESC                 Closes Preview, then Search, then the App.\n\n");
    printf("  VIEW MODES:\n");
    printf("    1, 2, 3, 4          Switch between different layout modes:\n");
    printf("                        1: Grid\n");
    printf("                        2: Horizontal River\n");
    printf("                        3: Vertical River\n");
    printf("                        4: Wave\n\n");
    printf("CONFIGURATION:\n");
    printf("  Raypaper can be fully customized by editing the configuration file located at:\n");
    printf("  ~/.config/raypaper/raypaper.conf\n\n");
    printf("  cache_max_age_days  Auto-prune cached thumbnails older than this many days on\n");
    printf("                      startup (default 30). Set to 0 to keep entries forever.\n");
    printf("  Thumbnails are cached at ~/.cache/raypaper/thumbnails\n");
    printf("  ($XDG_CACHE_HOME/raypaper/thumbnails if set).\n");
}

void CheckForHelpFlag(int argc, char **argv)
{
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--help") == 0)
        {
            print_help();
            exit(0);
        }
    }
}

bool CheckForClearCacheFlag(int argc, char **argv)
{
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--clear-cache") == 0)
        {
            return true;
        }
    }
    return false;
}

const char *ParseCommandLineArgs(int argc, char **argv, bool *filename_only_out)
{
    const char *wallpaper_path = NULL;

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--startup-effect") == 0 && i + 1 < argc)
        {
            g_startupEffect = ParseEffect(argv[++i]);
        }
        else if (strcmp(argv[i], "--keypress-effect") == 0 && i + 1 < argc)
        {
            g_keypressEffect = ParseEffect(argv[++i]);
        }
        else if (strcmp(argv[i], "--exit-effect") == 0 && i + 1 < argc)
        {
            g_exitEffect = ParseEffect(argv[++i]);
        }
        else if (strcmp(argv[i], "--filename") == 0)
        {
            *filename_only_out = true;
        }
        else if (strcmp(argv[i], "--width") == 0 && i + 1 < argc)
        {
            g_win_width = atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "--height") == 0 && i + 1 < argc)
        {
            g_win_height = atoi(argv[++i]);
        }
        else if (argv[i][0] != '-' && wallpaper_path == NULL)
        {
            wallpaper_path = argv[i];
        }
    }

    return wallpaper_path;
}
