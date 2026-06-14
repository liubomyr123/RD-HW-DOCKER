#include "Common.hpp"
#include "base/MissionProcessor.hpp"
#include "interfaces/IBallisticSolver.hpp"
#include "factories/SolverFactory.hpp"
#include "factories/ProviderFactory.hpp"

#ifdef _WIN32
#include <windows.h>
#endif

int main()
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    IBallisticSolver* solver = createSolver(SolverType::ANALYTICAL);
    ITargetProvider* targets = createProvider(ProviderType::JSON);

    MissionProcessor mission(solver, targets);
    mission.init(LoaderType::FILE);
    while (mission.hasNext()) 
    { 
        mission.step();
    }
    mission.writeOutput();

    return 0;
}
