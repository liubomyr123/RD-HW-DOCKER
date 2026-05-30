#include "../include/ballistics.hpp"

#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <unordered_map>

std::unordered_map<std::string, AmmoInfo> ammo_types_info = {
    {"VOG-17",      {0.35f, 0.07f, 0.0f, true}},
    {"M67",         {0.6f,  0.1f,  0.0f, true}},
    {"RKG-3",       {1.2f,  0.1f,  0.0f, true}},
    {"GLIDING-VOG", {0.45f, 0.1f,  1.0f, false}},
    {"GLIDING-RKG", {1.4f,  0.1f,  1.0f, false}},
};

bool getInputData(const std::string& file_name, InputData& inputData)
{
    LOG_PROCESS("Reading " + file_name + "...");

    std::ifstream file(file_name);
    if (!file) {
        LOG_ERROR("Error opening file");
        return false;
    }
    
    // Example: "100 100 100 200 200 10 10 VOG-17"
    file >> inputData.xd >> inputData.yd >> inputData.zd 
        >> inputData.targetX >> inputData.targetY 
        >> inputData.attackSpeed 
        >> inputData.accelerationPath 
        >> inputData.ammo_name;

    if (file.fail()) {
        LOG_ERROR("Invalid format: wrong data types");
        return false;
    }
    else
    {
        LOG_SUCCESS("Successfully found all params.");
    }

    LOG_INFO("📄 Result:");
    LOG_INFO("  - xd: " << inputData.xd);
    LOG_INFO("  - yd: " << inputData.yd);
    LOG_INFO("  - zd: " << inputData.zd);
    LOG_INFO("  - targetX: " << inputData.targetX);
    LOG_INFO("  - targetY: " << inputData.targetY);
    LOG_INFO("  - attackSpeed: " << inputData.attackSpeed);
    LOG_INFO("  - accelerationPath: " << inputData.accelerationPath);
    LOG_INFO("  - ammo_name: " << inputData.ammo_name);

    std::string extra;
    if (file >> extra) {
        LOG_WARN("Found extra data: " + extra + " (ignored)");
    }

    return true;
}

bool getAmmoInfoByType(const std::string ammo_name, AmmoInfo& outAmmo)
{
    LOG_PROCESS("Searching ammo info for " << ammo_name << "...");

    auto it = ammo_types_info.find(ammo_name);
    
    if (it == ammo_types_info.end()) {
        LOG_WARN("Unknown ammo type: " << ammo_name);
        return false;
    }

    LOG_SUCCESS("Successfully found ammo type.");
    outAmmo = it->second;

    LOG_INFO("📄 Result:");
    LOG_INFO("  - m: " << outAmmo.m);
    LOG_INFO("  - d: " << outAmmo.d);
    LOG_INFO("  - l: " << outAmmo.l);
    LOG_INFO("  - isFreeFall: " << (outAmmo.isFreeFall ? "true" : "false"));

    return true;
}

bool getAmmoTimeOfFlight(float& result, const InputData& inputData, const AmmoInfo& outAmmo)
{
    LOG_PROCESS("Calculating time of fly...");

    float d = outAmmo.d;
    float m = outAmmo.m;
    float l = outAmmo.l;
    float v0 = inputData.attackSpeed;
    float z0 = inputData.zd;

    // a · t³ + b · t² + c = 0
    // a = d·g·m − 2d²·l·V₀
    // b = −3g·m² + 3d·l·m·V₀
    // c = 6m²·Z₀
    // V₀ — швидкість атаки дрона, Z₀ — висота дрона (zd), g = 9.81 м/с².
    float a = (d * GRAVITATIONAL_ACCELERATION * m) - 2 * d * d * l * v0;
    if (std::abs(a) < 1e-6f)
    {
        LOG_ERROR("Invalid a: we cannot divide by zero");
        return false;
    }
    float b = (-1) * 3 * GRAVITATIONAL_ACCELERATION * m * m + 3 * d * l * m * v0;
    float c = 6 * m * m * z0;

    // p = − b² / (3a²)
    // q = 2b³ / (27a³) + c / a
    // φ = arccos( 3q / (2p) · √(−3/p) )
    float p = (-1) * b * b / (3 * a * a);
    if (p >= 0.0f)
    {
        LOG_ERROR("Invalid p: must be negative for Cardano trig solution");
        return false;
    }
    float q = 2 * b * b * b / (27 * a * a * a) + c / a;

    float arg = (3 * q) / (2 * p) * std::sqrt(-3 / p);
    if (arg < -1.0f || arg > 1.0f)
    {
        LOG_ERROR("Invalid acos argument: out of range");
        return false;
    }
    float f = std::acos(arg);

    // t = 2√(−p/3) · cos( (φ + 4π) / 3 ) − b / (3a)
    float t = 2 * std::sqrt((p * (-1)) / 3) * std::cos((f + 4 * M_PI) / 3) - b / (3 * a);
    LOG_SUCCESS("Successfully found time of flight");

    LOG_INFO("📄 Result:");
    LOG_INFO("  - t: " << t);

    result = t;
    return true;
}

