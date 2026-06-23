#include "base/Ballistics.hpp"
#include "interfaces/IBallisticSolver.hpp"
#include "base/MissionProcessor.hpp"
#include "base/Logger.hpp"
#include "interfaces/IConfigLoader.hpp"
#include "factories/LoaderFactory.hpp"

MissionProcessor::MissionProcessor(IBallisticSolver* s, ITargetProvider* t)
    : solver(s), targets(t)
{
    droneConfig = nullptr;
    ammoParams = nullptr;
}

bool MissionProcessor::reset()
{
    return true;
}

bool MissionProcessor::changeSolver(IBallisticSolver* newSolver)
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

// 1. Взяти наступну ціль через targets->getTarget(currentIdx)
// 2. Викликати solver->solve(dronePos, target.pos, altitude, ammo ...)
// 3. Збільшити лічильник, повернути результат
bool MissionProcessor::step()
{
    if (solver == nullptr)
    {
        return false;
    }
    if (state.finished)
    {
        return false;
    }
    bool isOk = solver->solve(
        droneConfig,
        ammoParams,
        targets,
        state, 
        outputData
    );
    if (isOk == false)
    {
        return false;
    }
    state.simulation_count++;
    state.totalSimTime = state.simulation_count * droneConfig->simTimeStep;
    outputData.totalSteps = state.simulation_count;
    return true;
}

bool MissionProcessor::hasNext()
{
    if (!state.finished && state.simulation_count <= SIM_MAX_STEPS)
    {
        return true;
    }
    return false;
}

bool MissionProcessor::init(LoaderType type)
{
    IConfigLoader* configLoader = createLoader(type);
    if (configLoader == nullptr)
    {
        return false;
    }
    if (!configLoader->load())
    {
        delete configLoader;
        return false;
    }
    droneConfig = configLoader->getConfig();
    ammoParams = configLoader->getAmmoParams();
    if (droneConfig == nullptr || ammoParams == nullptr)
    {
        delete configLoader; 
        return false;
    }
    // 
    delete configLoader;
    configLoader = nullptr;
    // 
    if (targets == nullptr)
    {
        return false;
    }
    if (!targets->load())
    {
        return false;
    }
    outputData.steps.clear();
    outputData.steps.resize(SIM_MAX_STEPS + 1);
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

bool MissionProcessor::writeOutput()
{
    if (!writeOutputToFile(outputData)) 
    {
        return false;
    }
    return true;
}

MissionProcessor::~MissionProcessor()
{
    if (solver != nullptr)
    {
        delete solver;
    }
    if (targets != nullptr)
    {
        delete targets;
    }
    if (droneConfig != nullptr)
    {
        delete droneConfig;
    }
    if (ammoParams != nullptr)
    {
        delete ammoParams;
    }
    for (auto* step : outputData.steps)
    {
        delete step;
    }

    outputData.steps.clear();
    outputData.totalSteps = 0;
}
