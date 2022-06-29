#include "flecs.h"
#include "velocity.h"
#include "position.h"
#include "ints.h"
#include "input.h"
#include "util.h"
#include <vector>
#include "ray2d.h"
#include "states.h"
#include "state_util.h"
#include <cmath>
#include "solid_rect.h"
#include "collisions.h"


// maybe separate into different functions for the purpose of not checking control lock timer in system
// void movement_GroundSpeed_update_from_Input_and_Physics_System(flecs::iter &it, GroundSpeed *groundSpeeds, Velocity *velocities, Input *inputs, StateCurrPrev *states, Angle *angles){

// }


// void movement_Velocity_apply_GroundSpeed(flecs::iter &it, GroundSpeed *groundSpeeds, Velocity *velocities, StateCurrPrev *states, Angle *angles){

// }

// ======= 
// SYSTEMS
// =======

void movement_apply_velocity_to_position_System(flecs::iter &it, Velocity *velocities, Position *positions){
    for(auto i : it){
        positions[i].x += velocities[i].x ;//* it.delta_time();
        positions[i].y += velocities[i].y ;//* it.delta_time();
    }
}





void movement_GrounSpeed_Velocity_update_from_Input_and_Phyics_System(flecs::iter &it, Velocity *velocities, GroundSpeed *groundSpeeds, Input *inputs, StateCurrPrev *states, Angle *angles){

    const float acc = 0.046875;// * 600;
    const float frc = 0.046875;// * 600;
    const float dec = 0.5;// * 600;
    const float topSpeed = 6;// * 600;
    const float jump = 6.5f;
    const float slp = 0.125f;
    const float slpRollUp = 0.078125f;
    const float slpRollDown = 0.3125f;
    const float fall = 2.5f;

    const float airAcc = 0.09375f;
    
   

    for(auto i : it){
        if(State_util_did_just_enter(states[i], STATE_ON_GROUND)){

            // shallow
            if((angles[i].rads >= degToRads(0.0f) && angles[i].rads <= degToRads(23.0f))
            || (angles[i].rads >= degToRads(339.0f) && angles[i].rads <= degToRads(360.0f))){
                groundSpeeds[i].val = velocities[i].x;
            }
            // half steep
            else if((angles[i].rads > degToRads(23.0f) && angles[i].rads <= degToRads(45.0f))
            || angles[i].rads >= degToRads(316.0f) && angles[i].rads < degToRads(339.0f)){
                float xSpeedAbs = fabs(velocities[i].x);
                float ySpeedAbs = fabs(velocities[i].y);
                if(xSpeedAbs > ySpeedAbs){
                    groundSpeeds[i].val = velocities[i].x;
                }
                else{
                    groundSpeeds[i].val = velocities[i].y * 0.5f * -sign(sin(angles[i].rads));
                }

            }
            // full steep
            else if((angles[i].rads > degToRads(46.0f) && angles[i].rads <= degToRads(90.0f))
            || (angles[i].rads >= degToRads(271.0f) && angles[i].rads < degToRads(316.0f))){
                float xSpeedAbs = fabs(velocities[i].x);
                float ySpeedAbs = fabs(velocities[i].y);
                if(xSpeedAbs > ySpeedAbs){
                    groundSpeeds[i].val = velocities[i].x;
                }
                else{
                    groundSpeeds[i].val = velocities[i].y * -sign(sin(angles[i].rads));
                }
            }
        }

        if(states[i].currentState == STATE_ON_GROUND){
            groundSpeeds[i].val -= slp * sin(angles[i].rads);

            // side
            if(Input_is_pressed(inputs[i], "left") && !it.entity(i).has<ControlLockTimer>()){
                if(groundSpeeds[i].val > 0){ // moving right
                    groundSpeeds[i].val -= dec;
                }
                else { // moving left
                    groundSpeeds[i].val -= acc;
                    groundSpeeds[i].val = MAX(groundSpeeds[i].val, -topSpeed);
                }
            }
            else if(Input_is_pressed(inputs[i], "right") && !it.entity(i).has<ControlLockTimer>()){
                if(groundSpeeds[i].val < 0){ // moving left
                    groundSpeeds[i].val += dec;
                }
                else { //moving right
                    groundSpeeds[i].val += acc;
                    groundSpeeds[i].val = MIN(groundSpeeds[i].val, topSpeed);
                }
            }
            else {
                if(groundSpeeds[i].val == 0){

                }
                else if(groundSpeeds[i].val > 0){
                    groundSpeeds[i].val -= frc;
                    if(groundSpeeds[i].val < 0){
                        groundSpeeds[i].val = 0;
                    }
                }
                else if(groundSpeeds[i].val < 0){
                    groundSpeeds[i].val += frc;
                    if(groundSpeeds[i].val > 0){
                        groundSpeeds[i].val = 0;
                    }
                }
                else {
                    //groundSpeeds[i].val = 0;
                }
            }

            velocities[i].x = groundSpeeds[i].val * cos(angles[i].rads);
            velocities[i].y = groundSpeeds[i].val * -sin(angles[i].rads);


            if(Input_is_just_pressed(inputs[i], "jump")){
                State_util_set(states[i], STATE_IN_AIR);
                velocities[i].x -= jump * sin(angles[i].rads);
                velocities[i].y -= jump * cos(angles[i].rads);
            }
        }
        else if(states[i].currentState == STATE_IN_AIR){
            if(!Input_is_pressed(inputs[i], "jump")){
                if(velocities[i].y < -4.0f){
                    velocities[i].y = -4.0f;

                }
            }
            // side
            if(Input_is_pressed(inputs[i], "left")){
                velocities[i].x -= airAcc;
                velocities[i].x = MAX(velocities[i].x, -topSpeed);
            }
            else if(Input_is_pressed(inputs[i], "right")){
                velocities[i].x += airAcc;
                velocities[i].x = MIN(velocities[i].x, topSpeed);
            }
        }
       
    }
}


