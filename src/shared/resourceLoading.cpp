#include <flecs.h>
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <position.h>
#include "ints.h"

template<typename T> 
void copyDynamicArrayToVector(T *arr, size_t size, std::vector<T> &vec){
    for(int i = 0; i < size; i++){
        vec.push_back(arr[i]);
    }
}



void loadPlatformVertices(flecs::world &ecs){

    //ecs.defer_begin();

    SDL_RWops *loadContext = SDL_RWFromFile("platformVertices", "rb");
    if(loadContext == NULL){
        printf("No file!\n");
        return;
    }
    size_t numEntities = 0;

    SDL_RWread(loadContext, &numEntities, sizeof(size_t), 1);
    Position *positions = (Position *)malloc(sizeof(Position)*numEntities);
    PlatformVertexCollection *platformVertexCollections = (PlatformVertexCollection *)calloc(numEntities, sizeof(PlatformVertexCollection)); 
    SDL_RWread(loadContext, positions, sizeof(Position), numEntities);



    for(int i = 0; i < numEntities; i++){
        size_t count = 0;
        SDL_RWread(loadContext, &count, sizeof(size_t), 1);

        Position *platformVertices = (Position *)malloc(sizeof(Position) * count);
        SDL_RWread(loadContext, platformVertices, sizeof(Position), count);

        PlatformVertexCollection pvc;
        pvc.edgeColor = (SDL_Color){255,255,255,255};
    
        copyDynamicArrayToVector<Position>(platformVertices, count, pvc.vals);


        platformVertexCollections[i].edgeColor = pvc.edgeColor;
        platformVertexCollections[i].nodeColor = (SDL_Color){0,255,255,255};
        platformVertexCollections[i].vals = pvc.vals;


        free(platformVertices);
    }

    SDL_RWclose(loadContext);
    

    for(int i = 0; i < numEntities; i++){
        flecs::entity e = ecs.entity();
        e.set<Position>(positions[i]);
        e.set<PlatformVertexCollection>(platformVertexCollections[i]);
    }

    free(platformVertexCollections);
    free(positions);

   //ecs.defer_end();

}