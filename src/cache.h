#ifndef RAYPAPER_CACHE_H
#define RAYPAPER_CACHE_H

#include "raylib.h"
#include <stdbool.h>

/*
 * On-disk thumbnail cache.
 *
 * Decoding a full-resolution wallpaper just to shrink it to a ~150px
 * thumbnail is the expensive part of startup. This caches the already-
 * shrunk result at ~/.cache/raypaper/thumbnails (or
 * $XDG_CACHE_HOME/raypaper/thumbnails) keyed by source path + mtime +
 * the configured thumb size, so a source file being edited/replaced or a
 * base_thumb_size change both correctly invalidate their old entry.
 *
 * A manifest (thumbnails/manifest.tsv) maps each cache file back to its
 * source path + mtime, so InitThumbnailCache() can sweep on every startup
 * and drop entries that are stale (source changed/vanished) or older
 * than cache_max_age_days. wallpaper.c doesn't need to know any of this -
 * it just asks to load or save a thumbnail for a path.
 */

/* Sets up the cache directory + manifest. If clear_requested, wipes the
 * cache entirely (--clear-cache); otherwise prunes stale/expired entries.
 * Call once at startup, after config is loaded (cache_max_age_days must
 * already be set). */
void InitThumbnailCache(bool clear_requested);

/* Cache hit: loads the pre-shrunk thumbnail for src_path into *out and
 * returns true. Cache miss (or unreadable cache file): returns false and
 * leaves *out untouched. */
bool TryLoadCachedThumbnail(const char *src_path, Image *out);

/* Saves a freshly generated thumbnail (already cropped/resized by the
 * caller) to the cache for src_path, and records it in the manifest. */
void SaveThumbnailToCache(const char *src_path, Image thumb);

#endif // RAYPAPER_CACHE_H