bool getHorizontalFlightRange(float& result, const InputData& inputData, const AmmoInfo& outAmmo, const float& ammoTimeOfFlight)
{
    LOG_PROCESS("Calculating horizontal flight range...");

    // h = V₀t 
    //      − t²d·V₀/(2m)
    //      + t³(6d·g·l·m − 6d²(l²-1)·V₀)/(36m²)
    //      + t⁴(
    //              −6d²g·l·(1+l²+l⁴)m
    //              + 3d³l²(1+l²)V₀
    //              + 6d³l⁴(1+l²)V₀
    //          ) / (36(1+l²)²m³)
    //      + t⁵(
    //              3d³g·l³m
    //              − 3d⁴l²(1+l²)V₀
    //          ) / (36(1+l²)m⁴)

    // V₀ — швидкість атаки дрона, Z₀ — висота дрона (zd), g = 9.81 м/с²
    // t - час польоту снаряду (Time of Flight)

    float t = ammoTimeOfFlight;
    float d = outAmmo.d;
    float m = outAmmo.m;
    if (std::abs(m) < 1e-6f)
    {
        LOG_ERROR("Invalid m: we cannot divide by zero");
        return false;
    }
    float l = outAmmo.l;
    float v0 = inputData.attackSpeed;

    float t2 = t * t;
    float t3 = t2 * t;
    float t4 = t2 * t2;
    float t5 = t4 * t;

    float d2 = d * d;
    float d3 = d2 * d;
    float d4 = d2 * d2;

    float m2 = m * m;
    float m3 = m2 * m;
    float m4 = m2 * m2;

    float l2 = l * l;
    float l3 = l2 * l;
    float l4 = l2 * l2;

    // V₀t
    float step0 = v0 * t;

    // − t²d·V₀/(2m)
    float step1 = ((-1) * t2 * d * v0) / (2 * m);

    // + t³(6d·g·l·m − 6d²(l²-1)·V₀)/(36m²)
    float step2 = t3 * ((6 * d * GRAVITATIONAL_ACCELERATION * l * m) - (6 * d2 * (l2 - 1) * v0)) / (36 * m2);

    // −6d²g·l·(1+l²+l⁴)m
    float step3_0 = (-1) * 6 * d2 * GRAVITATIONAL_ACCELERATION * l * (1 + l2 + l4) * m;
    // + 3d³l²(1+l²)V₀
    float step3_1 = 3 * d3 * l2 * (1 + l2) * v0;
    // + 6d³l⁴(1+l²)V₀
    float step3_2 = 6 * d3 * l4 * (1 + l2) * v0;
    // 36(1+l²)²m³
    float step3_3 = 36 * (1 + l2) * (1 + l2) * m3;

    float step3 = t4 * (step3_0 + step3_1 + step3_2) / step3_3;

    // 3d³g·l³m
    float step4_0 = 3 * d3 * GRAVITATIONAL_ACCELERATION * l3 * m;
    // − 3d⁴l²(1+l²)V₀
    float step4_1 = (-1) * 3 * d4 * l2 * (1 + l2)* v0;
    // 36(1+l²)m⁴
    float step4_2 = 36 * (1 + l2) * m4;

    float step4 = t5 * (step4_0 + step4_1) / step4_2;

    float h = step0 + step1 + step2 + step3 + step4;

    LOG_SUCCESS("Successfully calculated horizontal flight range");

    LOG_INFO("📄 Result:");
    LOG_INFO("  - h: " << h);

    result = h;
    return true;
}

bool getDistanceToTarget(float& result, const InputData& inputData)
{
    LOG_PROCESS("Calculating horizontal distance from copter to target...");

    // D = √( (targetX − xd)² + (targetY − yd)² )
    // targetX, targetY - координати цілі
    // xd, yd - координати дрона

    float xDiff = inputData.targetX - inputData.xd;
    float yDiff = inputData.targetY - inputData.yd;

    float step0 = xDiff * xDiff;
    float step1 = yDiff * yDiff;

    float D = std::sqrt(step0 + step1);

    LOG_SUCCESS("Successfully calculated distance from drone to target");

    LOG_INFO("📄 Result:");
    LOG_INFO("  - D: " << D);

    result = D;
    return true;
}