void movement_velocity_apply_gravity_System(flecs::iter &it, Velocity *velocities, StateCurrPrev *states){
    const float grv = 0.21875f;
    for(int i : it){

        if(states[i].currentState == STATE_IN_AIR){
            if(velocities[i].y < 0.0f && velocities[i].y > -4.0f){
                velocities[i].x -= ((velocities[i].x / 0.125f) / 256);
            }

            velocities[i].y += grv;
            if (velocities[i].y > 16.0f){
                velocities[i].y = 16.0f;
            } 
        }
    }
}



void movement_ControlLockTimer_update_when_on_ground(flecs::iter &it, ControlLockTimer *controlLockTimers, StateCurrPrev *states){
    for(u64 i : it){
        if(states[i].currentState != STATE_ON_GROUND){
            continue;
        }

        controlLockTimers[i].accumulator += it.delta_time();
        if(controlLockTimers[i].accumulator >= controlLockTimers[i].endTicks){

            controlLockTimers[i].endTicks = 0.0f;
            controlLockTimers[i].accumulator = 0.0f;

            flecs::world world = it.world();
            
            world.defer_begin();

            it.entity(i).remove<ControlLockTimer>();
            
            world.defer_end();

        }
    }
}

void movement_GroundSpeed_zero_ControlLockTimer_add_based_on_too_steep_angle_System(flecs::iter &it, GroundSpeed *groundSpeeds, Angle *angles, StateCurrPrev *states){
    const float slip_range_angle_degrees_start = 46.0f;
    const float slip_range_angle_degrees_end = 315.0f;

    for(u64 i : it){
        float angle_degrees = util_rads_to_degrees(angles[i].rads);

        if(util_in_range(angle_degrees, slip_range_angle_degrees_start, slip_range_angle_degrees_end)){
            if(fabs(groundSpeeds[i].val) < 2.5f){
                State_util_set(states[i], STATE_IN_AIR);
                groundSpeeds[i].val = 0.0f;

                flecs::world world = it.world();

                world.defer_begin();

                it.entity(i).set<ControlLockTimer>(
                    (ControlLockTimer){
                        0.5f, 0.0f
                    }
                );

                world.defer_end();
            }
        }
    }
}