#pragma once

#include "dto/AmmoParams.hpp"
#include "dto/DroneConfig.hpp"

// Завантажувач даних: конфіг місії та параметри боєприпасу
class IConfigLoader
{
public:
    virtual bool load() = 0;
    virtual DroneConfig* getConfig() const = 0;
    virtual AmmoParams* getAmmoParams() const = 0;
    virtual ~IConfigLoader() = default;
};