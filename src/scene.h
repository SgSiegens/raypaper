#ifndef RAYPAPER_SCENE_H
#define RAYPAPER_SCENE_H

#include <stdbool.h>

/* Advances the fly-in/layout-mode animation for the currently filtered
 * wallpapers, picks up hover state, and draws the thumbnail grid/river/
 * wave plus the full-screen preview overlay. Called once per frame from
 * inside render.c's mainTarget texture pass. */
void UpdateAndDrawScene(int filteredCount, int *filteredIndices, float delta, bool isPreviewing, bool isSearching);

#endif // RAYPAPER_SCENE_H
