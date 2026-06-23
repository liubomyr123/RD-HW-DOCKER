#pragma once

#include "dto/TargetsData.hpp"
#include "interfaces/ITargetProvider.hpp"

// Завантажує цілі з JSON-файлу
class JsonTargetProvider : public ITargetProvider
{
    TargetsData targetsData;
public:
    bool load() override;
    int getTargetCount() const override;
    Target* getTarget(int idx) const override;
    int getTargetsTimeSteps() const override;
    ~JsonTargetProvider();
};
