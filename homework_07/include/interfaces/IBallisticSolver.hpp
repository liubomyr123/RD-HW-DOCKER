#pragma once

#include "dto/DroneConfig.hpp"
#include "dto/AmmoParams.hpp"
#include "dto/OutputData.hpp"
#include "dto/SimState.hpp"
#include "interfaces/ITargetProvider.hpp"

// Калькулятор балістики: обчислює точку скиду
class IBallisticSolver
{
public:
    virtual bool solve(
        const DroneConfig* const droneConfig,
        const AmmoParams* const ammoParams,
        const ITargetProvider* const targets,
        SimState& state, 
        OutputData& outputData) = 0;
    virtual ~IBallisticSolver() = default;
};