#pragma once

#include "Engine.hpp"

namespace css
{

namespace CelestialBody
{

template<typename T>
struct Mass {
    T mass;
};

using MassF = Mass<float>;

template<typename T>
struct Size {
    std::size_t size;
};

using SizeF = Size<float>;

struct LifeTime {
    // TODO: to replace by std::chrono.
    std::size_t size;
};

}

}