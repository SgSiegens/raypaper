#ifndef RAYPAPER_SHADERS_H
#define RAYPAPER_SHADERS_H

/* Post-processing pass: bloom composite, vignette, grain, and the
 * glitch/blur/pixelate/shake/collapse/reveal effect variations. */
extern const char *postProcessingFs;

/* Simple separable gaussian blur, used both directions for the bloom mask. */
extern const char *blurFs;

#endif // RAYPAPER_SHADERS_H
