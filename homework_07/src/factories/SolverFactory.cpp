#include "Common.hpp"
#include "interfaces/IBallisticSolver.hpp"
#include "solvers/AnalyticalSolver.hpp"

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