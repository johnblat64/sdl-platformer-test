#include "collisions.h"
#include "flecs.h"
#include <SDL2/SDL.h>
#include "camera.h"
#include "render.h"
#include "ray2d.h"
#include "solid_rect.h"
#include "ints.h"
#include "window.h"
#include "v2d.h"
#include "mouseState.h"
#include "util.h"



const SDL_Color SELECTED_PlatformPath_EDGE_COLOR = {20, 250, 20, 255};
const SDL_Color DEFAULT_PlatformPath_EDGE_COLOR = {255,255,255,255};
const SDL_Color DEFAULT_NODE_COLOR = {220,220,220,255};
const SDL_Color SELECTED_NODE_COLOR = {0, 255, 255, 255};



void renderSelectedPlatformNodeSystem(flecs::iter &it, SelectedForEditing *ss, Position *positions, PlatformPath *platformPaths){
    for(auto i : it){
        render_poly_line(
            positions[i], 
            platformPaths[i].nodes, 
            SELECTED_PlatformPath_EDGE_COLOR
        );
    }
}

// without SelectedForEditing Component
void renderUnselectedPlatformNodeSystem(flecs::iter &it, Position *positions, PlatformPath *platformPaths){
    for(auto i : it){
        render_poly_line(
            positions[i], 
            platformPaths[i].nodes, 
            DEFAULT_PlatformPath_EDGE_COLOR
        );
    }
}

// without SelectedForEditingNode Component
void renderUnselectedPlatformNodeNodesSystem(flecs::iter &it, Position *positions, PlatformPath *platformPaths){
    for(auto i : it){
        int size = platformPaths[i].nodes.size();
        for(int j = 0; j < size; j++){
            Position nodePositionGlobal;
            nodePositionGlobal.x = positions[i].x + platformPaths[i].nodes[j].x;
            nodePositionGlobal.y = positions[i].y + platformPaths[i].nodes[j].y;

            render_diamond(nodePositionGlobal, DEFAULT_NODE_COLOR);
        }
    }
}


void renderSelectedPlatformNodeNodesSystem(flecs::iter &it, SelectedForEditingNode *sns, Position *positions, PlatformPath *platformPaths){
    for(auto i : it){
        size_t size = platformPaths[i].nodes.size();

        for(int j = 0; j < size; j++){
            Position nodePositionGlobal;
            nodePositionGlobal.x = positions[i].x + platformPaths[i].nodes[j].x;
            nodePositionGlobal.y = positions[i].y + platformPaths[i].nodes[j].y;

            if(sns[i].idx == j){
                render_diamond(nodePositionGlobal, SELECTED_NODE_COLOR);
            }
            else {
                render_diamond(nodePositionGlobal, DEFAULT_NODE_COLOR);
            }
        }
    }
}


v2d rotateSensorPositionBasedOnGroundMode(v2d v, v2d o, GroundMode groundMode){
    v2d rotated = v;
    if(groundMode == LEFT_WALL_GM){
        rotated = v2dRotate90DegreesCW(rotated, o);
    }
    else if(groundMode == RIGHT_WALL_GM){
        rotated = v2dRotate90DegreesCCW(rotated, o);
    }
    else if(groundMode == CEILING_GM ){
        rotated = v2dRotate180Degrees(v, o);
    }
    return rotated;
}


void renderBorderedHorizontalLine(float startX, float endX, float y, SDL_Color fillColor, SDL_Color borderColor){
    SDL_SetRenderDrawColor(gRenderer,  fillColor.r, fillColor.g, fillColor.b ,fillColor.a);
    SDL_RenderDrawLineF(gRenderer, startX, y , endX, y);

    SDL_SetRenderDrawColor(gRenderer, borderColor.r, borderColor.g, borderColor.b, borderColor.a);
    SDL_RenderDrawLineF(gRenderer, startX , y - 1 , endX, y - 1);
    SDL_RenderDrawLineF(gRenderer, startX, y  + 1, endX , y + 1);
}


void renderBorderedVerticalLine(float startY, float endY, float x, SDL_Color fillColor, SDL_Color borderColor){
    SDL_SetRenderDrawColor(gRenderer,  fillColor.r, fillColor.g, fillColor.b ,fillColor.a);
    SDL_RenderDrawLineF(gRenderer, x, startY , x, endY);

    SDL_SetRenderDrawColor(gRenderer, borderColor.r, borderColor.g, borderColor.b, borderColor.a);
    SDL_RenderDrawLineF(gRenderer, x - 1, startY , x - 1, endY);
    SDL_RenderDrawLineF(gRenderer, x + 1, startY , x + 1, endY);
}




