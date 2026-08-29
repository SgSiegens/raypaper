#ifndef RAYPAPER_WALLPAPER_H
#define RAYPAPER_WALLPAPER_H

/* Scans dir for .jpg/.jpeg/.png files and populates the wallpapers[] array
 * (up to g_max_wallpapers entries), each with animPos/animSize seeded for
 * their fly-in entrance animation. */
void LoadWallpapers(const char *dir);

/* Background worker: repeatedly claims the next un-thumbnailed wallpaper
 * and decodes+crops+resizes it into pendingImages[], for main.c to upload
 * to the GPU. Runs until loader_running is cleared. */
void *LoaderThread(void *arg);

/* One-shot worker started when the user opens a full-screen preview;
 * decodes the original (non-thumbnail) image into pendingFullImage. */
void *FullPreviewLoaderThread(void *path);

#endif // RAYPAPER_WALLPAPER_H
