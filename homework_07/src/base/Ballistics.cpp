#include "base/Ballistics.hpp"
#include "Common.hpp"
#include "base/Logger.hpp"
#include "dto/Nlohmann.hpp"

#include <cmath>
#include <cstdlib>
#include <fstream>
#include <string>

bool getJSONDroneConfig(DroneConfig* droneConfig)
{
    LOG_PROCESS("Reading " << CONFIG_JSON_FILE_NAME << "...");
    try
    {
        if (droneConfig == nullptr)
        {
            LOG_ERROR("Pointer is null");
            return false;
        }
        std::ifstream file(CONFIG_JSON_FILE_NAME);
        if (!file) {
            LOG_ERROR("Error opening file");
            return false;
        }

        json jParseData = json::parse(file);
        config::Data jData = jParseData.get<config::Data>();

        /**
         * {
         *   "drone": {
         *      "position": { "x": 0.0, "y": 0.0 },
         *      "altitude": 100.0,
         *      "initialDirection": 0.0,
         *      "attackSpeed": 15.0,
         *      "accelerationPath": 50.0,
         *      "angularSpeed": 0.5,
         *      "turnThreshold": 0.05
         *   },
         *   "ammo": "VOG-17",
         *   "simulation": {
         *      "timeStep": 0.1,
         *      "hitRadius": 2.0
         *   },
         *   "targetArrayTimeStep": 1.0
         * }
         */
        droneConfig->startPos = {jData.drone.position.x, jData.drone.position.y};
        droneConfig->altitude = jData.drone.altitude;
        droneConfig->initialDir = jData.drone.initialDirection;
        droneConfig->attackSpeed = jData.drone.attackSpeed;
        droneConfig->accelPath = jData.drone.accelerationPath;
        droneConfig->ammoName = jData.ammo;
        droneConfig->arrayTimeStep = jData.targetArrayTimeStep;
        droneConfig->simTimeStep = jData.simulation.timeStep;
        droneConfig->hitRadius = jData.simulation.hitRadius;
        droneConfig->angularSpeed = jData.drone.angularSpeed;
        droneConfig->turnThreshold = jData.drone.turnThreshold;

        LOG_INFO("📄 Result:");
        LOG_INFO("  - xd: " << droneConfig->startPos.x);
        LOG_INFO("  - yd: " << droneConfig->startPos.y);
        LOG_INFO("  - zd: " << droneConfig->altitude);
        LOG_INFO("  - initialDir: " << droneConfig->initialDir);
        LOG_INFO("  - attackSpeed: " << droneConfig->attackSpeed);
        LOG_INFO("  - accelerationPath: " << droneConfig->accelPath);
        LOG_INFO("  - ammo_name: " << droneConfig->ammoName);
        LOG_INFO("  - arrayTimeStep: " << droneConfig->arrayTimeStep);
        LOG_INFO("  - simTimeStep: " << droneConfig->simTimeStep);
        LOG_INFO("  - hitRadius: " << droneConfig->hitRadius);
        LOG_INFO("  - angularSpeed: " << droneConfig->angularSpeed);
        LOG_INFO("  - turnThreshold: " << droneConfig->turnThreshold);
    }
    catch (const std::exception& e)
    {
        LOG_ERROR(e.what());
        return false;
    }

    // LOG_SUCCESS("Successfully found all params.");

    return true;
}

bool getTargetsJSONData(TargetsData& targetsData)
{
    LOG_PROCESS("Reading " << TARGETS_JSON_FILE_NAME << "...");
    try
    {
        std::ifstream file(TARGETS_JSON_FILE_NAME);
        if (!file) {
            LOG_ERROR("Error opening file");
            return false;
        }

        json jParseData = json::parse(file);
        targets::Data jData = jParseData.get<targets::Data>();

        /**
         * {
         *   "targetCount": 5,
         *   "timeSteps": 60,
         *   "targets": [
         *     {
         *       "positions": [
         *         { "x": 300.0, "y": 200.0 },
         *         { "x": 301.2, "y": 200.8 },
         *         { "x": 302.4, "y": 201.6 },
         *         "... ще 57 записів ..."
         *       ]
         *     },
         *     {
         *       "positions": [
         *         { "x": 528.3, "y": 121.2 },
         *         ...
         *       ]
         *     },
         *     ... ще 3 цілі ...
         *   ]
         * }
         */
        targetsData.targetCount = jData.targetCount;
        targetsData.timeSteps = jData.timeSteps;
        targetsData.targets = new Coord*[jData.targetCount];
        for (int i = 0; i < jData.targetCount; i++)
        {
            targetsData.targets[i] = new Coord[jData.timeSteps];
            const auto targetPositions = jData.targets[i].positions;
            for (int j = 0; j < jData.timeSteps; j++)
            {
                targetsData.targets[i][j].x = targetPositions[j].x;
                targetsData.targets[i][j].y = targetPositions[j].y;
            }
        }
    }
    catch (const std::exception& e)
    {
        LOG_ERROR(e.what());
        return false;
    }

    LOG_SUCCESS("Successfully read all data for " << 5 << " targets.");

    return true;
}

