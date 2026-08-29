#include "cache.h"
#include "state.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <stdint.h>
#include <time.h>

static char g_cache_dir[1024] = {0};
static char g_cache_manifest_path[1200] = {0};
static pthread_mutex_t g_manifest_mutex = PTHREAD_MUTEX_INITIALIZER;

static void mkdir_p(const char *path)
{
    char tmp[1024];
    snprintf(tmp, sizeof(tmp), "%s", path);
    size_t len = strlen(tmp);
    if (len && tmp[len - 1] == '/') tmp[len - 1] = '\0';

    for (char *p = tmp + 1; *p; p++)
    {
        if (*p == '/')
        {
            *p = '\0';
            mkdir(tmp, 0755);
            *p = '/';
        }
    }
    mkdir(tmp, 0755);
}

static uint64_t fnv1a_hash(const char *str)
{
    uint64_t hash = 14695981039346656037ULL;
    for (; *str; str++)
    {
        hash ^= (unsigned char)(*str);
        hash *= 1099511628211ULL;
    }
    return hash;
}

/* Cache key includes path + mtime + thumb size, so edited/replaced files
 * and config changes to base_thumb_size both correctly invalidate. */
static uint64_t ComputeThumbnailHash(const char *src_path, long mtime)
{
    char key[1200];
    snprintf(key, sizeof(key), "%s|%ld|%d", src_path, mtime, g_base_thumb_size);
    return fnv1a_hash(key);
}

static void GetThumbnailCachePathFromHash(uint64_t hash, char *out, size_t out_size)
{
    snprintf(out, out_size, "%s/%016llx.png", g_cache_dir, (unsigned long long)hash);
}

/* Appended (thread-safe) every time a thumbnail is freshly generated, so
 * PruneThumbnailCache() can later map cache files back to their source. */
static void AppendManifestEntry(uint64_t hash, const char *src_path, long mtime)
{
    pthread_mutex_lock(&g_manifest_mutex);
    FILE *f = fopen(g_cache_manifest_path, "a");
    if (f)
    {
        fprintf(f, "%016llx\t%s\t%ld\n", (unsigned long long)hash, src_path, mtime);
        fclose(f);
    }
    pthread_mutex_unlock(&g_manifest_mutex);
}

/* Wipes every cached thumbnail and the manifest. Used by --clear-cache. */
static void ClearThumbnailCache(void)
{
    DIR *d = opendir(g_cache_dir);
    if (d)
    {
        struct dirent *entry;
        int removed = 0;
        while ((entry = readdir(d)) != NULL)
        {
            if (entry->d_name[0] == '.') continue;
            char full[1300];
            snprintf(full, sizeof(full), "%s/%s", g_cache_dir, entry->d_name);
            if (unlink(full) == 0) removed++;
        }
        closedir(d);
        LogMessage(LOG_INFO, "Thumbnail cache: cleared %d file(s)", removed);
    }
    unlink(g_cache_manifest_path);
}

/* Startup sweep: drops manifest entries whose source changed/vanished, and
 * entries older than g_cache_max_age_days. Cheap - just stat()s and
 * unlink()s, no image decoding. */
static void PruneThumbnailCache(void)
{
    FILE *mf = fopen(g_cache_manifest_path, "r");
    if (!mf) return;

    char tmp_manifest[1300];
    snprintf(tmp_manifest, sizeof(tmp_manifest), "%s.tmp", g_cache_manifest_path);
    FILE *out = fopen(tmp_manifest, "w");
    if (!out) { fclose(mf); return; }

    time_t now = time(NULL);
    long max_age_secs = (g_cache_max_age_days > 0) ? (long)g_cache_max_age_days * 86400L : -1;

    char line[1300];
    int kept = 0, pruned = 0;
    while (fgets(line, sizeof(line), mf))
    {
        char *nl = strchr(line, '\n');
        if (nl) *nl = '\0';

        char *tab1 = strchr(line, '\t');
        if (!tab1) continue;
        *tab1 = '\0';
        char *src_path = tab1 + 1;

        char *tab2 = strrchr(src_path, '\t');
        if (!tab2) continue;
        *tab2 = '\0';
        long recorded_mtime = atol(tab2 + 1);

        char hash_hex[24];
        strncpy(hash_hex, line, sizeof(hash_hex) - 1);
        hash_hex[sizeof(hash_hex) - 1] = '\0';
        char cache_file[1300];
        snprintf(cache_file, sizeof(cache_file), "%s/%s.png", g_cache_dir, hash_hex);

        struct stat src_st, cache_st;
        bool source_ok = (stat(src_path, &src_st) == 0) && ((long)src_st.st_mtime == recorded_mtime);
        bool cache_exists = (stat(cache_file, &cache_st) == 0);
        bool too_old = (max_age_secs >= 0) && cache_exists && (difftime(now, cache_st.st_mtime) > (double)max_age_secs);

        if (!source_ok || !cache_exists || too_old)
        {
            if (cache_exists) unlink(cache_file);
            pruned++;
            continue;
        }

        fprintf(out, "%s\t%s\t%ld\n", hash_hex, src_path, recorded_mtime);
        kept++;
    }

    fclose(mf);
    fclose(out);
    rename(tmp_manifest, g_cache_manifest_path);

    if (pruned > 0)
        LogMessage(LOG_INFO, "Thumbnail cache: pruned %d stale/expired entr%s (%d kept)",
                   pruned, pruned == 1 ? "y" : "ies", kept);
}

void InitThumbnailCache(bool clear_requested)
{
    const char *xdg_cache = getenv("XDG_CACHE_HOME");
    if (xdg_cache && *xdg_cache)
        snprintf(g_cache_dir, sizeof(g_cache_dir), "%s/raypaper/thumbnails", xdg_cache);
    else
        snprintf(g_cache_dir, sizeof(g_cache_dir), "%s/.cache/raypaper/thumbnails", get_home_dir());

    snprintf(g_cache_manifest_path, sizeof(g_cache_manifest_path), "%s/manifest.tsv", g_cache_dir);

    mkdir_p(g_cache_dir);
    LogMessage(LOG_INFO, "Thumbnail cache directory: %s", g_cache_dir);

    if (clear_requested)
        ClearThumbnailCache();
    else
        PruneThumbnailCache();
}

bool TryLoadCachedThumbnail(const char *src_path, Image *out)
{
    struct stat st;
    long mtime = (stat(src_path, &st) == 0) ? (long)st.st_mtime : 0;
    uint64_t hash = ComputeThumbnailHash(src_path, mtime);

    char cache_path[1200];
    GetThumbnailCachePathFromHash(hash, cache_path, sizeof(cache_path));

    if (access(cache_path, F_OK) != 0) return false;

    Image cached = LoadImage(cache_path);
    if (!cached.data)
    {
        LogMessage(LOG_WARNING, "Cached thumbnail unreadable, regenerating: %s", cache_path);
        return false;
    }

    *out = cached;
    return true;
}

void SaveThumbnailToCache(const char *src_path, Image thumb)
{
    struct stat st;
    long mtime = (stat(src_path, &st) == 0) ? (long)st.st_mtime : 0;
    uint64_t hash = ComputeThumbnailHash(src_path, mtime);

    char cache_path[1200];
    GetThumbnailCachePathFromHash(hash, cache_path, sizeof(cache_path));

    if (ExportImage(thumb, cache_path))
    {
        AppendManifestEntry(hash, src_path, mtime);
    }
    else
    {
        LogMessage(LOG_WARNING, "Failed to write thumbnail cache: %s", cache_path);
    }
}
