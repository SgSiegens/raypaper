#include "util.h"

#include "raylib.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <unistd.h>
#include <pwd.h>

void LogMessage(int level, const char *format, ...)
{
    const char *level_str = "";
    const char *color_code = "";
    const char *color_reset = "\033[0m";

    switch (level)
    {
        case LOG_INFO:
            level_str = "INFO";
            color_code = "\033[0;34m";
            break;
        case LOG_WARNING:
            level_str = "WARN";
            color_code = "\033[0;33m";
            break;
        case LOG_ERROR:
            level_str = "ERROR";
            color_code = "\033[0;31m";
            break;
    }

    fprintf(stderr, "%s[%s] ", color_code, level_str);

    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);

    fprintf(stderr, "%s\n", color_reset);
}

const char *get_home_dir(void)
{
    const char *home = getenv("HOME");
    if (!home)
    {
        struct passwd *pw = getpwuid(getuid());
        if (pw)
        {
            home = pw->pw_dir;
        }
    }
    return home ? home : ".";
}

const char *stristr(const char *haystack, const char *needle)
{
    if (!needle || !*needle)
    {
        return haystack;
    }
    for (; *haystack; ++haystack)
    {
        const char *h = haystack;
        const char *n = needle;
        while (*h && *n && (tolower((unsigned char)*h) == tolower((unsigned char)*n)))
        {
            h++;
            n++;
        }
        if (!*n)
        {
            return haystack;
        }
    }
    return NULL;
}

char *trim_whitespace(char *str)
{
    char *end;
    while (isspace((unsigned char)*str)) str++;
    if (*str == 0) return str;
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return str;
}

EffectType ParseEffect(const char *arg)
{
    if (strcasecmp(arg, "none") == 0) return EFFECT_NONE;
    if (strcasecmp(arg, "glitch") == 0) return EFFECT_GLITCH;
    if (strcasecmp(arg, "blur") == 0) return EFFECT_BLUR;
    if (strcasecmp(arg, "pixelate") == 0) return EFFECT_PIXELATE;
    if (strcasecmp(arg, "reveal") == 0) return EFFECT_REVEAL;
    if (strcasecmp(arg, "shake") == 0) return EFFECT_SHAKE;
    if (strcasecmp(arg, "collapse") == 0) return EFFECT_COLLAPSE;

    LogMessage(LOG_WARNING, "Unrecognized effect '%s'. Defaulting to 'none'.", arg);
    return EFFECT_NONE;
}
