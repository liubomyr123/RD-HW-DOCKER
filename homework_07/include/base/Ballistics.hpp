#pragma once

#include "dto/AmmoParams.hpp"
#include "dto/Coord.hpp"
#include "dto/DroneConfig.hpp"
#include "dto/OutputData.hpp"
#include "dto/SimState.hpp"
#include "dto/TargetsData.hpp"

bool getJSONDroneConfig(DroneConfig* droneConfig);
bool getTargetsJSONData(TargetsData& targetsData);
bool getJSONAmmoParamByType(const std::string& ammo_name, AmmoParams* ammoParam);
bool getAmmoTimeOfFlight(float& result, 
    const DroneConfig * const droneConfig,
    const AmmoParams * const ammoParam);
bool getHorizontalFlightRange(float& result, 
    const DroneConfig * const droneConfig,
    const AmmoParams * const ammoParam,
    const float& ammoTimeOfFlight);
bool getDistanceToTarget(float& result, const Coord& drone, const Coord& target);
bool getAmmoDropPoint(Coord& result, 
    const Coord& drone,
    const float& D, 
    const float& h, 
    const Coord& target);
bool interpolate(Coord& result, 
    const int& timeSteps,
    const float& timeFrame,
    const DroneConfig * const droneConfig,
    const Coord* const target);
bool getTargetVelocity(Coord& result, 
    const int& timeSteps,
    const SimState& state, 
    const DroneConfig * const droneConfig,
    const Coord* const target);
bool getPredictedPosition(Coord& result, 
    const Coord& targetVelocity,
    const Coord& targetPosition,
    const float& totalTime);
bool getTimeToTarget(float& result, 
    const float& distanceToTarget, 
    const float& speed);
bool getNormalizedAngle(float& result);
bool getAcceleration(float& result, const DroneConfig * const droneConfig);
bool writeOutputToFile(const OutputData& outputData);
