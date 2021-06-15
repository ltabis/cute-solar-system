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
    std::size_t iterations;
    std::chrono::milliseconds refresh_rate;

    static auto
        emplace(entt::registry &world, const entt::entity &entity, std::size_t iterations, std::chrono::milliseconds refresh_rate)
            -> OrbitVizualiser &
    {
        return world.emplace_or_replace<OrbitVizualiser>(entity, entity, world.create(), iterations, refresh_rate);
    }

    static auto compute_n_iterations(entt::registry &world) -> void
    {
        auto nb_celestial_bodies = world.size<OrbitVizualiser>();

        // keeping track of initial velocity & positions to reset them after.
        auto initial_position = std::unordered_map<entt::entity, glm::vec3>(nb_celestial_bodies);
        auto initial_velocity = std::unordered_map<entt::entity, glm::vec3>(nb_celestial_bodies);

        // computing orbits for each instance that emplaced the component.
        world.view<OrbitVizualiser>().each([&world, &initial_position, &initial_velocity](
                                               entt::entity entity, OrbitVizualiser &component) {
            // getting the body's characteristics.
            auto &body_position = world.get<kawe::Position3f>(entity).component;
            auto &body_velocity = world.get<kawe::Velocity3f>(entity).component;
            const auto body_mass = world.get<CelestialBody::MassF>(entity).mass;

            // storing initial values.
            initial_position[entity] = body_position;
            initial_velocity[entity] = body_velocity;

            // removing position attributes of the orbital gizmo if it exists.
            world.remove_if_exists<kawe::Render::VBO<kawe::Render::VAO::Attribute::POSITION>>(component.orbit_gizmo);

            // creating a buffer of coordinates to create the gizmo's vertices.
            auto vertices = std::vector<float>(component.iterations * 3);

            for (std::size_t it = 0; it < component.iterations; ++it) {
                // updating all velocities.
                for (const auto &other : world.view<entt::tag<"CelestialBody"_hs>>()) {
                    // ! could break if id system change for an object.
                    // TODO: refactore this.
                    if (entity == other) continue;

                    const auto other_position = world.get<kawe::Position3f>(other).component;
                    const auto other_mass = world.get<CelestialBody::MassF>(other).mass;

                    const auto sqr_dist = glm::pow((body_position - other_position).length(), 2);
                    const auto force_dir = glm::normalize(body_position - other_position);
                    const auto force = force_dir * gravitational_constant_d * static_cast<double>(body_mass)
                                       * static_cast<double>(other_mass) / sqr_dist;
                    const auto acceleration = force / static_cast<double>(body_mass);

                    world.patch<kawe::Velocity3f>(
                        entity, [&acceleration](auto &vel) { vel.component += acceleration; });
                }

                // updating all positions.
                for (const auto &other : world.view<entt::tag<"CelestialBody"_hs>>()) {
                    // ! could break if id system change for an object.
                    // TODO: refactore this.
                    if (entity == other) continue;

                    auto &new_pos = world.patch<kawe::Position3f>(entity, [&world, &entity](auto &pos) {
                        pos.component += world.get<kawe::Velocity3f>(entity).component;
                    });

                    vertices.push_back(static_cast<float>(new_pos.component.x));
                    vertices.push_back(static_cast<float>(new_pos.component.y));
                    vertices.push_back(static_cast<float>(new_pos.component.z));
                }
            }

            kawe::Render::VBO<kawe::Render::VAO::Attribute::POSITION>::emplace(
                world, component.orbit_gizmo, vertices, 3);
            world.patch<kawe::Render::VAO>(component.orbit_gizmo, [](kawe::Render::VAO &vao) {
                vao.mode = kawe::Render::VAO::DisplayMode::LINES;
            });
            world.emplace_or_replace<kawe::FillColor>(component.orbit_gizmo, glm::vec4{1.0f, 1.0f, 1.0f, 1.0f});

            spdlog::warn("entity {}", entity);
            std::for_each(vertices.begin(), vertices.end(), [](tinyobj::real_t coord) { spdlog::warn(coord); });
            spdlog::warn("gizmo end.");
        });

        // reset celestial bodies initial position & velocities.
        for (const auto &entity : world.view<entt::tag<"CelestialBody"_hs>>()) {
            spdlog::warn(
                "initial_velocity: {}, {}, {}",
                initial_velocity[entity].x,
                initial_velocity[entity].y,
                initial_velocity[entity].z);

            spdlog::warn(
                "initial_position: {}, {}, {}",
                initial_position[entity].x,
                initial_position[entity].y,
                initial_position[entity].z);

            // updating velocity.
            world.patch<kawe::Velocity3f>(
                entity, [init_vel = initial_velocity[entity]](auto &vel) { vel.component = init_vel; });

            // updating position.
            world.patch<kawe::Position3f>(
                entity, [init_pos = initial_position[entity]](auto &pos) { pos.component = init_pos; });
        }
    }
};

} // namespace CelestialBody

} // namespace css
