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


void renderPlatformVerticesNodesSystem(flecs::iter &it, Position *positions, PlatformVertices *PlatformVerticesCollection){
    for(auto i : it){
        int size = PlatformVerticesCollection[i].vals.size();
        for(int j = 0; j < size; j++){
            Position vertexPositionGlobal;
            vertexPositionGlobal.x = positions[i].x + PlatformVerticesCollection[i].vals[j].x;
            vertexPositionGlobal.y = positions[i].y + PlatformVerticesCollection[i].vals[j].y;

            renderDiamond(vertexPositionGlobal);
        }
    }
}




void renderSensorsSystem(flecs::iter &it, Position *positions, Sensors *sensorCollections){
    // move this somewhere else
    Position centerScreen = {(float)gScreenWidth/2.0f, (float)gScreenHeight/2.0f}; 

    for(u64 i : it){

        for(int j = 0; j < 2; j++){
            float scale;
            SDL_RenderGetScale(gRenderer,&scale, nullptr );
            Position scaledCenterScreen = {centerScreen.x / scale, centerScreen.y / scale};

            Position actualPosition;
            actualPosition.x = positions[i].x + sensorCollections[i].rays[j].startingPosition.x;
            actualPosition.y = positions[i].y + sensorCollections[i].rays[j].startingPosition.y;
            actualPosition.x = actualPosition.x - gCameraPosition.x + scaledCenterScreen.x;
            actualPosition.y = actualPosition.y - gCameraPosition.y + scaledCenterScreen.y;


            SDL_SetRenderDrawColor(gRenderer, 255,0,0,255);
            SDL_RenderDrawLineF(gRenderer, actualPosition.x, actualPosition.y , actualPosition.x, actualPosition.y  + sensorCollections[i].rays[j].distance);

            SDL_SetRenderDrawColor(gRenderer, 0,0,0,255);
            SDL_RenderDrawLineF(gRenderer, actualPosition.x - 1, actualPosition.y , actualPosition.x - 1, actualPosition.y  + sensorCollections[i].rays[j].distance);
            SDL_RenderDrawLineF(gRenderer, actualPosition.x + 1, actualPosition.y , actualPosition.x + 1, actualPosition.y  + sensorCollections[i].rays[j].distance);


        }
    }
}
