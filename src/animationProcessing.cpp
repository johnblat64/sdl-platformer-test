#include "animation.h"
#include "render.h"
#include "window.h"
#include "spriteSheetsProcessing.h"
#include "animationProcessing.h"
#include "position.h"
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "flecs.h"
#include "input.h"

//
// HELPER
//
u32 RCToI(u32 row, u32 col, u32 numRows){
    return (row * numRows) + col;
}


void iToRC(u32 i, u32 numRows, u32 *row, u32 *col){
    *row = i / (numRows);
    *col = i % (numRows);
}


//
// SYSTEMS
//
void animationsAccumulationSystem(flecs::iter &it, AnimatedSprite *animatedSprites){

    for(auto i : it){
        u32 currentAnimation = animatedSprites[i].currentAnimation;

        if(isAnimationFinished(&animatedSprites[i].animations[currentAnimation])){
            if(animatedSprites[i].animations[currentAnimation].isLoop == false) continue;
            animatedSprites[i].animations[currentAnimation].currentFrame = 0;
        }

        animatedSprites[i].animations[currentAnimation].accumulator += it.delta_time();

        if(animatedSprites[i].animations[currentAnimation].accumulator > animatedSprites[i].animations[currentAnimation].msPerFrame){            
            animatedSprites[i].animations[currentAnimation].accumulator = 0.0f;
            animatedSprites[i].animations[currentAnimation].currentFrame++;

            
        }
    }
    
    
}


Position gCameraPosition;
Position gCenterScreen = {320, 240};

void renderingAnimatedSpritesSystem(flecs::iter &it, AnimatedSprite *animatedSprites, Position *positions){
    for(auto i : it){
        SpriteSheet spriteSheet = getSpriteSheet(animatedSprites[i].spriteSheetId);
        float cell_w = spriteSheet.w / spriteSheet.numCellRows;
        float cell_h = spriteSheet.h / spriteSheet.numCellCols;

        float scale;
        SDL_RenderGetScale(gRenderer,&scale, NULL );
        Position scaledCenterScreen = {gCenterScreen.x / scale, gCenterScreen.y / scale};
        
        SDL_Rect renderRect;
        renderRect.x = (positions[i].x - gCameraPosition.x + scaledCenterScreen.x) - (cell_w*0.5f);
        renderRect.y = (positions[i].y - gCameraPosition.y + scaledCenterScreen.y) - (cell_h*0.5f);
        
        renderRect.w = cell_w;
        renderRect.h = cell_h;
        

        Animation currentAnimation = animatedSprites[i].animations[animatedSprites[i].currentAnimation];
        SDL_Rect srcRect;
        u32 spriteSheetCellIndex = currentAnimation.arrFrames[currentAnimation.currentFrame];
        u32 cell_r;
        u32 cell_c;
        iToRC(spriteSheetCellIndex, spriteSheet.numCellRows, &cell_r, &cell_c);
        srcRect.x = cell_c * cell_w;
        srcRect.y = cell_r * cell_h;
        srcRect.w = cell_w;
        srcRect.h = cell_h;
        SDL_RendererFlip flip = animatedSprites[i].flip == SDL_FLIP_HORIZONTAL ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE; 
        SDL_RenderCopyEx(gRenderer, spriteSheet.texture, &srcRect, &renderRect, 0.0, NULL, flip);
    }
    
}


void keyStateAnimationSetterSystem(flecs::iter &it, AnimatedSprite *animatedSprites, KeyboardState *keyStatesCollections){
    for(auto i : it){
        u8 *keyStates = keyStatesCollections[i].keyStates;
        if(keyStates[SDL_SCANCODE_0]){
            animatedSpritePlay(&animatedSprites[i], "walk");
        } 
        if(keyStates[SDL_SCANCODE_1]){
            animatedSpritePlay(&animatedSprites[i], "run");

        } 
        if(keyStates[SDL_SCANCODE_2]){
            animatedSpritePlay(&animatedSprites[i], "stand-attack");
            restartAnimation(&animatedSprites[i].animations[animatedSprites[i].currentAnimation]);
        } 
        // if(keyStates[SDL_SCANCODE_3]){
        //     animatedSprites[i].currentAnimation = 3;
        // } 
    }
}


void keyStateFlipSystem(flecs::iter &it, AnimatedSprite *animatedSprites, KeyboardState *keyStatesCollections){
    for(auto i : it){
        u8 *keyStates = keyStatesCollections[i].keyStates;

        if(keyStates[SDL_SCANCODE_A]){
            animatedSprites[i].flip = SDL_FLIP_HORIZONTAL;
        }
        else if(keyStates[SDL_SCANCODE_D]){
            animatedSprites[i].flip = SDL_FLIP_NONE;
        }
    }
}


// 
// MANAGING, LOADING, CREATION, ETC
// 

void animatedSpritePlay(AnimatedSprite *as, const char *animationName){
    for(int i = 0; i < as->numAnimations; i++){
        if(strncmp(as->animations[i].name, animationName, 16) == 0){
            as->currentAnimation = i;
            return;
        }
    }
    assert(0 && "Animation name was not found in list of animations");
}

AnimatedSprite createAnimatedSprite(u32 spriteSheetId){
    AnimatedSprite animatedSprite;
    animatedSprite.numAnimations = 0;
    animatedSprite.currentAnimation = 0;
    animatedSprite.spriteSheetId = spriteSheetId;
    return animatedSprite;
}


void addNewAnimationToAnimatedSprite(AnimatedSprite *animatedSprite){
    Animation animation;
    animation.numFrames = 0;
    animation.currentFrame = 0;
    animation.accumulator = 0.0f;
    animation.fps = 0.0f;
    animation.msPerFrame = 0.0f;

    animatedSprite->animations[animatedSprite->numAnimations] = animation;
    animatedSprite->numAnimations++;
}


void overwriteAnimationOnAnimatedSprite(AnimatedSprite *animatedSprite, u32 animationIndex, Animation animation){
    animatedSprite->animations[animationIndex] = animation;
}


void restartAnimation(Animation *animation){
    animation->currentFrame = 0;
}


bool isAnimationFinished(Animation *animation){
    return animation->currentFrame >= animation->numFrames - 1;
}