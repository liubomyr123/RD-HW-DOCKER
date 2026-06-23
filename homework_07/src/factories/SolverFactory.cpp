#include "Common.hpp"
#include "interfaces/IBallisticSolver.hpp"
#include "solvers/AnalyticalSolver.hpp"

IBallisticSolver* createSolver(SolverType type)
{
    IBallisticSolver* solver = nullptr;
    switch (type) {
        case SolverType::ANALYTICAL:
        {
            solver = new AnalyticalSolver();
            break;
        }
        default:
        {
            solver = nullptr;
            break;
        }
    }
    return solver;
}