#ifndef timesteph
#define timesteph

#include <SDL2/SDL.h>
#include "util.h"

#define MAX_DELTA_TIME 0.0167

float getDeltaTime(){
    static float previousFrameTicks = 0.0f;
    float currentFrameTicks = SDL_GetTicks() / 1000.0f;
    float deltaTime = currentFrameTicks - previousFrameTicks;
    deltaTime = MIN(MAX_DELTA_TIME, deltaTime); // adjusting for pausing, debugging breaks, etc
    return deltaTime;
}

#endif