#include "providers/JsonTargetProvider.hpp"
#include "base/Ballistics.hpp"

bool JsonTargetProvider::load()
{
    if (!getTargetsJSONData(targetsData))
    {
        return false;
    }
    return true;
}
int JsonTargetProvider::getTargetCount() const
{
    return targetsData.targetCount;
}
Target* JsonTargetProvider::getTarget(int idx) const
{
    return targetsData.targets[idx];
}
int JsonTargetProvider::getTargetsTimeSteps() const
{
    return targetsData.timeSteps;
}
JsonTargetProvider::~JsonTargetProvider()
{
    for (auto* target : targetsData.targets)
    {
        delete[] target->coord;
        delete target;
    }

    targetsData.targets.clear();
}