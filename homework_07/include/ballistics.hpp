#pragma once

#include <string>
#include <cmath>
#include "json.hpp"
#include "logger.hpp"

using json = nlohmann::json;

#define GRAVITATIONAL_ACCELERATION 9.81f
#define SIM_MAX_STEPS 10000

#define CONFIG_JSON_FILE_NAME "data/config.json"
#define AMMO_JSON_FILE_NAME "data/ammo.json"
#define TARGETS_JSON_FILE_NAME "data/targets.json"
#define SIMULATION_JSON_FILE_NAME "data/simulation.json"

struct Coord {
	float x;
	float y;

    Coord operator+(const Coord& other) const 
    {
        Coord result{};
        result.x = x + other.x;
        result.y = y + other.y;
        return result;
    }

    Coord operator-(const Coord& other) const 
    {
        Coord result{};
        result.x = x - other.x;
        result.y = y - other.y;
        return result;
    }

    Coord operator*(float scalar) const 
    {
        Coord result{};
        result.x = x * scalar;
        result.y = y * scalar;
        return result;
    }

    Coord operator/(float s) const {
        return {x / s, y / s};
    }

    float getVectorLength() const
    {
        return std::hypot(x, y);
    }

    // bool operator==(const Coord& o) const {
    //     return std::abs(x - o.x) < 1e-6f &&
    //         std::abs(y - o.y) < 1e-6f;
    // }

    // Coord getDirectionVector() const
    // {
    //     float len = getVectorLength();

    //     if (len == 0.0f)
    //     {
    //         return Coord{0.0f, 0.0f};
    //     }

    //     return Coord{ x / len, y / len };
    // }
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Coord, 
    x, 
    y)

namespace config {
    struct Drone_Data {
        Coord position;
        float altitude;
        float initialDirection;
        float attackSpeed;
        float accelerationPath;
        float angularSpeed;
        float turnThreshold;
    };

    struct Simulation_Data {
        float timeStep;
        float hitRadius;
    };

    struct Data {
        Drone_Data drone;
        std::string ammo;
        Simulation_Data simulation;
        int targetArrayTimeStep;
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Drone_Data,
        position,
        altitude,
        initialDirection,
        attackSpeed,
        accelerationPath,
        angularSpeed,
        turnThreshold)
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Simulation_Data,
        timeStep,
        hitRadius)
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Data,
        drone,
        ammo,
        simulation,
        targetArrayTimeStep)
}

namespace simulation {
    /**
     *     | Назва              | Опис  |
     *     ------------------------------
     *     | dropPoint          | Точка скиду (firePoint) — куди летить дрон, щоб скинути бомбу. Розраховується з lead targeting. |
     *     | aimPoint           | Точка влучання бомби = позиція дрона + hDist у напрямку direction. Куди впаде бомба, якщо скинути просто зараз. |
     *     | predictedTarget    | Прогнозована позиція цілі через flightTime. Ідеальна точка — коли aimPoint == predictedTarget. |
     */
    struct Step_Data {
        Coord position;          	
        float direction;    	
        int   state;        	
        int   targetIdx;    	
        Coord dropPoint;    	
        Coord aimPoint;     	
        Coord predictedTarget;  
    };

    struct Data {
        int totalSteps;
        std::vector<Step_Data> steps;
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Step_Data,
        position,
        direction,
        state,
        targetIdx,
        dropPoint,
        aimPoint,
        predictedTarget)
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Data,
        totalSteps,
        steps)
}

namespace targets {
    struct Target_Item_Data {
        std::vector<Coord> positions;
    };

    struct Data {
        int targetCount;
        int timeSteps;
        std::vector<Target_Item_Data> targets;
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Target_Item_Data, 
        positions)
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Data, 
        targetCount,
        timeSteps,
        targets)
}

namespace ammo {
    struct Data {
        std::string name;
        float mass;
        float drag;
        float lift;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Data, 
        name, 
        mass, 
        drag, 
        lift)
}

/**
 *      | Значення  | Назва         | Опис                                          |
 *      -----------------------------------------------------------------------------
 *      | 0         | STOPPED       | Повна зупинка (v = 0))                        |
 *      | 1         | ACCELERATING  | Розгін від 0 до attackSpeed                   |
 *      | 2         | DECELERATING  | Гальмування від attackSpeed до 0              |
 *      | 3         | TURNING       | Поворот на місці (v \= 0, зміна напрямку)     |
 *      | 4         | MOVING        | Рух з крейсерською швидкістю attackSpeed      |
 */
enum DroneState
{
    STOPPED = 0,
    ACCELERATING,
    DECELERATING,
    TURNING,
    MOVING,
};

/**
 *     | Назва       | m (кг) | d (drag) | l (lift) | Тип               |
 *     ------------------------------------------------------------------
 *     | VOG-17      | 0.35   | 0.07     | 0.0      | Вільне падіння    |
 *     | M67         | 0.6    | 0.10     | 0.0      | Вільне падіння    |
 *     | RKG-3       | 1.2    | 0.10     | 0.0      | Вільне падіння    |
 *     | GLIDING-VOG | 0.45   | 0.10     | 1.0      | Планеруючий       |
 *     | GLIDING-RKG | 1.4    | 0.10     | 1.0      | Планеруючий       |
 * 
 *     m — маса боєприпасу (кг)
 *     d — коефіцієнт аеродинамічного опору
 *     l — коефіцієнт підйомної сили (0 = вільне падіння, 1 = планерування).
 */
struct AmmoParams {
	std::string name;
	float mass; 	// маса (кг)
	float drag; 	// коефіцієнт опору
	float lift; 	// коефіцієнт підйому
};

