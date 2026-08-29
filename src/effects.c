#include "effects.h"
#include "state.h"

#include "raylib.h"
#include "raymath.h"

#include <math.h>

/* Only TriggerParticleBurst touches this, so it stays local instead of
 * living in state.h (see note in effects.h - nothing draws these yet). */
static Particle particles[MAX_POSSIBLE_PARTICLES];

void TriggerEffect(EffectType type, float duration)
{
    if (type == EFFECT_NONE)
    {
        return;
    }
    g_activeEffect = type;
    g_effectTimer = duration;
    g_effectDuration = duration;
}

void TriggerParticleBurst(Vector2 pos)
{
    for (int i = 0; i < g_particle_count; i++)
    {
        float angle = (float)GetRandomValue(0, 360) * DEG2RAD;
        float speed = (float)GetRandomValue(200, 400);
        particles[i] = (Particle){
            .pos = pos,
            .vel = {sinf(angle) * speed, cosf(angle) * speed},
            .life = 1.0f,
            .color = Fade(AppTheme.ripple, 0.5f + (float)GetRandomValue(0, 50) / 100.0f)
        };
    }
}
