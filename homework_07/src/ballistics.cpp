#include "../include/ballistics.hpp"
#include "../include/logger.hpp"

#include <cmath>
#include <cstdlib>
#include <fstream>
#include <string>

bool getJSONDroneConfig(DroneConfig* droneConfig, std::string CONFIG_JSON_FILE_NAME)
{
    LOG_PROCESS("Reading " + CONFIG_JSON_FILE_NAME + "...");
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

bool getTargetsJSONData(TargetsData& targetsData, std::string TARGETS_JSON_FILE_NAME)
{
    // LOG_PROCESS("Reading " + TARGETS_JSON_FILE_NAME + "...");
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

    // LOG_SUCCESS("Successfully read all data for " << 5 << " targets.");

    return true;
}

bool getJSONAmmoParamByType(const std::string& ammo_name, AmmoParams* ammoParam, std::string AMMO_JSON_FILE_NAME)
{
    // LOG_PROCESS("Searching ammo info for " << ammo_name << "...");
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
        // LOG_SUCCESS("Successfully found ammo type.");
        LOG_INFO("📄 Result:");
        LOG_INFO("  - Ammo name: " << ammoParam->name);
        LOG_INFO("  - m: " << ammoParam->mass);
        LOG_INFO("  - d: " << ammoParam->drag);
        LOG_INFO("  - l: " << ammoParam->lift);
        return true;
    }

    // LOG_ERROR("Ammo was not found");
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
    const int& targetIndex,
    const float& timeFrame,
    const DroneConfig * const droneConfig,
    const TargetsData& targetsData)
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
    
    int idx = (int)std::floor(t / droneConfig->arrayTimeStep) % targetsData.timeSteps;
    int next = (idx + 1) % targetsData.timeSteps;
    float frac = (t - idx * droneConfig->arrayTimeStep) / droneConfig->arrayTimeStep;

    float x0 = targetsData.targets[targetIndex][idx].x;
    float x1 = targetsData.targets[targetIndex][next].x;
    float x = x0 + (x1 - x0) * frac;
    result.x = x;

    float y0 = targetsData.targets[targetIndex][idx].y;
    float y1 = targetsData.targets[targetIndex][next].y;
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
    const int targetIndex,
    const SimState& state, 
    const DroneConfig * const droneConfig,
    const TargetsData& targetsData)
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
    if (!interpolate(currentPos, targetIndex, timeFrame0, droneConfig, targetsData))
    {
        return false;
    }
    Coord nextPos{};
    if (!interpolate(nextPos, targetIndex, timeFrame1, droneConfig, targetsData))
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

bool writeOutputToFile(const OutputData& outputData, std::string SIMULATION_JSON_FILE_NAME)
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



















#define CONFIG_JSON_FILE_NAME "data/config.json"
#define AMMO_JSON_FILE_NAME "data/ammo.json"
#define TARGETS_JSON_FILE_NAME "data/targets.json"
#define SIMULATION_JSON_FILE_NAME "data/simulation.json"

// Завантажує цілі з JSON-файлу
class JsonTargetProvider : public ITargetProvider
{
    TargetsData targetsData;
public:
    bool load()
    {
        if (!getTargetsJSONData(targetsData, TARGETS_JSON_FILE_NAME))
        {
            return false;
        }
        return true;
    }
    int getTargetCount() override
    {
        return targetsData.targetCount;
    }
    Coord* getTarget(int idx) override
    {
        return targetsData.targets[idx];
    }
    TargetsData getTargetsData() override
    {
        return targetsData;
    }
};

// Аналітичне рішення (формула з ДЗ1)
class AnalyticalSolver : public IBallisticSolver
{
    virtual bool solve() override
    {
        return true;
    }
};

// Читає конфіг та параметри боєприпасу з файлу
class FileConfigLoader : public IConfigLoader
{
    DroneConfig *droneConfig = nullptr;
    AmmoParams *ammoParam = nullptr;
public:
    bool load() override
    {
        droneConfig = new DroneConfig();
        if (!getJSONDroneConfig(droneConfig, CONFIG_JSON_FILE_NAME))
        {
            return false;
        }
        ammoParam = new AmmoParams();
        if (!getJSONAmmoParamByType(droneConfig->ammoName, ammoParam, AMMO_JSON_FILE_NAME))
        {
            return false;
        }
        return true;
    }
    AmmoParams* getAmmoParams() override
    {
        return ammoParam;
    }
    DroneConfig* getConfig() override
    {
        return droneConfig;
    }
    ~FileConfigLoader()
    {
        if (droneConfig != nullptr)
        {
            delete droneConfig;
        }
        if (ammoParam != nullptr)
        {
            delete ammoParam;
        }
    }
};