struct DroneConfig {
	Coord startPos;     	// початкова позиція (x, y)
	float altitude;     	// висота
	float initialDir;   	// початковий напрямок (рад)
	float attackSpeed;  	// швидкість атаки (м/с)
	float accelPath;    	// шлях розгону (м)
	std::string ammoName; 	// обрані боєприпаси
	float arrayTimeStep;	// крок часу масиву цілей
	float simTimeStep;  	// крок симуляції
	float hitRadius;    	// радіус влучення
	float angularSpeed; 	// кутова швидкість (рад/с)
	float turnThreshold;	// поріг повороту (рад)
};

struct SimStep {
	Coord pos;          	// позиція дрона
	float direction;    	// напрямок (рад)
	int   state;        	// стан автомата (0-4)
	int   targetIdx;    	// індекс поточної цілі
	Coord dropPoint;    	// точка скиду (куди летить дрон)
	Coord aimPoint;     	// куди впаде бомба (якщо скинути зараз)
	Coord predictedTarget;  // прогнозована позиція цілі
};

struct SimState {
    float totalSimTime = 0.0f;

    Coord dronePosition;
    float droneZ;

    float droneDir;
    float droneVelocity = 0.0f;

    float angleTurnLeft = 0.0f;
    float dropPointDir;

    DroneState droneState = STOPPED;
    int currentTargetIndex = -1;
};

struct OutputData {
    size_t totalSteps = 0;
    SimStep** steps = nullptr;
};

struct TargetsData {
    int targetCount = 0;
    int timeSteps = 0;
    Coord** targets = nullptr;
};

bool getJSONDroneConfig(DroneConfig* droneConfig, std::string);
bool getTargetsJSONData(TargetsData& targetsData, std::string);
bool getJSONAmmoParamByType(const std::string& ammo_name, AmmoParams* ammoParam, std::string);
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
    const int& targetIndex,
    const float& timeFrame,
    const DroneConfig * const droneConfig,
    const TargetsData& targetsData);
bool getTargetVelocity(Coord& result, 
    const int targetIndex,
    const SimState& state, 
    const DroneConfig * const droneConfig,
    const TargetsData& targetsData);
bool getPredictedPosition(Coord& result, 
    const Coord& targetVelocity,
    const Coord& targetPosition,
    const float& totalTime);
bool getTimeToTarget(float& result, 
    const float& distanceToTarget, 
    const float& speed);
bool getNormalizedAngle(float& result);
bool getAcceleration(float& result, const DroneConfig * const droneConfig);
bool writeOutputToFile(const OutputData& outputData, std::string);

struct Simulation
{
    DroneConfig* droneConfig = nullptr;
    AmmoParams* ammoParam = nullptr;
    TargetsData targetsData{};
    OutputData outputData{};
    SimState state{};

    ~Simulation()
    {
        cleanup();
    }

    void cleanup()
    {
        LOG_PROCESS("Cleaning up memory...");
        delete droneConfig;
        droneConfig = nullptr;
        delete ammoParam;
        ammoParam = nullptr;

        if (targetsData.targets)
        {
            for (int i = 0; i < targetsData.targetCount; i++)
                delete[] targetsData.targets[i];

            delete[] targetsData.targets;
            targetsData.targets = nullptr;
        }

        if (outputData.steps)
        {
            for (size_t i = 0; i < outputData.totalSteps; i++)
            {
                delete outputData.steps[i];
                outputData.steps[i] = nullptr;
            }

            delete[] outputData.steps;
            outputData.steps = nullptr;
            
            outputData.totalSteps = 0;
        }
    }
};

// Провайдер цілей: кількість та дані кожної цілі (позиція, швидкість)
class ITargetProvider
{
public:
    bool load();
    virtual int getTargetCount() = 0;
    virtual Coord* getTarget(int idx) = 0;
    virtual TargetsData getTargetsData() = 0;
    virtual ~ITargetProvider() {}
};

// Калькулятор балістики: обчислює точку скиду
class IBallisticSolver
{
public:
    virtual bool solve() = 0;
    virtual ~IBallisticSolver() {}
};

// Завантажувач даних: конфіг місії та параметри боєприпасу
class IConfigLoader
{
public:
    virtual bool load() = 0;
    virtual DroneConfig* getConfig() = 0;
    virtual AmmoParams* getAmmoParams() = 0;
    virtual ~IConfigLoader() {}
};


enum class SolverType   { ANALYTICAL };
enum class ProviderType { JSON };
enum class LoaderType   { FILE };

class MissionProcessor
{   
private:
    IBallisticSolver* solver;
    ITargetProvider* targets;
    DroneConfig* droneConfig;
    AmmoParams* ammoParams;

    SimState state;
    OutputData outputData;
    float ammoTimeOfFlight;
    float horizontalFlightRange;
    float acceleration;

    int simulation_count;

public:
    MissionProcessor(IBallisticSolver* s, ITargetProvider* t);

    // Завантажити конфіг через IConfigLoader, підготувати дані для ітерації
    bool init(LoaderType type);
    // Перевірити, чи є ще необроблені цілі
    bool hasNext();
    // Обробити наступну ціль: взяти дані з ITargetProvider, обчислити через IBallisticSolver, повернути DropPoint
    bool step();
    // Почати ітерацію спочатку
    bool reset();
    // Підмінити solver на льоту (Стратегія)
    bool changeSolver(IBallisticSolver* newSolver);
    bool writeOutput();
};

IBallisticSolver* createSolver(SolverType type);
ITargetProvider* createProvider(ProviderType type);
IConfigLoader* createLoader(LoaderType type);
