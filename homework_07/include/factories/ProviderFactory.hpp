#pragma once

#include "Common.hpp"
#include "interfaces/ITargetProvider.hpp"

ITargetProvider* createProvider(ProviderType type);
