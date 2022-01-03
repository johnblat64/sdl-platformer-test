#ifndef animproch
#define animproch

#include "animation.h"
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "position.h"
#include "flecs.h"

extern Position gCameraPosition;
/**
 * @brief runs all the animations provided. Will advance frames as per the fps specified
 * 
 * @param animations 
 * @param numAnimations 
 * @param deltaTime 
 */
// void animationsAccumulationSystem(AnimatedSprite *animatedSprites, size_t numAnimations, float deltaTime);
void animationsAccumulationSystem(flecs::iter &it, AnimatedSprite *animatedSprites);


/**
 * @brief Create a Animated Sprite object with the spriteSheetId
 * 
 * @param spriteSheetId the id of the sprite sheet this animatedSprite will reference
 * @return AnimatedSprite 
 */
AnimatedSprite createAnimatedSprite(u32 spriteSheetId);

/**
 * @brief creates a blank animation to the animated Sprite's animation list
 * 
 * @param animatedSprite 
 */
void addNewAnimationToAnimatedSprite(AnimatedSprite *animatedSprite);

/**
 * @brief Overwrites an animation in the animation list at animationIndex within the animatedsprite
 * 
 * @param animatedSprite the Animated sprite to modify
 * @param animationIndex the index in the animated sprite's animation list to overwrite
 * @param Animation the animation that will overwrite the animation at the animation index
 */
void overwriteAnimationOnAnimatedSprite(AnimatedSprite *animatedSprite, u32 animationIndex, Animation animation);


/**
 * @brief Will actually set the ms per frame as well. Will use the fps provided to calculate this along with the number of frames in the animatedSprite. Storing the fps so that user can see this when modifying the value.
 * 
 * @param animatedSprite The animated sprite to calculate the ms per frame
 * @param fps frames per second
 */
void setFpsForAnimatedSprite(AnimatedSprite *animatedSprite, float fps);

/**
 * @brief renders an animated sprite's current frame at the posisions specified
 * 
 * @param animatedSprites list of animated sprites
 * @param positions list of posisitions for all the animated sprites
 * @param numEntities number of iterations over the above arrays
 */
void renderingAnimatedSpritesSystem(flecs::iter &it, AnimatedSprite *animatedSprites, Position *positions);

/**
 * @brief just sets the currentFrame to 0. This can be very helpful for non looping animatins
 * 
 * @param animation 
 */
void restartAnimation(Animation *animation);

/**
 * @brief has the number of frames in animation been reached?
 * 
 */
bool isAnimationFinished(Animation *animation);

#endif