#include "Common.hpp"
#include "interfaces/IConfigLoader.hpp"
#include "loaders/FileConfigLoader.hpp"

IConfigLoader* createLoader(LoaderType type)
{
    IConfigLoader* loader = nullptr;
    switch (type) {
        case LoaderType::FILE:
        {
            loader = new FileConfigLoader();
            break;
        }
        default:
        {
            loader = nullptr;
            break;
        }
    }
    return loader;
}