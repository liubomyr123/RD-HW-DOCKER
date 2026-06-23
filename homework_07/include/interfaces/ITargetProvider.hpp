#pragma once

#include "dto/Target.hpp"

// Провайдер цілей: кількість та дані кожної цілі (позиція, швидкість)
class ITargetProvider
{
public:
    virtual bool load() = 0;
    virtual int getTargetCount() const = 0;
    virtual Target* getTarget(int idx) const = 0;
    virtual int getTargetsTimeSteps() const = 0;
    virtual ~ITargetProvider() = default;
};