#pragma once

#include "dto/Coord.hpp"

struct TargetsData {
    int targetCount = 0;
    int timeSteps = 0;
    Coord** targets = nullptr;
};
