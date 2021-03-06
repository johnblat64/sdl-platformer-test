#include <SDL2/SDL.h>
#include "flecs.h"
#include "position.h"
#include "input.h"
#include "render.h"
#include "debug_render.h"
#include "camera.h"
#define V2D_IMPLEMENTATION
#include "v2d.h"
#include "resourceLoading.h"
#include "mouseState.h"
#include "mouseStateProcessing.h"
#include "eventHandling.h"
#include "timestep.h"
#include "loadSave.h"
#include "editingFunctionality.h"

SDL_Renderer *gRenderer;
SDL_Window *gWindow;


int gScreenWidth = 640 * 2;
int gScreenHeight = 480 * 2;

// main loop flag
bool quit = false;


/**
 * @brief MOUSE RELATED STUFF
 * 
 */


MouseState mouseState;






/**
 * @brief END MOUSE RELATED STUFF
 * 
 */


/**
 * @brief EVENT PROCESSNIG
 * 
 */




void quitEventProcessor(SDL_Event &event){
    if(event.type == SDL_QUIT){
        quit = true;
    }
}


void mouseWheelStateEventProcessor(SDL_Event &event){
    if(event.type == SDL_MOUSEWHEEL){
        if(event.wheel.y > 0){
            mouseState.currentMouseWheelState = SCROLL_UP;
        }
        else if(event.wheel.y < 0){
            mouseState.currentMouseWheelState = SCROLL_DOWN;
        }
    }
}

/**
 * @brief END EVENT PROCESSNIG
 * 
 */


/** PLATFORM RELATED STUFF
 * 
 */



/** END PLATFORM RELATED STUFF
 * 
 */






/**
 * @brief Registers the initial systems
 * 
 * @param ecs 
 */
void registerSystems(flecs::world &ecs){
    ecs.system<Input>()
        .kind(flecs::PreUpdate)
        .iter(Input_update_input_button_states_System);
    
    ecs.system<MouseState>()
        .kind(flecs::PreUpdate)
        .iter(MouseState_update_System);

    ecs.system<Input>()
        .kind(flecs::OnUpdate)
        .iter(save_PlatformPaths_on_save_button_release_System);
    
    ecs.system<Input>()
        .kind(flecs::OnUpdate)
        .iter(load_PlatformPaths_on_load_button_release_System);
    
    ecs.system<Position, PlatformPath>()
        .kind(flecs::OnStore)
        .iter(renderSelectedPlatformNodeSystem);
    
    ecs.system<Input>()
        .kind(flecs::OnUpdate)
        .iter(cam_input_zoom_System);
    
    ecs.system<Input, MouseState>()
        .kind(flecs::OnUpdate)
        .iter(AddVertexAtMousePositionOnSelectedPlatformPathSystem);
    
    ecs.system<>()
        .kind(flecs::OnUpdate)
        .iter(cam_zoom_render_frame_start_System);
    
    ecs.system<Input>()
        .kind(flecs::OnUpdate)
        .iter(cam_input_camera_move_System);
    
    ecs.system<>()
        .kind(flecs::PreFrame)
        .iter(render_frame_start_System);
    
    ecs.system<>()
        .kind(flecs::PostFrame)
        .iter(render_end_frame_System);
    
}


void mouseScrollWheelZoomSetter(MouseState mouseState);
void mouseScrollWheelZoomSetter(MouseState mouseState){
    if(mouseState.currentMouseWheelState == SCROLL_UP){
        gZoomAmount += 0.05;
    }
    else if(mouseState.currentMouseWheelState == SCROLL_DOWN){
        gZoomAmount -= 0.05;
    }
}





int main(){
    gZoomAmount = 1.0f;

    Input userInput;

    userInput.inputStates.push_back((InputMappingState){
        std::string("save"),
        SDL_SCANCODE_S,
        INPUT_IS_NOT_PRESSED,
        INPUT_IS_NOT_PRESSED
    });

    userInput.inputStates.push_back((InputMappingState){
        std::string("load"),
        SDL_SCANCODE_L,
        INPUT_IS_NOT_PRESSED,
        INPUT_IS_NOT_PRESSED
    });

    userInput.inputStates.push_back((InputMappingState){
        std::string("zoom-in"),
        SDL_SCANCODE_UP,
        INPUT_IS_NOT_PRESSED,
        INPUT_IS_NOT_PRESSED
    });

    userInput.inputStates.push_back((InputMappingState){
        std::string("zoom-out"),
        SDL_SCANCODE_DOWN,
        INPUT_IS_NOT_PRESSED,
        INPUT_IS_NOT_PRESSED
    });

    userInput.inputStates.push_back((InputMappingState){
        std::string("zoom-reset"),
        SDL_SCANCODE_R,
        INPUT_IS_NOT_PRESSED,
        INPUT_IS_NOT_PRESSED
    });

    userInput.inputStates.push_back((InputMappingState){
        std::string("camera-pan-up"),
        SDL_SCANCODE_W,
        INPUT_IS_NOT_PRESSED,
        INPUT_IS_NOT_PRESSED
    });

    userInput.inputStates.push_back((InputMappingState){
        std::string("camera-pan-down"),
        SDL_SCANCODE_S,
        INPUT_IS_NOT_PRESSED,
        INPUT_IS_NOT_PRESSED
    });

    userInput.inputStates.push_back((InputMappingState){
        std::string("camera-pan-left"),
        SDL_SCANCODE_A,
        INPUT_IS_NOT_PRESSED,
        INPUT_IS_NOT_PRESSED
    });

    userInput.inputStates.push_back((InputMappingState){
        std::string("camera-pan-right"),
        SDL_SCANCODE_D,
        INPUT_IS_NOT_PRESSED,
        INPUT_IS_NOT_PRESSED
    });

    gCameraPosition.x = gScreenWidth/2;
    gCameraPosition.y = gScreenHeight/2;

    // sdl
    SDL_Init(SDL_INIT_VIDEO);
    gWindow = SDL_CreateWindow(
        "title",
        0,
        0, 
        gScreenWidth,
        gScreenHeight, 
        SDL_WINDOW_SHOWN
    );
    gRenderer = SDL_CreateRenderer(
        gWindow, 
        -1, 
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC 
    );

    mouseState.lmbCurrentState = INPUT_IS_NOT_PRESSED;
    mouseState.rmbCurrentState = INPUT_IS_NOT_PRESSED;

    SDL_Event event;
    registerEventProcessor(quitEventProcessor);
    registerEventProcessor(mouseWheelStateEventProcessor);
    

    // flecs
    flecs::world ecs;

    registerSystems(ecs);

    flecs::entity editorUser = ecs.entity();
    editorUser.set<Input>(userInput);
    editorUser.set<MouseState>(mouseState);


    gTimeStep = ts_TimeStep_init(60.0f);

    while(!quit){

        ts_TimeStep_start_ticks_set_to_current_ticks(gTimeStep);

        mouseState.currentMouseWheelState = NOT_SCROLLING;

        while(SDL_PollEvent(&event)){
            runRegisteredEventProcessors(event);
        }
        
        gKeyStates = (u8 *)SDL_GetKeyboardState(NULL);

        mouseScrollWheelZoomSetter(mouseState);
        
        ecs.progress();

    }
}