bool isManeuverRequired(const float& h, const float& accelerationPath, const float& D)
{
    bool result = (h + accelerationPath) > D;
    if (result)
    {
        LOG_WARN("Maneuver required: drone is too close to the target.");
    }
    else
    {
        LOG_SUCCESS("No maneuver required: drone is at correct release distance.");
    }
    return result;
}

bool getNewDroneCoordinatesForManeuver(float& newX, float& newY, const InputData& inputData, const float& D, const float& h)
{
    LOG_PROCESS("Calculating new drone coordinates for maneuver...");    

    // xd' = targetX − (targetX − xd) · (h + accelerationPath) / D
    // yd' = targetY − (targetY − yd) · (h + accelerationPath) / D

    if (std::abs(D) < 1e-6f)
    {
        LOG_ERROR("Invalid D: we cannot divide by zero");
        return false;
    }

    float step0 = (h + inputData.accelerationPath) / D;

    newX = inputData.targetX - (inputData.targetX - inputData.xd) * step0;
    newY = inputData.targetY - (inputData.targetY - inputData.yd) * step0;

    LOG_SUCCESS("Successfully calculated new drone coordinates for maneuver.");

    LOG_INFO("📄 Result:");
    LOG_INFO("  - xd': " << newX);
    LOG_INFO("  - yd': " << newY);

    return true;
}

bool getAmmoDropPoint(OutputData& outputData, const InputData& inputData, const AmmoInfo& outAmmo, const float& D, const float& h)
{
    LOG_PROCESS("Calculating ammo drop point...");    

    // ratio = (D − h) / D
    // fireX = xd + (targetX − xd) · ratio
    // fireY = yd + (targetY − yd) · ratio

    if (std::abs(D) < 1e-6f)
    {
        LOG_ERROR("Invalid D: we cannot divide by zero");
        return false;
    }

    float newXd = inputData.xd;
    float newYd = inputData.yd;
    float newH = h;
    float newD = D;
    auto newInputData = inputData;
    if (isManeuverRequired(h, inputData.accelerationPath, D))
    {
        if (!getNewDroneCoordinatesForManeuver(newXd, newYd, inputData, D, h)) 
        {
            return false;
        }

        outputData.postManeuverX = newXd;
        outputData.postManeuverY = newYd;
        outputData.isRecalculated = true;

        newInputData.xd = newXd;
        newInputData.yd = newYd;

        if (!getDistanceToTarget(newD, newInputData)) 
        {
            return false;
        }
        if (std::abs(newD) < 1e-6f)
        {
            LOG_ERROR("Invalid D: we cannot divide by zero");
            return false;
        }

        float ammoTimeOfFlight = 0.0f;
        if (!getAmmoTimeOfFlight(ammoTimeOfFlight, newInputData, outAmmo)) 
        {
            return false;
        }

        if (!getHorizontalFlightRange(newH, newInputData, outAmmo, ammoTimeOfFlight)) 
        {
            return false;
        }
    }

    float ratio = (newD - newH) / newD;

    outputData.fireX = newInputData.xd + (newInputData.targetX - newInputData.xd) * ratio;
    outputData.fireY = newInputData.yd + (newInputData.targetY - newInputData.yd) * ratio;

    LOG_SUCCESS("Successfully calculated ammo drop point.");

    LOG_INFO("📄 Result:");
    LOG_INFO("  - fireX: " << outputData.fireX);
    LOG_INFO("  - fireY: " << outputData.fireY);

    return true;
}

bool writeOutputToFile(const std::string& file_name, const OutputData& outputData)
{
    LOG_PROCESS("Writing result to " << file_name << "...");

    std::ofstream file(file_name);
    if (!file)
    {
        LOG_ERROR("Error opening file");
        return false;
    }

    if (outputData.isRecalculated)
    {
        file << outputData.postManeuverX << " " << outputData.postManeuverY << " ";
    }
    file << outputData.fireX << " " << outputData.fireY;

    if (file.fail())
    {
        LOG_ERROR("Failed while writing to file");
        return false;
    }

    file.close();

    LOG_SUCCESS("Successfully wrote result to file");
    return true;
}
