#pragma once

#include "Common.hpp"
#include "interfaces/IConfigLoader.hpp"

IConfigLoader* createLoader(LoaderType type);
