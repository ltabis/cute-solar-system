#pragma once

#include "Engine.hpp"
#include "primitives/Line.hpp"

constexpr auto gravitational_constant = 0.000001f;

using namespace entt::literals;

namespace css {

namespace CelestialBody {

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
    std::size_t lifetime;
};

struct OrbitVizualiser {
    entt::entity parent;
    entt::entity orbit_gizmo;
    std::size_t iterations;
    std::chrono::milliseconds refresh_rate;

    static auto
        emplace(entt::registry &world, const entt::entity &entity, std::size_t iterations, std::chrono::milliseconds refresh_rate)
            -> OrbitVizualiser &
    {
        auto &vizualiser =
            world.emplace_or_replace<OrbitVizualiser>(entity, entity, world.create(), iterations, refresh_rate);

        const auto compute_n_iterations = [&world,
                                           emplaced_visualizer = world.get<OrbitVizualiser>(entity)]() -> void {
            // reconstructing the gizmo's positions.

            world.remove_if_exists<kawe::Render::VBO<kawe::Render::VAO::Attribute::POSITION>>(
                emplaced_visualizer.orbit_gizmo);

            std::vector<float> vertices{};
            vertices.reserve(emplaced_visualizer.iterations * 3);

            glm::vec3 body_position = world.get<kawe::Position3f>(emplaced_visualizer.parent).component;
            glm::vec3 body_velocity = world.get<kawe::Velocity3f>(emplaced_visualizer.parent).component;
            const auto body_mass = world.get<CelestialBody::MassF>(emplaced_visualizer.parent).mass;

            for (std::size_t it = 0; it < emplaced_visualizer.iterations; ++it)
                for (const auto &other : world.view<entt::tag<"CelestialBody"_hs>>()) {
                    // ! could break if id system change for an object.
                    // TODO: refactore this.
                    if (emplaced_visualizer.parent == other) continue;

                    const glm::vec3 other_position = world.get<kawe::Position3f>(other).component;
                    const auto other_mass = world.get<CelestialBody::MassF>(other).mass;

                    const auto sqr_dist =
                        static_cast<float>(glm::pow((body_position - other_position).length(), 2));
                    const auto force_dir = glm::normalize(body_position - other_position);
                    const auto force = force_dir * gravitational_constant * body_mass * other_mass / sqr_dist;
                    const auto acceleration = force / body_mass;

                    // updating the current body's velocity.
                    body_velocity += acceleration;
                    body_position += body_velocity;

                    vertices.push_back(body_position.x);
                    vertices.push_back(body_position.y);
                    vertices.push_back(body_position.z);
                }

            kawe::Render::VBO<kawe::Render::VAO::Attribute::POSITION>::emplace(
                world, emplaced_visualizer.orbit_gizmo, vertices, 3);
            world.emplace_or_replace<kawe::FillColor>(
                emplaced_visualizer.orbit_gizmo, glm::vec4{1.0f, 1.0f, 1.0f, 1.0f});

            std::for_each(vertices.begin(), vertices.end(), [](tinyobj::real_t coord) { spdlog::warn(coord); });
        };

        kawe::Clock::emplace(world, entity, refresh_rate, compute_n_iterations);

        return vizualiser;
    }
};

} // namespace CelestialBody

} // namespace css
