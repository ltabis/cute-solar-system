#pragma once

#include "Engine.hpp"
#include "primitives/Line.hpp"

constexpr auto gravitational_constant = 0.000001f;
constexpr auto gravitational_constant_d = 0.1;

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

    // class used to simulate orbits.
    struct DummyCelestialBody {
        glm::dvec3 position;
        glm::dvec3 velocity;
        float mass;

        entt::entity ref;
    };

    static auto emplace(entt::registry &world, const entt::entity &entity) -> OrbitVizualiser &
    {
        return world.emplace_or_replace<OrbitVizualiser>(entity, entity, world.create());
    }

    static auto copy_body(const entt::registry &world, const entt::entity &entity) -> DummyCelestialBody
    {
        return DummyCelestialBody{
            .position = world.get<kawe::Position3f>(entity).component,
            .velocity = world.get<kawe::Velocity3f>(entity).component,
            .mass = world.get<CelestialBody::MassF>(entity).mass,
            .ref = entity};
    }

    static auto compute_n_iterations(entt::registry &world, std::size_t iterations) -> void
    {
        spdlog::debug("computing {} iterations for orbits.", iterations);
        auto nb_celestial_bodies = world.size<OrbitVizualiser>();

        std::size_t body_index = 0;

        // keeping track of initial velocity & positions.
        // we don't want to update the actual bodies.
        auto simulation_bodies = std::vector<DummyCelestialBody>(nb_celestial_bodies);

        // initializing all 'virtual' bodies.
        for (auto entity : world.view<OrbitVizualiser>())
            simulation_bodies[body_index++] = copy_body(world, entity);

        // initializing all 'virtual' bodies.
        auto bodies_gizmos = std::vector<std::vector<float>>(nb_celestial_bodies);

        std::for_each(bodies_gizmos.begin(), bodies_gizmos.end(), [&iterations](auto &vertices) {
            vertices = std::vector<float>(iterations);
        });

        // simulating all body's orbits.
        for (std::size_t it = 0; it < iterations; ++it) {
            for (auto &body : simulation_bodies)
                for (auto &other : simulation_bodies) {
                    // ! could break if id system change for an object.
                    // TODO: refactore this.
                    if (body.ref == other.ref) continue;

                    const auto sqr_dist = glm::pow((body.position - other.position).length(), 2);
                    const auto force_dir = glm::normalize(body.position - other.position);
                    const auto force = force_dir * gravitational_constant_d * static_cast<double>(body.mass)
                                       * static_cast<double>(other.mass) / sqr_dist;
                    const auto acceleration = force / static_cast<double>(body.mass);

                    // updating the velocity of the current body.
                    body.velocity += acceleration;
                }

            body_index = 0;

            // updating all positions.
            for (auto &body : simulation_bodies) {
                // updating current body position.
                body.position += body.velocity;

                // adding new coords to the current gizmo's vertex.
                bodies_gizmos[body_index].push_back(static_cast<float>(body.position.x));
                bodies_gizmos[body_index].push_back(static_cast<float>(body.position.y));
                bodies_gizmos[body_index].push_back(static_cast<float>(body.position.z));

                ++body_index;
            }
        }

        body_index = 0;

        // updating orbits VAOs.
        for (auto &body : simulation_bodies) {
            auto &component = world.get<OrbitVizualiser>(body.ref);

            kawe::Render::VBO<kawe::Render::VAO::Attribute::POSITION>::emplace(
                world, component.orbit_gizmo, bodies_gizmos[body_index], 3);

            world.patch<kawe::Render::VAO>(component.orbit_gizmo, [](kawe::Render::VAO &vao) {
                vao.mode = kawe::Render::VAO::DisplayMode::LINES;
            });

            // world.emplace<kawe::FillColor>(component.orbit_gizmo, glm::vec4{1.0f, 1.0f, 1.0f, 1.0f});

            ++body_index;
        }

        spdlog::debug("finished computing iterations.");
    }
}; // namespace CelestialBody

} // namespace CelestialBody

} // namespace css