bool getJSONAmmoParamByType(const std::string& ammo_name, AmmoParams* ammoParam)
{
    LOG_PROCESS("Searching ammo info for " << ammo_name << "...");
    bool found = false;
    try
    {
        if (ammoParam == nullptr)
        {
            LOG_ERROR("Pointer is null");
            return false;
        }
        std::ifstream file(AMMO_JSON_FILE_NAME);
        if (!file) {
            LOG_ERROR("Error opening file");
            return false;
        }

        json jParseData = json::parse(file);
        std::vector<ammo::Data> ammoData = jParseData.get<std::vector<ammo::Data>>();

        for (const auto& ammoItem : ammoData)
        {
            if (ammo_name == ammoItem.name)
            {
                found = true;
                ammoParam->name = ammoItem.name;
                ammoParam->mass = ammoItem.mass;
                ammoParam->drag = ammoItem.drag;
                ammoParam->lift = ammoItem.lift;
                break;
            }
        }
    }
    catch (const std::exception& e)
    {
        LOG_ERROR(e.what());
        return false;
    }

    if (found)
    {
        LOG_SUCCESS("Successfully found ammo type.");
        LOG_INFO("📄 Result:");
        LOG_INFO("  - Ammo name: " << ammoParam->name);
        LOG_INFO("  - m: " << ammoParam->mass);
        LOG_INFO("  - d: " << ammoParam->drag);
        LOG_INFO("  - l: " << ammoParam->lift);
        return true;
    }

    LOG_ERROR("Ammo was not found");
    return false;
}

bool getAmmoTimeOfFlight(float& result, 
    const DroneConfig * const droneConfig,
    const AmmoParams * const ammoParam)
{
    // LOG_PROCESS("Calculating time of fly...");

    float d = ammoParam->drag;
    float m = ammoParam->mass;
    float l = ammoParam->lift;
    float v0 = droneConfig->attackSpeed;
    float z0 = droneConfig->altitude;

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

    // LOG_SUCCESS("Successfully found time of flight");

    LOG_INFO("📄 Result:");
    LOG_INFO("  - t: " << t);

    result = t;
    return true;
}

bool getHorizontalFlightRange(float& result, 
    const DroneConfig * const droneConfig,
    const AmmoParams * const ammoParam,
    const float& ammoTimeOfFlight)
{
    // LOG_PROCESS("Calculating horizontal flight range...");

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
    float d = ammoParam->drag;
    float m = ammoParam->mass;
    if (std::abs(m) < 1e-6f)
    {
        LOG_ERROR("Invalid m: we cannot divide by zero");
        return false;
    }
    float l = ammoParam->lift;
    float v0 = droneConfig->attackSpeed;

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

    // LOG_SUCCESS("Successfully calculated horizontal flight range");

    LOG_INFO("📄 Result:");
    LOG_INFO("  - h: " << h << "[m]");

    result = h;
    return true;
}

bool getDistanceToTarget(float& result, const Coord& drone, const Coord& target)
{
    // LOG_PROCESS("Calculating horizontal distance from copter to target...");

    // D = √( (targetX − xd)² + (targetY − yd)² )
    // targetX, targetY - координати цілі
    // xd, yd - координати дрона

    Coord vector = target - drone;
    result = vector.getVectorLength();

    // LOG_SUCCESS("Successfully calculated distance from drone to target");

    LOG_INFO("📄 Result:");
    LOG_INFO("  - D: " << result << "[m]");

    return true;
}

