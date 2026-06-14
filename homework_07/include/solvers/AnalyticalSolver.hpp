#include "interfaces/IBallisticSolver.hpp"

// Аналітичне рішення (формула з ДЗ1)
class AnalyticalSolver : public IBallisticSolver
{
    float ammoTimeOfFlight;
    float horizontalFlightRange;
    float acceleration;
public:
    virtual bool solve(
        const DroneConfig* const droneConfig,
        const AmmoParams* const ammoParams,
        const ITargetProvider* const targets,
        SimState& state, 
        OutputData& outputData) override;
};
