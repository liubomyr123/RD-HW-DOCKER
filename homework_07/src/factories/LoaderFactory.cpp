#include "Common.hpp"
#include "interfaces/IConfigLoader.hpp"
#include "loaders/FileConfigLoader.hpp"

IConfigLoader* createLoader(LoaderType type)
{
    IConfigLoader* provider = nullptr;
    switch (type) {
        case LoaderType::FILE:
        {
            provider = new FileConfigLoader();
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