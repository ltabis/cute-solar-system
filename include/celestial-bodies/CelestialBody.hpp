#pragma once

#include "Engine.hpp"

namespace css
{

class CelestialBody
{
public:
    auto name() const -> std::string_view { return m_Name; }
    auto mass() const -> std::size_t { return m_Mass; }
    auto size() const -> std::size_t { return m_Size; }
    auto velocity() const -> const glm::vec3 & { return m_Velocity; }
    auto position() const -> const glm::vec3 & { return m_Position; }
    auto rotation() const -> const glm::vec3 & { return m_Rotation; }

private:
    std::string m_Name;
    std::size_t m_Mass;
    std::size_t m_Size;
    glm::vec3 m_Velocity;
    glm::vec3 m_Position;
    glm::vec3 m_Rotation;

    // TODO: to replace by std::chrono.
    std::optional<std::size_t> m_LifeTime;
};

}