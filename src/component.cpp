#include "component.hpp"

ComponentFactory &getComponentFactory()
{
    static ComponentFactory factory;
    return factory;
}