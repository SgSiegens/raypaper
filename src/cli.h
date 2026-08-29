#ifndef RAYPAPER_CLI_H
#define RAYPAPER_CLI_H

#include <stdbool.h>

void print_help(void);

/* Scans argv for --help; if present, prints help and exits the process
 * immediately (matches the original's early-exit-before-config behavior).
 * No-op otherwise. */
void CheckForHelpFlag(int argc, char **argv);

/* Scans argv for --clear-cache. Checked in the same early pass as --help
 * (before config/cache init), matching the order the flag needs to affect
 * InitThumbnailCache(). Returns true if present. */
bool CheckForClearCacheFlag(int argc, char **argv);

/* Parses the remaining flags (effects, --filename, --width/--height) into
 * state.h globals, sets *filename_only_out, and returns the positional
 * wallpaper-directory argument, or NULL if none was given. */
const char *ParseCommandLineArgs(int argc, char **argv, bool *filename_only_out);

#endif // RAYPAPER_CLI_H
