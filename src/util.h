#ifndef RAYPAPER_UTIL_H
#define RAYPAPER_UTIL_H

#include "types.h"

/* Colored log line to stderr; level is one of raylib's LOG_INFO/LOG_WARNING/LOG_ERROR. */
void LogMessage(int level, const char *format, ...);

const char *get_home_dir(void);

/* Case-insensitive strstr. */
const char *stristr(const char *haystack, const char *needle);

/* Trims leading/trailing whitespace in place, returns pointer into str. */
char *trim_whitespace(char *str);

/* Maps a config/CLI effect name ("glitch", "blur", ...) to an EffectType. */
EffectType ParseEffect(const char *arg);

#endif // RAYPAPER_UTIL_H
