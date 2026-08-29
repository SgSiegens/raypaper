#include "wallpaper.h"
#include "state.h"
#include "util.h"
#include "cache.h"

#include "raylib.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <dirent.h>
#include <unistd.h>
#include <stdatomic.h>

/* File-local, matching the original: nothing outside this module needs
 * per-thumbnail decoding directly, only through LoaderThread. */
static Image LoadThumbnailImage(const char *fp)
{
    Image cached;
    if (TryLoadCachedThumbnail(fp, &cached))
    {
        return cached;
    }

    Image i = LoadImage(fp);
    if (i.data)
    {
        int cs = (i.width < i.height) ? i.width : i.height;
        Rectangle cr = {
            (float)(i.width - cs) / 2,
            (float)(i.height - cs) / 2,
            (float)cs,
            (float)cs
        };
        ImageCrop(&i, cr);
        ImageResize(&i, g_base_thumb_size, g_base_thumb_size);
        SaveThumbnailToCache(fp, i);
    }
    else
    {
        LogMessage(LOG_WARNING, "Failed to load thumbnail for: %s", fp);
    }
    return i;
}

void *FullPreviewLoaderThread(void *p)
{
    Image i = LoadImage((const char *)p);
    if (i.data)
    {
        pendingFullImage = i;
        atomic_store(&fullImagePending, true);
    }
    return NULL;
}

void *LoaderThread(void *a)
{
    (void)a;
    while (atomic_load(&loader_running))
    {
        int i = atomic_fetch_add(&next_load_index, 1);
        if (i >= wallpaper_count)
        {
            usleep(100000);
            continue;
        }
        if (!atomic_load(&imagePending[i]) && !atomic_load(&wallpapers[i].loaded))
        {
            Image img = LoadThumbnailImage(wallpapers[i].path);
            if (img.data)
            {
                pendingImages[i] = img;
                atomic_store(&imagePending[i], true);
            }
        }
    }
    return NULL;
}

void LoadWallpapers(const char *dir)
{
    DIR *dp = opendir(dir);
    if (!dp)
    {
        LogMessage(LOG_ERROR, "Could not open wallpaper directory: %s", dir);
        return;
    }
    struct dirent *entry;
    while ((entry = readdir(dp)) != NULL && wallpaper_count < g_max_wallpapers)
    {
        if (entry->d_type != DT_REG)
        {
            continue;
        }
        const char *ext = strrchr(entry->d_name, '.');
        if (!ext || (strcasecmp(ext, ".jpg") != 0 && strcasecmp(ext, ".jpeg") != 0 && strcasecmp(ext, ".png") != 0))
        {
            continue;
        }

        char *fullpath;
        if (asprintf(&fullpath, "%s/%s", dir, entry->d_name) == -1) continue;

        wallpapers[wallpaper_count] = (Wallpaper){
            .path = strdup(fullpath),
            .filename = strdup(entry->d_name),
            .loaded = false,
            .hoverAnim = 0.0f,
            .animPos = {(float)GetRandomValue(-500, 500), (float)GetRandomValue(800, 1200)},
            .animSize = {g_base_thumb_size, g_base_thumb_size}
        };
        atomic_store(&imagePending[wallpaper_count], false);
        wallpaper_count++;
    }
    closedir(dp);
}
