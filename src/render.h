#ifndef RAYPAPER_RENDER_H
#define RAYPAPER_RENDER_H

#include <stdbool.h>

/* Loads the post-processing + blur shaders and allocates the render
 * textures (main scene, bloom mask, blur ping-pong, hi-res bloom mask)
 * at the given size. Call once after InitWindow(). */
void InitRenderPipeline(int width, int height);

/* Call once per frame, before RenderFrame(). Reallocates the render
 * textures if the window was resized this frame. */
void HandleRenderResize(void);

/* Draws one full frame: scene + search bar into the main render target,
 * the hover-glow bloom mask, a two-pass gaussian blur of that mask, then
 * composites everything through the post-processing shader and presents
 * it, followed by the bottom help-text line. */
void RenderFrame(int filteredCount, int *filteredIndices, float delta, bool isPreviewing, bool isSearching);

/* Unloads shaders and render textures. Call before CloseWindow(). */
void ShutdownRenderPipeline(void);

#endif // RAYPAPER_RENDER_H
