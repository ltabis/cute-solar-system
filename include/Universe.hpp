#pragma once

#include "celestial-bodies/CelestialBodyComponents.hpp"

using namespace entt::literals;

namespace css {

// TODO: convert this class into a component.
class Universe {
public:
    Universe(entt::registry &world) : m_World{world} {}

    auto add_body(
        const std::string &name,
        const std::string &model_path,
        const std::string &texture_path = "",
        const glm::vec3 &position = glm::vec3(0.f),
        const glm::vec3 &initial_velocity = glm::vec3(0.f),
        const float size = 1.f,
        const float mass = 1.f) -> entt::entity
    {
        auto body_model = m_World.create();

        kawe::Mesh::emplace(m_World, body_model, model_path);

        // loading texture if one is provided
        if (!texture_path.empty()) kawe::Texture2D::emplace(m_World, body_model, texture_path);

        const auto vbo = m_World.get<kawe::Render::VBO<kawe::Render::VAO::Attribute::POSITION>>(body_model);
        const auto index_size = vbo.vertices.size() / vbo.stride_size;
        kawe::Render::VBO<kawe::Render::VAO::Attribute::COLOR>::emplace(
            m_World, body_model, std::vector<float>(index_size * 4, 1.0f), 4);
        m_World.emplace<CelestialBody::MassF>(body_model, mass);
        m_World.emplace<CelestialBody::SizeF>(body_model, size);
        m_World.emplace<kawe::Name>(body_model, name);
        m_World.emplace<kawe::Scale3f>(body_model, glm::vec3(size));
        m_World.emplace<kawe::Position3f>(body_model, position);
        m_World.emplace<kawe::Velocity3f>(body_model, initial_velocity);
        // CelestialBody::OrbitVizualiser::emplace(m_World, body_model, 10, std::chrono::milliseconds(1000));
        m_World.emplace<entt::tag<"CelestialBody"_hs>>(body_model);

        m_Bodies.push_back(body_model);

        return body_model;
    }

    auto on_update_bodies(const kawe::event::TimeElapsed &e) -> void
    {
        const auto dt_nano = e.world_time;
        const auto dt_secs =
            static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(dt_nano).count())
            / 1'000'000.0;

        for (const auto &body : m_Bodies)
            for (const auto &other : m_Bodies) {
                // ! could break if id system change for an object.
                // TODO: refactore this.
                if (body == other) continue;

                const auto body_position = m_World.get<kawe::Position3f>(body).component;
                const auto body_mass = static_cast<double>(m_World.get<CelestialBody::MassF>(body).mass);
                const auto other_position = m_World.get<kawe::Position3f>(other).component;
                const auto other_mass = static_cast<double>(m_World.get<CelestialBody::MassF>(other).mass);

                const auto sqr_dist = glm::pow((body_position - other_position).length(), 2);
                const auto force_dir = glm::normalize(body_position - other_position);
                const auto force = force_dir * gravitational_constant * body_mass * other_mass / sqr_dist;
                const auto acceleration = force / body_mass;

                // updating the current body's velocity.
                m_World.patch<kawe::Velocity3f>(body, [&acceleration, &dt_secs](auto &velocity) {
                    velocity.component += acceleration * dt_secs;
                });
            }
    }

private:
    entt::registry &m_World;
    std::vector<entt::entity> m_Bodies;
};

} // namespace css
