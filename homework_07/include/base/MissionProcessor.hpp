#pragma once

#include "Common.hpp"
#include "dto/OutputData.hpp"
#include "dto/SimState.hpp"
#include "interfaces/IBallisticSolver.hpp"

// class IBallisticSolver;
// class ITargetProvider;
// class DroneConfig;
// class AmmoParams;

class MissionProcessor
{   
private:
    IBallisticSolver* solver;
    ITargetProvider* targets;
    DroneConfig* droneConfig;
    AmmoParams* ammoParams;

    SimState state;
    OutputData outputData;

public:
    MissionProcessor(IBallisticSolver* s, ITargetProvider* t);
    ~MissionProcessor();

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
