#pragma once

#include "dto/SimStep.hpp"

struct OutputData {
    size_t totalSteps = 0;
    SimStep** steps = nullptr;
};