enum class SolverType   { ANALYTICAL };
enum class ProviderType { JSON };
enum class LoaderType   { FILE };
 
IBallisticSolver* createSolver(SolverType type)
{
    IBallisticSolver* provider = nullptr;
    switch (type) {
        case SolverType::ANALYTICAL:
        {
            provider = new AnalyticalSolver();
            break;
        }
        default:
        {
            provider = nullptr;
            break;
        }
    }
    return provider;
}
ITargetProvider* createProvider(ProviderType type, std::string param)
{
    ITargetProvider* provider = nullptr;
    switch (type) {
        case ProviderType::JSON:
        {
            provider = new JsonTargetProvider();
            break;
        }
        default:
        {
            provider = nullptr;
            break;
        }
    }
    return provider;
}
IConfigLoader* createLoader(LoaderType type)
{
    IConfigLoader* provider = nullptr;
    switch (type) {
        case LoaderType::FILE:
        {
            provider = new FileConfigLoader();
            break;
        }
        default:
        {
            provider = nullptr;
            break;
        }
    }
    return provider;
}

class MissionProcessor
{
    IBallisticSolver* solver = nullptr;
    ITargetProvider* targets = nullptr;
    DroneConfig* droneConfig = nullptr;
    AmmoParams* ammoParams = nullptr;

    SimState state{};
    OutputData outputData{};
    float ammoTimeOfFlight = 0.0f;
    float horizontalFlightRange = 0.0f;
    float acceleration = 0.0f;

    int simulation_count = 0;

    MissionProcessor(IBallisticSolver* s, ITargetProvider* t) : solver(s), targets(t) {}

