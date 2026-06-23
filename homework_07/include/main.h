#pragma once 

#ifdef _WIN32
#include <windows.h>
#endif

#include "Common.hpp"                       // IWYU pragma: keep
#include "base/MissionProcessor.hpp"        // IWYU pragma: keep
#include "interfaces/IBallisticSolver.hpp"  // IWYU pragma: keep
#include "factories/SolverFactory.hpp"      // IWYU pragma: keep
#include "factories/ProviderFactory.hpp"    // IWYU pragma: keep