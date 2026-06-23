#pragma once

#include <vector>
#include "dto/SimStep.hpp"

struct OutputData {
    size_t totalSteps = 0;
    std::vector<SimStep*> steps;
};