bool getAmmoDropPoint(Coord& result, 
    const Coord& drone,
    const float& D, 
    const float& h, 
    const Coord& target)
{
    // LOG_PROCESS("Calculating ammo drop point...");    

    // ratio = (D − h) / D
    // fireX = xd + (targetX − xd) · ratio
    // fireY = yd + (targetY − yd) · ratio

    if (std::abs(D) < 1e-6f)
    {
        LOG_ERROR("Invalid D: we cannot divide by zero");
        return false;
    }

    float ratio = (D - h) / D;

    result.x = drone.x + (target.x - drone.x) * ratio;
    result.y = drone.y + (target.y - drone.y) * ratio;

    // LOG_SUCCESS("Successfully calculated ammo drop point.");

    LOG_INFO("📄 Result:");
    LOG_INFO("  - fireX: " << result.x);
    LOG_INFO("  - fireY: " << result.y);

    return true;
}

bool interpolate(Coord& result, 
    const int& timeSteps,
    const float& timeFrame,
    const DroneConfig * const droneConfig,
    const Coord* const target)
{
    // LOG_PROCESS("Calculating new interpolated position...");

    /**
     * int idx = (int)floor(t / arrayTimeStep) % 60;
     * int next = (idx + 1) % 60;
     * float frac = (t - idx * arrayTimeStep) / arrayTimeStep;
     * float x = targetX[i][idx] + (targetX[i][next] - targetX[i][idx]) * frac;
     * 
     * t - конретний момент симуляції
     * arrayTimeStep - час між координатами цілей
     */

    if (std::abs(droneConfig->arrayTimeStep) < 1e-6f)
    {
        LOG_ERROR("Invalid arrayTimeStep: we cannot divide by zero");
        return false;
    }

    float t = timeFrame;
    
    int idx = (int)std::floor(t / droneConfig->arrayTimeStep) % timeSteps;
    int next = (idx + 1) % timeSteps;
    float frac = (t - idx * droneConfig->arrayTimeStep) / droneConfig->arrayTimeStep;

    auto x0 = target[idx].x;
    auto x1 = target[next].x;
    float x = x0 + (x1 - x0) * frac;
    result.x = x;

    auto y0 = target[idx].y;
    auto y1 = target[next].y;
    float y = y0 + (y1 - y0) * frac;
    result.y = y;

    // LOG_SUCCESS("Successfully interpolated distance for target " << targetIndex);

    LOG_INFO("📄 Result:");
    LOG_INFO("  - x0: " << x0 << " -> " << "x1: " << x1);
    LOG_INFO("  - y0: " << y0 << " -> " << "y1: " << y1);
    LOG_INFO("  - New x for target: " << x);
    LOG_INFO("  - New y for target: " << y);
    LOG_INFO("  - Time interval between target snapshots: " << frac * 100 << "%");

    return true;
}

bool getTargetVelocity(Coord& result, 
    const int& timeSteps,
    const SimState& state, 
    const DroneConfig * const droneConfig,
    const Coord* const target)
{
    // LOG_PROCESS("Calculating target velocity...");

    /**
     * float dx = targetX(t + dt) - targetX(t);
     * float dy = targetY(t + dt) - targetY(t);
     * float targetVx = dx / dt;
     * float targetVy = dy / dt;
     * 
     * t - конретний момент симуляції
     * dt - малий крок (наприклад, simTimeStep або arrayTimeStep)
     * targetX(t) - знайти Х координату де буде ціль в момент часу t
     * targetX(t + dt) - знайти Х координату де буде ціль через dt секунд від моменту t
     */

    float t = state.totalSimTime;
    float dt = droneConfig->arrayTimeStep;

    if (std::abs(dt) < 1e-6f)
    {
        LOG_ERROR("Invalid arrayTimeStep: we cannot divide by zero");
        return false;
    }

    float timeFrame0 = t;
    float timeFrame1 = t + dt;

    Coord currentPos{};
    if (!interpolate(currentPos, timeSteps, timeFrame0, droneConfig, target))
    {
        return false;
    }
    Coord nextPos{};
    if (!interpolate(nextPos, timeSteps, timeFrame1, droneConfig, target))
    {
        return false;
    }

    float x0 = currentPos.x;
    float x1 = nextPos.x;
    float dx = x1 - x0;

    float y0 = currentPos.y;
    float y1 = nextPos.y;
    float dy = y1 - y0;

    float targetVx = dx / dt;
    float targetVy = dy / dt;

    result.x = targetVx;
    result.y = targetVy;

    // LOG_SUCCESS("Successfully calculated velocity for target " << targetIndex);

    LOG_INFO("📄 Result:");
    LOG_INFO("  - Velocity x: " << targetVx);
    LOG_INFO("  - Velocity y: " << targetVy);

    return true;
}

