#include "loaders/FileConfigLoader.hpp"
#include "base/Ballistics.hpp"

bool FileConfigLoader::load()
{
    droneConfig = new DroneConfig();
    if (!getJSONDroneConfig(droneConfig))
    {
        delete droneConfig;
        droneConfig = nullptr;
        return false;
    }
    ammoParam = new AmmoParams();
    if (!getJSONAmmoParamByType(droneConfig->ammoName, ammoParam))
    {
        delete ammoParam;
        ammoParam = nullptr;
        return false;
    }
    return true;
}
AmmoParams* FileConfigLoader::getAmmoParams() const
{
    return ammoParam;
}
DroneConfig* FileConfigLoader::getConfig() const
{
    return droneConfig;
}
FileConfigLoader::~FileConfigLoader()
{
}
