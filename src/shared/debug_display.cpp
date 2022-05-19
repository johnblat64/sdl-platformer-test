#include "collisions.h"
#include "flecs.h"
#include <SDL2/SDL.h>
#include "camera.h"
#include "render.h"
#include "ray2d.h"
#include "solid_rect.h"
#include "ints.h"
#include "window.h"
#include "shapes.h"



void renderPlatformVerticesSystem(flecs::iter &it, Position *positions, PlatformVertices *platformVerticesCollection){
    for(auto i : it){
        renderPolyLineInCamera(
            positions[i], 
            platformVerticesCollection[i].vals, 
            platformVerticesCollection[i].color
        );
    }
}




void renderRay2dCollectionsSystem(flecs::iter &it, Position *positions, std::vector<Ray2d> *ray2dCollections){
    // move this somewhere else
    Position centerScreen = {(float)gScreenWidth/2.0f, (float)gScreenHeight/2.0f}; 

    for(u64 i : it){
        std::vector<Ray2d> ray2ds = ray2dCollections[i];
        for(Ray2d ray : ray2ds){
            float scale;
            SDL_RenderGetScale(gRenderer,&scale, nullptr );
            Position scaledCenterScreen = {centerScreen.x / scale, centerScreen.y / scale};

            Position actualPosition;
            actualPosition.x = positions[i].x + ray.startingPosition.x;
            actualPosition.y = positions[i].y + ray.startingPosition.y;
            actualPosition.x = actualPosition.x - gCameraPosition.x + scaledCenterScreen.x;
            actualPosition.y = actualPosition.y - gCameraPosition.y + scaledCenterScreen.y;


            int iActualX = (int)actualPosition.x;
            int iActualY = (int)actualPosition.y;
            int iRayDistance = (int)ray.distance;

            SDL_SetRenderDrawColor(gRenderer, 255,0,0,255);
            SDL_RenderDrawLine(gRenderer, iActualX, iActualY , iActualX, iActualY  + iRayDistance);

            SDL_SetRenderDrawColor(gRenderer, 0,0,0,255);
            SDL_RenderDrawLine(gRenderer, iActualX - 1, iActualY , iActualX - 1, iActualY  + iRayDistance);
            SDL_RenderDrawLine(gRenderer, iActualX + 1, iActualY , iActualX + 1, iActualY  + iRayDistance);


        }
    }
}
