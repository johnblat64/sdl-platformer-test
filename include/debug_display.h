#ifndef dbg_display_h
#define dbg_display_h

#include "ray2d.h"
#include <vector>

void renderRectangularObjectsSystem(flecs::iter &it, Position *positions, SolidRect *rectObjects);
void renderRay2dCollectionsSystem(flecs::iter &it, Position *positions, std::vector<Ray2d> *ray2dCollections);

#endif