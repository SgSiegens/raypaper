#ifndef RAYPAPER_CONFIG_H
#define RAYPAPER_CONFIG_H

/* Fills in state.h globals (AppTheme, g_* tunables, g_win_width/height,
 * effect defaults) with hardcoded defaults. Call before ParseConfigFile(). */
void LoadDefaultConfig(void);

/* Reads ~/.config/raypaper/raypaper.conf (creating the directory if
 * missing) and overrides whatever keys it finds on top of the defaults. */
void ParseConfigFile(void);

#endif // RAYPAPER_CONFIG_H
