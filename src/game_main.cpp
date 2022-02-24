#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <string>
#include <vector>
#include "animation.h"
#include "animationProcessing.h"
#include "ints.h"
#include "spriteSheets.h"
#include "spriteSheetsProcessing.h"
#include "velocity.h"
#include "movement.h"
#include "input.h"
#include "flecs.h"
#include "ray2d.h"
#include "states.h"
#include "solid_rect.h"
#include "collisions.h"
#include "debug_display.h"

SDL_Renderer *gRenderer;
SDL_Window *gWindow;

SpriteSheet gSpriteSheets[MAX_SPRITE_SHEETS]; 
size_t gNumSpriteSheets = 0;


typedef struct Dimensions Dimensions;
struct Dimensions {
    float w, h;
};


int main(){
    bool quit = false;
    /**
     * FLECS SETUP
     * 
     */

    flecs::world world;
    flecs::entity pinkGuyEntity = world.entity("PinkGuy");
    flecs::entity owlGuyEntity = world.entity("OwlGuy");
    flecs::entity floor1Entity = world.entity("Floor");
    flecs::entity floor2Entity = world.entity("Floor2");


    /**
     * SDL SETUP
     * 
     */
    SDL_Init(SDL_INIT_VIDEO);
    gWindow = SDL_CreateWindow("title",0,0, 640,480, SDL_WINDOW_SHOWN);
    gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
    SDL_Event event;

    /**
     * SETUP BACKGROUND
     * 
     */
    SDL_Surface *bgSurface = IMG_Load("bg.png");
    SDL_Texture *bgTexture = SDL_CreateTextureFromSurface(gRenderer, bgSurface);
    float parallaxBgScale = 0.25;
    int bg_w = bgSurface->w;
    int bg_h = bgSurface->h;
    Position bgPosition = {640.0f/2,480.0f/2};
    SDL_Rect bgDestRect = {(int)bgPosition.x - bg_w/2,(int) bgPosition.y - bg_h/2, bg_w, bg_h}; 
    SDL_FreeSurface(bgSurface);


    /**
     * ANIMATED SPRITE SETUP
     * 
     */
    pinkGuyEntity.add<AnimatedSprite>();

    const char *filename = "pink-monster-animation-transparent.png";
    const char *animatedSpriteName =  "pink-monster-animation";
    u32 spriteSheetId = createSpriteSheet(filename, 15, 8, animatedSpriteName);
    AnimatedSprite animatedSprite = createAnimatedSprite(spriteSheetId);

    // add animations to animatedSprite
    Animation walkAnimation{};
    {
        walkAnimation.accumulator = 0.0f;
        walkAnimation.arrFrames[0] = 15;
        walkAnimation.arrFrames[1] = 16;
        walkAnimation.arrFrames[2] = 17;
        walkAnimation.arrFrames[3] = 18;
        walkAnimation.arrFrames[4] = 19;
        walkAnimation.arrFrames[5] = 20;
        walkAnimation.numFrames = 6;
        walkAnimation.currentFrame = 0;
        walkAnimation.fps = 12;
        walkAnimation.msPerFrame = 0.0833;
        walkAnimation.isLoop = true;

        const char *walkAnimationName = "walk";
        strncpy(walkAnimation.name, walkAnimationName, 5);
    }


    Animation runAnimation{};
    {
        runAnimation.accumulator = 0.0f;
        runAnimation.arrFrames[0] = 30;
        runAnimation.arrFrames[1] = 31;
        runAnimation.arrFrames[2] = 32;
        runAnimation.arrFrames[3] = 33;
        runAnimation.arrFrames[4] = 34;
        runAnimation.arrFrames[5] = 35;
        runAnimation.numFrames = 6;
        runAnimation.currentFrame = 0;
        runAnimation.fps = 12;
        runAnimation.msPerFrame = 0.0833;
        runAnimation.isLoop = true;

        const char *runAnimationName = "run";
        strncpy(runAnimation.name, runAnimationName, 4);
    }
    

    Animation standingAttackAnimation{};
    {
        standingAttackAnimation.accumulator = 0.0f;
        standingAttackAnimation.arrFrames[0] = 45;
        standingAttackAnimation.arrFrames[1] = 46;
        standingAttackAnimation.arrFrames[2] = 47;
        standingAttackAnimation.arrFrames[3] = 48;
        standingAttackAnimation.numFrames = 4;
        standingAttackAnimation.currentFrame = 0;
        standingAttackAnimation.fps = 12;
        standingAttackAnimation.msPerFrame = 0.0833;
        standingAttackAnimation.isLoop = false;
        const char *standingAttackAnimationName = "stand-attack";
        strncpy(standingAttackAnimation.name, standingAttackAnimationName, 16);
    }

    Animation idleAnimation{};
    {
        idleAnimation.accumulator = 0.0f;
        idleAnimation.arrFrames[0] = 75;
        idleAnimation.arrFrames[1] = 76;
        idleAnimation.arrFrames[2] = 77;
        idleAnimation.arrFrames[3] = 78;
        idleAnimation.arrFrames[4] = 77;
        idleAnimation.arrFrames[5] = 76;

        idleAnimation.numFrames = 6;
        idleAnimation.currentFrame = 0;
        idleAnimation.fps = 12;
        idleAnimation.msPerFrame = 0.0833;
        idleAnimation.isLoop = true;
        const char *idleAnimationName = "idle";
        strncpy(idleAnimation.name, idleAnimationName, 16);
    }
    
    
    addNewAnimationToAnimatedSprite(&animatedSprite);
    overwriteAnimationOnAnimatedSprite(&animatedSprite, 0, walkAnimation);

    addNewAnimationToAnimatedSprite(&animatedSprite);
    overwriteAnimationOnAnimatedSprite(&animatedSprite, 1, runAnimation);

    addNewAnimationToAnimatedSprite(&animatedSprite);
    overwriteAnimationOnAnimatedSprite(&animatedSprite, 2, standingAttackAnimation);

    addNewAnimationToAnimatedSprite(&animatedSprite);
    overwriteAnimationOnAnimatedSprite(&animatedSprite, 3, idleAnimation);

    animatedSprite.currentAnimation = 0;
    
    // ADD the setup AnimatedSprite to the pinkGuyEntity
    pinkGuyEntity.set<AnimatedSprite>(animatedSprite);
    pinkGuyEntity.set<Position>((Position){640.0f/2.0f,480.0f/2.0f});
    pinkGuyEntity.set<Velocity>((Velocity){0,0});
    // pinkGuyEntity.set<CollisionRect>((CollisionRect){32,32});
    
    InputButtonState buttonStates[3];
    buttonStates[0].currentInputState = INPUT_IS_NOT_PRESSED;
    buttonStates[0].previousInputState = INPUT_IS_NOT_PRESSED;
    buttonStates[0].name = std::string("left");
    buttonStates[0].sdlScancode = SDL_SCANCODE_A;

    buttonStates[1].currentInputState = INPUT_IS_NOT_PRESSED;
    buttonStates[1].previousInputState = INPUT_IS_NOT_PRESSED;
    buttonStates[1].name = "right";
    buttonStates[1].sdlScancode = SDL_SCANCODE_D;

    buttonStates[2].currentInputState = INPUT_IS_NOT_PRESSED;
    buttonStates[2].previousInputState = INPUT_IS_NOT_PRESSED;
    buttonStates[2].name = "jump";
    buttonStates[2].sdlScancode = SDL_SCANCODE_SPACE;

    Input pinkGuyInput;
    pinkGuyInput.buttonStates = buttonStates;
    pinkGuyInput.numButtomStates = 3;

    pinkGuyEntity.set<Input>(pinkGuyInput);

    Dimensions pinkDimensions;
    pinkDimensions.h = 32;
    pinkGuyEntity.set<Dimensions>(pinkDimensions);
     
    std::vector<Ray2d> rays;
    Ray2d ray0, ray1;
    ray0.startingPosition.x = -8.0f;
    ray0.startingPosition.y = 0.0f;
    ray0.distance = 32.0f;
    ray1.startingPosition.x = 8.0f;
    ray1.startingPosition.y = 0.0f;
    ray1.distance = 32.0f;
    rays.push_back(ray0);
    
    rays.push_back(ray1);

    pinkGuyEntity.set<std::vector<Ray2d>>(rays);

    State state = STATE_ON_GROUND;
    pinkGuyEntity.set<State>(state);
    

    // OTHER CHARACTER SETUP

    owlGuyEntity.add<AnimatedSprite>();

    const char *filename2 = "owlet-monster-animation-transparent.png";
    const char *animatedSpriteName2 =  "owl-monster-animation";
    u32 spriteSheetId2 = createSpriteSheet(filename2, 15, 8, animatedSpriteName2);
    AnimatedSprite animatedSprite2 = createAnimatedSprite(spriteSheetId2);

    // add animations to animatedSprite
    Animation walkAnimation2{};
    {
        walkAnimation2.accumulator = 0.0f;
        walkAnimation2.arrFrames[0] = 15;
        walkAnimation2.arrFrames[1] = 16;
        walkAnimation2.arrFrames[2] = 17;
        walkAnimation2.arrFrames[3] = 18;
        walkAnimation2.arrFrames[4] = 19;
        walkAnimation2.arrFrames[5] = 20;
        walkAnimation2.numFrames = 6;
        walkAnimation2.currentFrame = 0;
        walkAnimation2.fps = 12;
        walkAnimation2.msPerFrame = 0.0833;
        walkAnimation2.isLoop = true;
    }


    const char *walkAnimation2Name = "walk";
    strncpy(walkAnimation2.name, walkAnimation2Name, 5);

    

    Animation runAnimation2{};
    {
        runAnimation2.accumulator = 0.0f;
        runAnimation2.arrFrames[0] = 30;
        runAnimation2.arrFrames[1] = 31;
        runAnimation2.arrFrames[2] = 32;
        runAnimation2.arrFrames[3] = 33;
        runAnimation2.arrFrames[4] = 34;
        runAnimation2.arrFrames[5] = 35;
        runAnimation2.numFrames = 6;
        runAnimation2.currentFrame = 0;
        runAnimation2.fps = 12;
        runAnimation2.msPerFrame = 0.0833;
        runAnimation2.isLoop = true;
    }


    const char *runAnimation2Name = "run";
    strncpy(runAnimation2.name, runAnimation2Name, 3);

    Animation standingAttackAnimation2{};
    {
        standingAttackAnimation2.accumulator = 0.0f;
        standingAttackAnimation2.arrFrames[0] = 45;
        standingAttackAnimation2.arrFrames[1] = 46;
        standingAttackAnimation2.arrFrames[2] = 47;
        standingAttackAnimation2.arrFrames[3] = 48;
        standingAttackAnimation2.numFrames = 4;
        standingAttackAnimation2.currentFrame = 0;
        standingAttackAnimation2.fps = 12;
        standingAttackAnimation2.msPerFrame = 0.0833;
        standingAttackAnimation2.isLoop = false;
        const char *standingAttackAnimation2Name = "stand-attack";
        strncpy(standingAttackAnimation2.name, standingAttackAnimation2Name, 16);
    }

    
    addNewAnimationToAnimatedSprite(&animatedSprite2);
    overwriteAnimationOnAnimatedSprite(&animatedSprite2, 0, walkAnimation);

    addNewAnimationToAnimatedSprite(&animatedSprite2);
    overwriteAnimationOnAnimatedSprite(&animatedSprite2, 1, runAnimation);

    addNewAnimationToAnimatedSprite(&animatedSprite2);
    overwriteAnimationOnAnimatedSprite(&animatedSprite2, 2, standingAttackAnimation);

    animatedSprite2.currentAnimation = 0;

    owlGuyEntity.set<AnimatedSprite>(animatedSprite2);
    owlGuyEntity.set<Position>((Position){640.0f/2.0f + 50,480.0f/2.0f});


    
    
    // Set up the animation playing and rendering systems
    world.system<AnimatedSprite>("AnimatedSpritePlay").kind(flecs::OnUpdate).iter(animationsAccumulationSystem);

    world.system<AnimatedSprite, Position>("renderingAnimatedSprites").kind(flecs::OnStore).iter(renderingAnimatedSpritesSystem);

    world.system<AnimatedSprite, KeyboardState>("keyStateAnimationSpriteState").kind(flecs::OnUpdate).iter(KeyboardStateAnimationSetterSystem);

    world.system<Velocity, Input, State>("keyStateVelocitySetter").kind(flecs::PreUpdate).iter(InputVelocitySetterSystem);

    world.system<Position, std::vector<Ray2d>, Velocity, State>("collision").kind(flecs::PostUpdate).iter(ray2dRectangularObjectCollisionSystem);

    world.system<Velocity, Position>("move").kind(flecs::OnUpdate).iter(moveSystem);

    world.system<AnimatedSprite, Input>().kind(flecs::OnUpdate).iter(InputFlipSystem);

    world.system<Input>().kind(flecs::PreUpdate).iter(inputUpdateSystem);

    world.system<Position, std::vector<Ray2d>>().kind(flecs::OnStore).iter(renderRay2dCollectionsSystem);

    world.system<AnimatedSprite, Velocity>().kind(flecs::OnUpdate).iter(setAnimationBasedOnSpeedSystem);

    world.system<Velocity, State>().kind(flecs::OnUpdate).iter(gravitySystem);

    // TEST Rectangular objects

    SDL_Rect floorRect = {0,300,2000,40};
    SDL_Color floorRectColor = {0,0,200};

    SolidRect robj;
    robj.rect = floorRect;
    robj.color = floorRectColor;

    SDL_Rect floorRect2 = {500,290,2000,40};
    SDL_Color floorRectColor2 = {0,0,200};

    SolidRect robj2;
    robj2.rect = floorRect2;
    robj2.color = floorRectColor2;

    world.system<SolidRect>().kind(flecs::OnStore).iter(renderRectangularObjectsSystem);

    floor1Entity.set<SolidRect>(robj); 
    floor2Entity.set<SolidRect>(robj2); 


    // timing
    // float deltaTime = 0.0f;
    const float FPS = 60;
    const float secondsPerFrame = 1.0f / FPS;

    float zoomAmount = 1.0f;
    // main loop
    while(!quit){
        u64 startTicks = SDL_GetTicks();
        // deltaTime = getDeltaTime();

        while(SDL_PollEvent(&event)){
            if(event.type == SDL_QUIT){
                quit = true;\
                break;
            }
        }
        u8 *keyStates = (u8 *)SDL_GetKeyboardState(nullptr);
        KeyboardState keyboardState{};
        keyboardState.keyStates = keyStates;    

        gKeyStates = keyStates;

        if(keyStates[SDL_SCANCODE_UP]){
            zoomAmount += 0.005;
        }       

        if(keyStates[SDL_SCANCODE_DOWN]){
            zoomAmount -= 0.005;
        }       

        if(keyStates[SDL_SCANCODE_R]){
            zoomAmount = 1.0;
        }       


        pinkGuyEntity.set<KeyboardState>(keyboardState);

        SDL_SetRenderDrawColor(gRenderer, 0,0,0,255);
        SDL_RenderClear(gRenderer);

        gCameraPosition.x = pinkGuyEntity.get<Position>()->x;
        gCameraPosition.y = pinkGuyEntity.get<Position>()->y;

        Position centerScreen = {320, 240};
        Position scaledCenterScreen = {centerScreen.x / zoomAmount, centerScreen.y / zoomAmount};
        // background moving with camera
        bgDestRect.x =
                ((int) bgPosition.x - (int) gCameraPosition.x) * (int) parallaxBgScale + (int) scaledCenterScreen.x -
                ((int) bg_w / 2);
        bgDestRect.y =
                ((int) bgPosition.y - (int) gCameraPosition.y + (int) scaledCenterScreen.y - ((int) bg_h / 2) );

        SDL_RenderSetScale(gRenderer, zoomAmount, zoomAmount);

        // draw background
        SDL_RenderCopy(gRenderer, bgTexture, nullptr, &bgDestRect);

        world.progress();


        SDL_RenderPresent(gRenderer);

        u64 endTicks = SDL_GetTicks();

        u64 totalTicks = endTicks - startTicks;
        float totalSeconds = (float)totalTicks / 1000.0f;

        if(totalSeconds < secondsPerFrame){
            float secondsRemainingToFixTimeStep = secondsPerFrame - totalSeconds;
            float msRemainingToFixTimeStep = secondsRemainingToFixTimeStep * 1000;
            SDL_Delay((u32)msRemainingToFixTimeStep);
            // printf("ms to wait: %f\n", msRemainingToFixTimeStep);
        }
    }


    
}