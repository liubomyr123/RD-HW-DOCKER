#include "interfaces/IConfigLoader.hpp"

// Читає конфіг та параметри боєприпасу з файлу
class FileConfigLoader : public IConfigLoader
{
    DroneConfig *droneConfig = nullptr;
    AmmoParams *ammoParam = nullptr;
public:
    bool load() override;
    AmmoParams* getAmmoParams() const override;
    DroneConfig* getConfig() const override;
    ~FileConfigLoader();
};