bool getPredictedPosition(Coord& result, 
    const Coord& targetVelocity,
    const Coord& targetPosition,
    const float& totalTime)
{
    // LOG_PROCESS("Calculating predicted position...");

    /**
     * float predictedX = targetX(t) + targetVx * totalTime;
     * float predictedY = targetY(t) + targetVy * totalTime;
     * 
     * targetVx - швидкість цілі по осі x
     * targetVy - швидкість цілі по осі y
     * totalTime - орієнтовний час прильоту дрона до точки скиду
     */

    float predictedX = targetPosition.x + targetVelocity.x * totalTime;
    float predictedY = targetPosition.y + targetVelocity.y * totalTime;

    result.x = predictedX;
    result.y = predictedY;

    // LOG_SUCCESS("Successfully calculated predicted position");

    LOG_INFO("📄 Result:");
    LOG_INFO("  - Predicted x: " << predictedX);
    LOG_INFO("  - Predicted y: " << predictedY);

    return true;
}

bool getTimeToTarget(float& result, 
    const float& distanceToTarget, 
    const float& speed)
{
    // LOG_PROCESS("Calculating drone travel time to target point...");

    if (std::abs(speed) < 1e-6f)
    {
        LOG_ERROR("Invalid speed: cannot divide by zero");
        return false;
    }

    result = distanceToTarget / speed;

    // LOG_SUCCESS("Successfully calculated time for distance");

    LOG_INFO("📄 Result:");
    LOG_INFO("  - Distance to point: " << distanceToTarget << "[m]");
    LOG_INFO("  - Speed: " << speed << "[m/s]");
    LOG_INFO("  - Time required: " << result << "[s]");

    return true;
}

bool getNormalizedAngle(float& result)
{
    // LOG_PROCESS("Normalizing angle...");

    /**
     * Логіка нормалізації кута проста. Ми перевіряємо чи вхідний кут є
     * в межах [-180; 180]. Тобто якщо наприклад нам приходить кут 190
     * то логічно що ми пройшли уже 180 + 10 градусів
     * І замість того щоб робити ці зайві 10 градусів,
     * ми будемо брати 190 - 360 = -170
     * Тобто нам простіше розвернутися в зворотню сторону на 170 і
     * досягнути тої самої точки: 190 градусів еквівалент -170 градусів
     */

    [[maybe_unused]] const float oldAngle = result;

    while (result > M_PI)
    {
        result -= 2.0f * M_PI;
    }

    while (result < -M_PI)
    {
        result += 2.0f * M_PI;
    }

    // LOG_SUCCESS("Successfully normalized");

    LOG_INFO("📄 Result:");
    LOG_INFO("  - Old angle: " << oldAngle);
    LOG_INFO("  - Normalized angle: " << result);

    return true;
}

bool getAcceleration(float& result, const DroneConfig * const droneConfig)
{
    // LOG_PROCESS("Calculating dron acceleration...");

    if (std::abs(droneConfig->accelPath) < 1e-6f)
    {
        LOG_ERROR("Invalid accelerationPath: cannot divide by zero");
        return false;
    }

    result = (droneConfig->attackSpeed * droneConfig->attackSpeed) / (2.0f * droneConfig->accelPath);

    // LOG_SUCCESS("Successfully calculated acceleration");

    LOG_INFO("📄 Result:");
    LOG_INFO("  - Dron acceleration: " << result);

    return true;
}

bool writeOutputToFile(const OutputData& outputData)
{
    // LOG_PROCESS("Writing result to " << SIMULATION_JSON_FILE_NAME << "...");

    try
    {
        std::ofstream file(SIMULATION_JSON_FILE_NAME);
        if (!file)
        {
            LOG_ERROR("Error opening file");
            return false;
        }

        simulation::Data simData;

        simData.totalSteps = outputData.totalSteps;

        for (size_t i = 0; i < outputData.totalSteps; i++)
        {
            auto step = outputData.steps[i];
            simulation::Step_Data stepData{};
            stepData.position = step->pos;
            stepData.direction = step->direction;
            stepData.state = step->state;
            stepData.targetIdx = step->targetIdx;
            stepData.dropPoint = step->dropPoint;
            stepData.aimPoint = step->aimPoint;
            stepData.predictedTarget = step->predictedTarget;

            simData.steps.push_back(stepData);
        }
        
        json jOut = simData;

        file << jOut.dump(4);

    }
    catch(const std::exception& e)
    {
        LOG_ERROR(e.what());
        return false;
    }
    
    // LOG_SUCCESS("Successfully wrote result to file");

    return true;
}
