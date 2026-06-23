#pragma once

#include "dto/Coord.hpp"
#include "dto/DroneState.hpp"

struct SimState {
    bool finished = false;

    float totalSimTime = 0.0f;

    Coord dronePosition;
    float droneZ;

    float droneDir;
    float droneVelocity = 0.0f;

    float angleTurnLeft = 0.0f;
    float dropPointDir;

    DroneState droneState = STOPPED;
    int currentTargetIndex = -1;

    int simulation_count = 0;
};