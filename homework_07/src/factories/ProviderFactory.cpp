#include "Common.hpp"
#include "interfaces/ITargetProvider.hpp"
#include "providers/JsonTargetProvider.hpp"

ITargetProvider* createProvider(ProviderType type)
{
    ITargetProvider* provider = nullptr;
    switch (type) {
        case ProviderType::JSON:
        {
            provider = new JsonTargetProvider();
            break;
        }
        default:
        {
            provider = nullptr;
            break;
        }
    }
    return provider;
}