    // Завантажити конфіг через IConfigLoader, підготувати дані для ітерації
    bool init(LoaderType type)
    {
        IConfigLoader* configLoader = createLoader(type);
        if (configLoader == nullptr)
        {
            return false;
        }
        if (!configLoader->load())
        {
            return false;
        }
        droneConfig = configLoader->getConfig();
        ammoParams = configLoader->getAmmoParams();
        if (droneConfig == nullptr || ammoParams == nullptr)
        {
            return false;
        }
        if (targets == nullptr)
        {
            return false;
        }
        if (!targets->load())
        {
            return false;
        }
        if (!getAmmoTimeOfFlight(ammoTimeOfFlight, droneConfig, ammoParams)) 
        {
            return false;
        }
        if (!getHorizontalFlightRange(horizontalFlightRange, droneConfig, ammoParams, ammoTimeOfFlight)) 
        {
            return false;
        }
        if (!getAcceleration(acceleration, droneConfig)) 
        {
            return false;
        }
        // 
        state.dronePosition = droneConfig->startPos;
        state.droneZ = droneConfig->altitude;
        // 
        state.droneDir = droneConfig->initialDir;
        state.dropPointDir = droneConfig->initialDir;
        state.droneState = STOPPED;
        // 
        return true;
    }
    // Перевірити, чи є ще необроблені цілі
    bool hasNext()
    {
        if (simulation_count <= SIM_MAX_STEPS)
        {
            return true;
        }
        return false;
    }
    // Обробити наступну ціль: взяти дані з ITargetProvider, обчислити через IBallisticSolver, повернути DropPoint
    bool step()
    {
        int targets_number = targets->getTargetCount();
        TargetsData targetsData = targets->getTargetsData();
        // 1. Взяти наступну ціль через targets->getTarget(currentIdx)
        // 2. Викликати solver->solve(dronePos, target.pos, altitude, ammo ...)
        // 3. Збільшити лічильник, повернути результат

        LOG_DEBUG("------ FRAME " << count << " ------");
        LOG_INFO("Total simulation time elapsed: " << sim.state.totalSimTime << "[s]");

        // Знайшли координати цілей на даний момент
        std::vector<Coord> interpolatedPosition(targets_number);
        for (int i = 0; i < targets_number; i++)
        {
            if (!interpolate(interpolatedPosition[i], i, state.totalSimTime, droneConfig, targetsData))
            {
                return false;
            }
        }

        // Шукаємо найкращу ціль на даний момент
        // Ітераційно пройтися та:
        // 1) розрахувати для кожної цілі точку скиду
        // 2) розрахувати час який треба дрону щоб долетіти до точки скиду + час польоту снаряду
        // 3) розрахувати саму позицію цілі
        // 4) вибрати ту ціль до якої треба найменше часу летіти
        float smallestTime = std::numeric_limits<float>::max();
        int bestTargetIndex = -1;
        Coord bestDropPoint{};
        Coord bestTarget{};
        for (int targetIterator = 0; targetIterator < targets_number; targetIterator++) 
        {
            LOG_DEBUG("-----------: target # " << targetIterator);

            Coord initialTarget{
                interpolatedPosition[targetIterator].x, 
                interpolatedPosition[targetIterator].y
            };

            // Нехай перше припущення цілі - там де ціль зараз стоїть
            Coord predictedTarget = initialTarget;
            Coord predictedDropPoint = initialTarget;

            for (size_t j = 0; j < 6; j++)
            {
                LOG_DEBUG("-----------: prediction # " << j);

                // ✅ 1) Треба знайти точку скиду 
                // ✅ 2) Скільки часу дрон буде до неї летіти 
                // ✅ 3) Час польоту снаряду ми уже маємо - ammoTimeOfFlight 
                // ✅ 4) І знаючи час долітання до точки скиду та політ снаряду можемо знайти 
                //                      де буде ціль від поточного моменту через цей час
                // ✅ 5) Оновити прогнозоване місце цілі і повторити ітерацію
                // ✅ 6) В кінці ми отримаємо для цієї цілі нове місце цілі та точку скиду

                float distanceToTarget = 0.0f;
                if (!getDistanceToTarget(distanceToTarget, 
                    state.dronePosition, 
                    predictedTarget)) 
                {
                    return false;
                }

                Coord ammoDropPoint{};
                if (!getAmmoDropPoint(ammoDropPoint,
                    state.dronePosition, 
                    distanceToTarget, 
                    horizontalFlightRange, 
                    predictedTarget)) 
                {
                    return false;
                }
                predictedDropPoint = ammoDropPoint;

                float distanceToDropPoint = 0.0f;
                if (!getDistanceToTarget(distanceToDropPoint, 
                    state.dronePosition, 
                    ammoDropPoint)) 
                {
                    return false;
                }

                float timeToDropPoint = 0.0f;
                if (!getTimeToTarget(timeToDropPoint, 
                    distanceToDropPoint, 
                    droneConfig->attackSpeed)) 
                {
                    return false;
                }

                float totalTime = timeToDropPoint + ammoTimeOfFlight;

                Coord targetVelocity{};
                if (!getTargetVelocity(targetVelocity, 
                    targetIterator, 
                    state, 
                    droneConfig, 
                    targetsData))
                {
                    return false;
                }
                Coord predictedPosition{};
                if (!getPredictedPosition(predictedPosition, 
                    targetVelocity, 
                    initialTarget,
                    totalTime)) 
                {
                    return false;
                }

                predictedTarget = predictedPosition;
            }
        
            // Далі треба знайти timeToStop, тобто додатковий час необхідний якщо ми хочемо змінити ціль
            // Будемо рахувати штрафний час до зупинки
            float timeToStop = 0.0f;
            switch (state.droneState)
            {
                case STOPPED:
                    // Ми уже стоїмо і нам не треба додаткового часу
                    break;
                case ACCELERATING:
                    // Швидкість дрона >=0, тому нам треба врахувати скільки часу
                    // дрон уже розганяється, як штрафний час необхідний щоб зупинитися
                    // 
                    // Наприклад:
                    //      - поточна швидкість = 8 м/с
                    //      - прискорення - 5 м/с
                    // 
                    // Тоді час який дрон уже затратив на досягнення швидкості 8 м/с буде:
                    // 5 м/с - 1 с
                    // 8 м/с - х с
                    // х = (8 м/с) / (5 м/с) = 1.6 c
                    timeToStop = state.droneVelocity / acceleration;
                    break;
                case MOVING:
                    // Дрон уже рухається зі стабільною макс швидкістю, отже треба врахувати
                    // час на гальмування = attackSpeed / acceleration, ми беремо 
                    // attackSpeed бо ми уже на цій стабільній швидкості
                    timeToStop = droneConfig->attackSpeed / acceleration;
                    break;
                case TURNING:
                    // Дрон зараз розвертається, і він все ще в процесі
                    // Не важливо чи він почав, чи ще в процесі, залишається ще певний кут 
                    // який йому залишилося розвернутися до попередньої активної цілі
                    // Отже, нам треба знати скільки по часу ще залишилося йому розвертатися і зупинитися
                    // 
                    // Наприклад, нехай залишилося ще розвернути 1.2 радіани
                    // І маємо максимальну швидкість повороту - angularSpeed рад/с, нехай 1 рад/с
                    // Тоді час необхідний щоб довернути решту 1.2 радіанів буде:
                    // 1 рад    - 1 с
                    // 1.2 рад  - х с
                    // х = (1.2 рад) / (1 рад) = 1.2 с
                    timeToStop = state.angleTurnLeft / droneConfig->angularSpeed;
                    break;
                case DECELERATING:
                    // Час необхідний щоб повністю зупинитися, тобто скільки ще часу він буде тормозити
                    // Ми маємо його поточну швидкість, яка падає
                    // І можемо знайти скільки ще часу він буде сповільнюватися до 0
                    // За умовою, швидкість прискорення та гальмування є однаковим
                    // Тому робимо так само як і в ACCELERATING
                    timeToStop = state.droneVelocity / acceleration;
                    break;
                default:
                    break;
            }

            float distanceToDropPoint = 0.0f;
            if (!getDistanceToTarget(distanceToDropPoint, 
                state.dronePosition, 
                predictedDropPoint)) 
            {
                return false;
            }

            float timeToDropPoint = 0.0f;
            if (!getTimeToTarget(timeToDropPoint, 
                distanceToDropPoint, 
                droneConfig->attackSpeed)) 
            {
                return false;
            }

            float totalTime = 0.0f;

            float distanceToTarget = 0.0f;
            if (!getDistanceToTarget(distanceToTarget, 
                state.dronePosition, 
                predictedTarget)) 
            {
                return false;
            }

            if (distanceToTarget >= horizontalFlightRange)
            {
                // Все добре, дрон є на достатній відстані від цілі щоб попасти снарядом
                totalTime = timeToDropPoint + ammoTimeOfFlight;
            }
            else
            {
                // Дрон перетнув межу horizontalFlightRange, треба врахувати час на маневр
                // Тому треба знайти скільки часу займе дрону на маневр: 
                //      - тобто пролетіти якусь додаткову дистанцію щоб мати можливість скинути
                //      - і можливо треба буде розвернутися до цілі, беремо найгірший випадок - розворот на 180 градусів
                // 
                // Нехай відстань від дрона до цілі - 5 метрів, а дистанція на скид - 7 метрів
                // Це означає, що дрон уже як 2 метри пропустив точку скиду
                // Тому йому треба відлетіти на ще на 5 метрів і ще як мінімум на 7 метрів, потім ще й розвернутися
                // 
                // 1 радіан = (180 градусів / M_PI)
                // 
                // Нехай у нас angularSpeed = 3 рад/с, тобто за 1 секунду ми повертаємось на 3 радіани або 3 * (180 градусів / M_PI)
                // Тоді щоб знайти час скільки дрон буде розвертатися на 180 градусів:
                // 
                // 1 сек - 3 * (180 градусів / M_PI)
                // х сек - 180 градусів
                // 
                // x сек = (180 градусів * 1 сек) / (3 * (180 градусів / M_PI))
                // x сек = ((180 градусів * 1 сек) / 1) * (M_PI / (3 * 180 градусів)) | скорочуємо на 180 градусів
                // x сек = ((1 сек) / 1) * (M_PI / (3))
                // x сек = M_PI / 3
                // Тобто час розвороту на 180 градусів - це M_PI / droneConfig->angularSpeed
                float timeOfFullTurn = (float)M_PI / droneConfig->angularSpeed;
                
                float restoreDistance = distanceToTarget + horizontalFlightRange;

                float restoreTime = 0.0f;
                if (!getTimeToTarget(restoreTime, restoreDistance, droneConfig->attackSpeed)) 
                {
                    return false;
                }

                totalTime = restoreTime + timeOfFullTurn + ammoTimeOfFlight;
            }

            // Якщо це якась інша ціль, не та до якої ми зараз летимо, будемо враховувати штраф на зміну цілі
            if (state.currentTargetIndex != -1 && static_cast<int>(targetIterator) != state.currentTargetIndex)
            {
                totalTime += timeToStop;
            }

            if (totalTime < smallestTime)
            {
                smallestTime = totalTime;
                bestTargetIndex = targetIterator;
                bestTarget = predictedTarget;
                bestDropPoint = predictedDropPoint;
            }
        }
        LOG_DEBUG("Best target -> " << bestTargetIndex);
        state.currentTargetIndex = bestTargetIndex;

        LOG_DEBUG("bestTargetX -> " << bestTarget.x);
        LOG_DEBUG("bestTargetY -> " << bestTarget.y);

        float bestDistanceToTarget = 0.0f;
        if (!getDistanceToTarget(bestDistanceToTarget, 
            state.dronePosition, 
            bestTarget)) 
        {
            return false;
        }

        if (bestDistanceToTarget >= horizontalFlightRange)
        {
            // Все добре, дрон є на достатній відстані від цілі щоб попасти снарядом
            // Точка скиду залишається без змін
        }
        else
        {
            // Дрон перетнув межу horizontalFlightRange, треба знову врахувати час на маневр
            // Треба знайти нову точку скиду, треба знайти точку на відстані horizontalFlightRange від цілі
            // Тобто у мене є координати цілі та дрона і відстань horizontalFlightRange
            // 
            // 1) Знайти вектор від дрона до цілі
            // 2) Знаходимо одиничний вектор
            // 3) Рухаємося на певну відстань по цьому 1-ому вектору
            // 
            // Наприклад,
            //      - дрон (x1 = 1, y1 = 2) 
            //      - ціль (x2 = 5, y2 = 4) 
            //      - треба відступити від цілі на 3 метри
            // 
            // 1) Знадемо векток від дрона до цілі:
            // Вектор рахуємо (куди - звідки):
            //      drone -> target => (targetX - droneX; targetY - droneY)
            //      target -> drone => (droneX - targetX; droneY - targetY)
            // 
            //      (x2 - x1; y2 - y1)
            //      (5-1; 4-2) = (4; 2)
            // 
            // 2) Знадемо одиничний вектор:
            //      а) Знайти довжину ветора: √((x2 - x1)² + (y2 - y1)²)
            //          √(4² + 2²) = √(16 + 4) = √(20) ~ 4.47
            //      б) Поділити координати на довжину:
            //          (4 / 4.47; 2 / 4.47) = (0.89; 0.44) = (nx; ny)
            // 3) Рухаємося на 3 метри від цілі:
            //          (x2 - nx * 3; y2 - ny * 3)
            //          (5 - 0.89 * 3; 4 - 0.44 * 3)
            //          (5 - 2.67; 4 - 1.32)
            //          (2.33; 2.68)
            // Отже, (2.33; 2.68) - це координати точки на відстані в 3 метри від цілі в напрямку дрона

            Coord direction = bestTarget - state.dronePosition;
            Coord offset = (direction / direction.getVectorLength()) * horizontalFlightRange;
            bestDropPoint = bestTarget - offset;
        }

        // Тепер треба знайти чи треба нам скидутаи снаряд прямо зараз, щоб виконати місію
        // Для цього дрон має рухатися, тобто мати швидкість attackSpeed - швидкість при якій він скидає
        // І через те що і ціль рухається також нам треба знати де буде ціль якщо ми
        // прямо зараз в поточкому фреймі скинемо снаряд і воно пролетить ammoTimeOfFlight секунд часу
        // І порівняти 2 точки - де впаде снаряд і де буде ціль
        // В ідеалі якщо це однакові точки - ми фактично фіксуємо попадання
        // Але за умовою ми маємо hitRadius - це допустима похибка попадання в метрах
        // Отже, у нас буде певний запас на попадання цілі
        // Тому будемо шукати точку де буде ціль 
        // Шукаємо відстань від точки де впаде снаряд до точки де буде ціль і ця відстань
        // має бути меншою-рівною hitRadius
        // 1) Знаходимо одиничний вектор напрямку дрона:
        // 2) Рахуємо зміщення по координатах на horizontalFlightRange по знайдемону вектору
        // 3) Знаходимо точку де буде снаряд 
        Coord vecDirection = {std::cos(state.droneDir), std::sin(state.droneDir)};
        Coord hit = state.dronePosition + vecDirection * horizontalFlightRange;
        // 4) Шукаємо точку де буде ціль ціль через ammoTimeOfFlight
        Coord targetVelocity{};
        if (!getTargetVelocity(targetVelocity, 
            state.currentTargetIndex, 
            state, 
            droneConfig, 
            targetsData))
        {
            return false;
        }
        Coord predictedPosition{};
        if (!getPredictedPosition(predictedPosition, 
            targetVelocity, 
            bestTarget,
            ammoTimeOfFlight)) 
        {
            return false;
        }

        // 5) Шукаємо відстань між цими точками
        float hitDistance = 0.0f;
        if (!getDistanceToTarget(hitDistance, 
            hit, 
            predictedPosition)) 
        {
            return false;
        }

        SimStep *step = new SimStep;
        step->pos = state.dronePosition;
        step->direction = state.droneDir;
        step->state = state.droneState;
        step->targetIdx = state.currentTargetIndex;
        step->dropPoint = bestDropPoint;
        step->aimPoint = hit;
        step->predictedTarget = predictedPosition;

        outputData.steps[simulation_count] = step;
        outputData.totalSteps = simulation_count + 1;
        if (state.droneState == MOVING && hitDistance <= droneConfig->hitRadius)
        {
            // Ура, реєструємо ураження цілі, зупиняємо симуляцію
            return true;
        }

        // Тепр коли ми маємо нову ціль + її координати + нову точку скиду
        // треба зрозуміти чи має дрон розвернутися до точки скиду
        // 1) Для цього знайдемо спочатку напрямок куди дрон має дивитися для нової цілі
        Coord maneuverVector = bestDropPoint - state.dronePosition;
        // 2) Далі треба знайти кут між Х і точкою скиду
        float angleToDropPoint = std::atan2(maneuverVector.y, maneuverVector.x);
        // angleToDropPoint - це значення в радіанах, і воно обмежене [-M_PI; M_PI] або [-180 градусів; 180 градуів]
        // 3) І тепер ми можемо знайти різницю між тим куди зараз дивитися дрон і тим куди треба
        float deltaAngle = angleToDropPoint - state.droneDir;
        if (!getNormalizedAngle(deltaAngle)) 
        {
            return false;
        }
        // І тепер якщо deltaAngle = 0, це означає що дрон дивитися прямо на точку скиду
        // Якщо < 0, тоді треба розвертатися ліворуч, якщо > 0, тоді праворуч
        LOG_DEBUG("deltaAngle -> " << deltaAngle);

        /**
         * STOPPED          -> TURNING / ACCELERATING
         *      чи можемо ми перейти в MOVING - ні, спочатку треба розігнатися
         *      чи можемо ми перейти в DECELERATING - ні, ми уже стоїмо
         * 
         * ACCELERATING     -> MOVING / DECELERATING
         *      чи можемо ми перейти в STOPPED - ні, спочатку треба гальмувати
         *      чи можемо ми перейти в TURNING - ні, спочатку треба перейти зупинитися
         * 
         * MOVING           -> DECELERATING
         *      чи можемо ми перейти в STOPPED - ні, спочатку треба гальмувати
         *      чи можемо ми перейти в ACCELERATING - ні, бо ми уже набрали максимальну швидкість
         *      чи можемо ми перейти в TURNING - ні, бо спочатку треба зупинитися
         * 
         * TURNING          -> ACCELERATING
         *      чи можемо ми перейти в STOPPED - ні, немає сенсу, ми досягнули етапу STOPPED щоб почати розвертатися 
         *      чи можемо ми перейти в MOVING - ні, спочатку треба розігнатися
         *      чи можемо ми перейти в DECELERATING - ні, у нас уже швидкість = 0
         * 
         * DECELERATING     -> STOPPED
         *      чи можемо ми перейти в ACCELERATING - ні, ціль гальмування це повністю зупинитися
         *      чи можемо ми перейти в MOVING - ні, спочатку треба розігнатися
         *      чи можемо ми перейти в TURNING - ні, спочатку треба повністю зупинитися
         */
        // І тепер нарешті коли нам усе відомо про ціль, напрямок ми тепер можемо почати керувати дроном
        switch (state.droneState)
        {
            // Якщо дрон стоїть, то він або на початку симуляції, або зупинився 
            // щоб перейти в стан TURNING якщо змінився напрямок, або в стан ACCELERATING
            // якщо напрямок ок
            case STOPPED:
            {
                // Якщо кут між напрямками дрона і цілі більше за порогове значення - turnThreshold,
                // ми переходимо в стан TURNING
                if (std::fabs(deltaAngle) > droneConfig->turnThreshold)
                {
                    // Коли ми вирішили розвертатися, ми зберігаємо скільки ще нам залишається 
                    // градусів або радіанів щоб точно дивитися на ціль
                    // Ми уже маємо це значення - deltaAngle
                    state.angleTurnLeft = std::fabs(deltaAngle);
                    // Далі нам треба оновити напрямок цілі куди ми маємо 
                    // дивитися щоб на кожному кроці в TURNING розуміти скільки ще залишилося
                    state.dropPointDir = angleToDropPoint;
                    state.droneState = TURNING;
                }
                else
                {
                    // За умовою, ми можемо продовжити рух дрона і поступово повертатися дугою
                    // до цілі якщо ми кут менше turnThreshold
                    // Тому ми просто оновлюємо напрямок дрона на ціль і починаємо його рух
                    state.droneDir = angleToDropPoint;
                    state.droneState = ACCELERATING;
                }
                break;
            }
            // Стан при якому дрон починає гальмувати до швидкості в 0 м/с
            // Після досягнення нульової швидкості - переходить в стан STOPPED
            case DECELERATING:
            {
                // На кожному фреймі коли ми гальмуємо, нам треба знати яку 
                // швидкість ми обрізаємо від поточної швидкісті дрона
                // За умовою швидкість прискорення та швидкість гальмування однакові
                // Тому треба знайти яку швидкість розвиває дрон за simTimeStep з певним прискоренням
                // 
                // Наприклад
                //      - прискорення 5 м/с
                //      - крок симуляції 0.5 с
                // 
                // Прискорення означає як швидкість змінюється з часом
                // Наприкла прискорення в 5 м/с означає що за 1 секунду ми змінюємо швидкість на 5 м/с
                // тоді за 0.5 с прискорення буде на половину меншим
                float oldDroneVelocity = state.droneVelocity;
                float decelerationSpeed = acceleration * droneConfig->simTimeStep;
                state.droneVelocity -= decelerationSpeed;
                if (state.droneVelocity <= 0.0f)
                {
                    state.droneVelocity = 0.0f; // Вирівнюємо якщо швидкість відємна
                    state.droneState = STOPPED;
                }
                // Щоб гальмування відбувалося нам треба не лише зменшувати швидкість дрона 
                // як значення але і поступово зупиняти в просторі
                // Для цього ми маємо рухати дрон з новою швидкістю в певному напрямку з кроком в simTimeStep
                // Отже, ми маємо одиничний вектор який буде показувати напрямок 
                // Зважаючи на те що дрон змінює свою швидкість не моментально, нам треба знайти середню
                // швидкість з якою він буде летіти протягом simTimeStep часу
                // Наприклад, на попередньому фреймі дрон летів зі швидкістю 10 м/с, на цьому фреймі він 
                // зменшить її на 2 м/с, тобто нова швидкість в кінці фрейму = 8 м/с
                // Тоді середня буде (10 + 8) / 2 = 9 м/с
                float avgVelocity = (oldDroneVelocity + state.droneVelocity) * 0.5f;
                // І маємо знайти відстань на яку пролетить дрон зі швидкістю avgVelocity за simTimeStep час
                // Відстань знайти просто - знаючи швидкість та час, можна знайти яку відстань дрон
                // пролетить за цей час: якщо швидкість 5 м/с, а час 0.5 с, тоді відстань = 5 м/с * 0.5 с = 2.5 м
                float shift = avgVelocity * droneConfig->simTimeStep;
                state.dronePosition.x += std::cos(state.droneDir) * shift; 
                state.dronePosition.y += std::sin(state.droneDir) * shift; 
                break;
            }
            // Дрон перебуває в стані прискорення якщо він перед цим стояв
            // і зараз починає розганятися в напрямку цілі. Розгін відбувається максимум
            // до швидкості attackSpeed, після досягнення якої дрон переходить в стан MOVING
            // Якщо дрону треба розвертатися, тоді в цьому стані він так само може перейти в DECELERATING
            case ACCELERATING:
            {
                if (std::fabs(deltaAngle) > droneConfig->turnThreshold)
                {
                    state.droneState = DECELERATING;
                }
                else
                {
                    // Аналогічно до того як ми гальмуємо, ми і розганяємося
                    // Тобто знаходимо швидкість яку задтний набрати дрон за simTimeStep
                    float oldDroneVelocity = state.droneVelocity;
                    float accelerationSpeed = acceleration * droneConfig->simTimeStep;
                    state.droneVelocity += accelerationSpeed;
                    // Якщо ми досягаємо макс швидкості в attackSpeed, то переходимо в MOVING
                    if (state.droneVelocity >= droneConfig->attackSpeed)
                    {
                        state.droneVelocity = droneConfig->attackSpeed; // Стабілізуємо якщо перестирнбемо attackSpeed
                        state.droneState = MOVING;
                    }

                    // Перед тим як змінювати розташування дрона портібно постійно уточнювати 
                    // його напрямок, бо може бути таке що ми дивимося на нову ціль, але
                    // ми не переступили turnThreshold
                    state.droneDir = angleToDropPoint;

                    // І тепер аналогічним чином до DECELERATING, рухаємо дрон в просторі вперед
                    float avgVelocity = (oldDroneVelocity + state.droneVelocity) * 0.5f;
                    float shift = avgVelocity * droneConfig->simTimeStep;
                    Coord direction {
                        std::cos(state.droneDir),
                        std::sin(state.droneDir)
                    };
                    state.dronePosition = state.dronePosition + direction * shift;
                }
                break;
            }
            // Цей стан дрона означає що дрон набрав свою максимальну швидкість - attackSpeed
            // Тобто на цьому етапі швидкість дрона не перевищує attackSpeed
            // Якщо дрону треба розвертатися, тоді в цьому стані він так само може перейти в DECELERATING
            case MOVING:
            {
                if (std::fabs(deltaAngle) > droneConfig->turnThreshold)
                {
                    state.droneState = DECELERATING;
                }
                else
                {
                    // Якщо ми не перестрибнули turnThreshold, ми всеодно 
                    // можемо поступово повертатися до іншої цілі
                    state.droneDir = angleToDropPoint;

                    // І тепер аналогічним чином до DECELERATING, рухаємо дрон в просторі вперед
                    float shift = state.droneVelocity * droneConfig->simTimeStep;
                    Coord direction {
                        std::cos(state.droneDir),
                        std::sin(state.droneDir)
                    };
                    state.dronePosition = state.dronePosition + direction * shift;
                }
                break;
            }
            // Дрон попадає в цей стан після того як він повністю зупинився
            // В цьому стані він розвертається в певному напрямку зі 
            // швидкістю angularSpeed - значення в радіанах за секунду
            // Коли завершив розвертатися до цілі, переходить в стан ACCELERATING
            case TURNING:
            {
                // Аналогічно до того як ми шукали яку швидкість дрона може скинути чи набрати за simTimeStep
                // Так само треба знайти на скільки радіан він може розвернутися за simTimeStep
                // Але ще треба знайти в яку сторону. Для цього шукаємо нову різницю між напрямком дрона і цілі
                float newDeltaAngle = state.dropPointDir - state.droneDir;
                if (!getNormalizedAngle(newDeltaAngle)) 
                {
                    return false;
                }
                // І перевіряємо значення на знак, можливо нам треба буде почати розворот в протилежну сторону
                float turnDirection = newDeltaAngle > 0 ? 1.0f : -1.0f;
                // Щоб знайти цей кут ми беремо швидкість обертання angularSpeed і дивимося на скільки розвернемося 
                // за simTimeStep
                // 
                // Наприклад
                //      - angularSpeed - 1 рад/с
                //      - simTimeStep - 0.1 с
                // 
                // Тоді за 0.1 с дрон розвернеться на одну десяту від angularSpeed
                float turnShift = droneConfig->angularSpeed * droneConfig->simTimeStep;
                // Тепер коли ми знаємо на скільки дрон здатний повернутися за simTimeStep
                // Треба змінити його напрямок щоб він тепер почав дивитися з цим зсувом
                float newDroneDir = state.droneDir + (turnShift * turnDirection);
                if (!getNormalizedAngle(newDroneDir)) 
                {
                    return false;
                }
                state.droneDir = newDroneDir;
                // І оновити скільки залишилося довернути
                state.angleTurnLeft -= turnShift;
                
                // Якщо дрон уже достаньо розвернувся
                if (state.angleTurnLeft <= 0.0f)
                {
                    state.angleTurnLeft = 0.0f; // Знову таки зрізаємо ті зайві радіани які могли залишитися
                    state.droneDir = state.dropPointDir; // І тут тоже вирівнюємо рівно на ціль
                    state.droneState = ACCELERATING;
                }
                break;
            }
            default:
                break;
        }

        simulation_count++;
        state.totalSimTime = simulation_count * droneConfig->simTimeStep;
        LOG_INFO("");

        return true;
    }
    // Почати ітерацію спочатку
    bool reset()
    {
        return true;
    }
    // Підмінити solver на льоту (Стратегія)
    bool changeSolver(IBallisticSolver* newSolver)
    {
        if (newSolver == nullptr)
        {
            LOG_WARN("New solver is NULL");
            if (solver == nullptr)
            {
                LOG_WARN("Current solver is also NULL");
            }
            return false;
        }
        if (solver == nullptr)
        {
            LOG_WARN("Current solver is NULL");
        }
        else
        {
            delete solver;
            solver = nullptr;
        }
        solver = newSolver;
        return true;
    }
};