void renderSensorsSystem(flecs::iter &it, Position *positions, Sensors *sensorCollections, GroundMode *groundModes){
    // move this somewhere else
    Position centerScreen = {(float)gScreenWidth/2.0f, (float)gScreenHeight/2.0f}; 

    for(u64 i : it){

        {
            v2d lfRotatedPos = rotateSensorPositionBasedOnGroundMode(sensorCollections[i].rays[LF_SENSOR].startingPosition, v2d(0.0f, 0.0f), groundModes[i]);

            v2d worldPosition = util_local_to_world_position(lfRotatedPos, positions[i]);

            v2d positionInCamera = cam_util_world_position_to_camera_position(worldPosition);

            SDL_Color fillColor = {167, 236, 29 ,255};
            SDL_Color borderColor = {0,0,0,255};

            if(groundModes[i] == LEFT_WALL_GM){
                Position p = {worldPosition.x - sensorCollections[i].rays[LF_SENSOR].distance, worldPosition.y};
                render_line(worldPosition, p, fillColor);
                //renderBorderedHorizontalLine(positionInCamera.x, positionInCamera.x - sensorCollections[i].rays[LF_SENSOR].distance, positionInCamera.y, fillColor, borderColor);
            }
            else if(groundModes[i] == RIGHT_WALL_GM){
                renderBorderedHorizontalLine(positionInCamera.x, positionInCamera.x + sensorCollections[i].rays[LF_SENSOR].distance, positionInCamera.y, fillColor, borderColor);
            }
            else if(groundModes[i] == CEILING_GM){
                renderBorderedVerticalLine(positionInCamera.y, positionInCamera.y - sensorCollections[i].rays[LF_SENSOR].distance, positionInCamera.x, fillColor, borderColor);
            }
            else {
                Position p = {worldPosition.x, worldPosition.y + sensorCollections[i].rays[LF_SENSOR].distance};
                render_line(worldPosition, p, fillColor);
                //renderBorderedVerticalLine(positionInCamera.y, positionInCamera.y + sensorCollections[i].rays[LF_SENSOR].distance, positionInCamera.x, fillColor, borderColor);
            }
            
        }

        {
            v2d rfRotatedPos = rotateSensorPositionBasedOnGroundMode(sensorCollections[i].rays[RF_SENSOR].startingPosition, v2d(0.0f, 0.0f), groundModes[i]);

            v2d worldPosition = util_local_to_world_position(rfRotatedPos, positions[i]);

            v2d positionInCamera = cam_util_world_position_to_camera_position(worldPosition);

            SDL_Color fillColor = {29, 236, 158,255};
            SDL_Color borderColor = {0,0,0,255};

            if(groundModes[i] == LEFT_WALL_GM){
                renderBorderedHorizontalLine(positionInCamera.x, positionInCamera.x - sensorCollections[i].rays[RF_SENSOR].distance, positionInCamera.y, fillColor, borderColor);
            }
            else if(groundModes[i] == RIGHT_WALL_GM){
                renderBorderedHorizontalLine(positionInCamera.x, positionInCamera.x + sensorCollections[i].rays[RF_SENSOR].distance, positionInCamera.y, fillColor, borderColor);
            }
            else if(groundModes[i] == CEILING_GM) {
                renderBorderedVerticalLine(positionInCamera.y, positionInCamera.y - sensorCollections[i].rays[RF_SENSOR].distance, positionInCamera.x, fillColor, borderColor);
            }
            else {
                renderBorderedVerticalLine(positionInCamera.y, positionInCamera.y + sensorCollections[i].rays[RF_SENSOR].distance, positionInCamera.x, fillColor, borderColor);
            }
        }


        {
            v2d lwRotatedPos = rotateSensorPositionBasedOnGroundMode(sensorCollections[i].rays[LW_SENSOR].startingPosition, v2d(0.0f, 0.0f), groundModes[i]);

            v2d worldPosition = util_local_to_world_position(lwRotatedPos, positions[i]);

            v2d positionInCamera = cam_util_world_position_to_camera_position(worldPosition);

            SDL_Color fillColor = {255, 151, 241 ,255};
            SDL_Color borderColor = {0,0,0,255};

            if(groundModes[i] == LEFT_WALL_GM){
                renderBorderedVerticalLine(positionInCamera.y, positionInCamera.y - sensorCollections[i].rays[LW_SENSOR].distance, positionInCamera.x, fillColor, borderColor);
            }
            else if(groundModes[i] == RIGHT_WALL_GM){
                renderBorderedVerticalLine(positionInCamera.y, positionInCamera.y + sensorCollections[i].rays[LW_SENSOR].distance, positionInCamera.x, fillColor, borderColor);
            }
            else if(groundModes[i] == CEILING_GM) {
                renderBorderedHorizontalLine(positionInCamera.x, positionInCamera.x + sensorCollections[i].rays[LW_SENSOR].distance, positionInCamera.y, fillColor, borderColor);
            }
            else {
                renderBorderedHorizontalLine(positionInCamera.x, positionInCamera.x - sensorCollections[i].rays[LW_SENSOR].distance, positionInCamera.y, fillColor, borderColor);
            }
        }

        {
            v2d lwRotatedPos = rotateSensorPositionBasedOnGroundMode(sensorCollections[i].rays[LW_SENSOR].startingPosition, v2d(0.0f, 0.0f), groundModes[i]);

            v2d worldPosition = util_local_to_world_position(lwRotatedPos, positions[i]);

            v2d positionInCamera = cam_util_world_position_to_camera_position(worldPosition);

            SDL_Color fillColor = {244, 0, 50 ,255};
            SDL_Color borderColor = {0,0,0,255};

            if(groundModes[i] == LEFT_WALL_GM){
                renderBorderedVerticalLine(positionInCamera.y, positionInCamera.y + sensorCollections[i].rays[LW_SENSOR].distance, positionInCamera.x, fillColor, borderColor);
            }
            else if(groundModes[i] == RIGHT_WALL_GM){
                renderBorderedVerticalLine(positionInCamera.y, positionInCamera.y - sensorCollections[i].rays[LW_SENSOR].distance, positionInCamera.x, fillColor, borderColor);
            }
            else if (groundModes[i] == CEILING_GM) {
                renderBorderedHorizontalLine(positionInCamera.x, positionInCamera.x + sensorCollections[i].rays[LW_SENSOR].distance, positionInCamera.y, fillColor, borderColor);
            }
            else {
                renderBorderedHorizontalLine(positionInCamera.x, positionInCamera.x + sensorCollections[i].rays[LW_SENSOR].distance, positionInCamera.y, fillColor, borderColor);
            }

        }
    }
}


