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
Coord* JsonTargetProvider::getTarget(int idx) const
{
    return targetsData.targets[idx];
}
int JsonTargetProvider::getTargetsTimeSteps() const
{
    return targetsData.timeSteps;
}
JsonTargetProvider::~JsonTargetProvider()
{
    if (targetsData.targets)
    {
        for (int i = 0; i < targetsData.targetCount; i++)
            delete[] targetsData.targets[i];

        delete[] targetsData.targets;
        targetsData.targets = nullptr;
    }
}
