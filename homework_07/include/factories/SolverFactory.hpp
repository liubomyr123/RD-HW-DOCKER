#pragma once

#include "Common.hpp"
#include "interfaces/IBallisticSolver.hpp"

IBallisticSolver* createSolver(SolverType type);