void renderUncommitedLinesToPlaceSystem(flecs::iter &it, Input *inputs, MouseState *mouseStates){
    for(int i : it){

        if(mouseStates[i].lmbCurrentState == INPUT_IS_PRESSED){

            Position mousePosition = {mouseStates[i].worldPosition.x, mouseStates[i].worldPosition.y};

            flecs::world ecs = it.world();

            Position tailnode;
            bool NoneSelected = true;
            auto f = ecs.filter<Position, PlatformPath, SelectedForEditing>();
            f.iter([&](flecs::iter &it, Position *ps, PlatformPath *platformPath, SelectedForEditing *selected){
                tailnode = platformPath[0].nodes[platformPath[0].nodes.size()-1];
                Position localPosition = v2d_sub(mousePosition, ps[0]);

                if(Input_is_pressed(inputs[i], "edit-angle-snap")){
                    
                    Position a = v2d_sub(localPosition, tailnode);
                    if(Input_is_pressed(inputs[i], "edit-angle-snap")){
                        if(abs(a.x) < abs(a.y)){
                            localPosition.x = platformPath[0].nodes.at(platformPath[0].nodes.size()-1).x;
                        }
                        else {
                            localPosition.y = platformPath[0].nodes.at(platformPath[0].nodes.size()-1).y;
                        }
                    }
                }
                

                
                NoneSelected = false;

                Position mouseAdjustedPos = v2d_add(localPosition, ps[0]);
                Position tailnodeAdjustedPos = v2d_add(tailnode, ps[0]);

                if(NoneSelected){
                    SDL_SetRenderDrawColor(gRenderer, 255,150,255,255);
                    render_diamond(mouseAdjustedPos, (SDL_Color){255,150,255,255});
                }
                else{
                    render_line(tailnodeAdjustedPos, mouseAdjustedPos, (SDL_Color){255,150,255,255});
                    SDL_SetRenderDrawColor(gRenderer, 255,150,255,255);
                    render_diamond(mouseAdjustedPos, (SDL_Color){255,150,255,255});

                }
            });

            

            
        }
    }
}