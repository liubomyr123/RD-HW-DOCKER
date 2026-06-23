#pragma once

#include "dto/Target.hpp"
#include <vector>

struct TargetsData {
    int targetCount = 0;
    int timeSteps = 0;
    std::vector<Target*> targets;
};
