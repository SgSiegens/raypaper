#ifndef RAYPAPER_EFFECTS_H
#define RAYPAPER_EFFECTS_H

#include "types.h"
#include "raylib.h"

/* Starts a timed post-processing effect (glitch/blur/pixelate/shake/
 * collapse/reveal). No-op for EFFECT_NONE. main.c ticks it down each
 * frame and feeds g_effectIntensity to the post shader. */
void TriggerEffect(EffectType type, float duration);

/* Spawns a burst of particles at pos. Note: nothing currently draws
 * particles[] - this was true in the original too, kept as-is. */
void TriggerParticleBurst(Vector2 pos);

#endif // RAYPAPER_EFFECTS_H
