#pragma once

#include "Engine.hpp"

constexpr auto gravitational_constant = 0.000001;

using namespace entt::literals;

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
    std::size_t lifetime;
};

struct OrbitVizualiser {
    entt::entity &parent;
    std::size_t iterations;
    std::chrono::milliseconds refresh_rate;
    std::vector<entt::entity> lines;

    static auto
        emplace(
            entt::registry &world,
            entt::entity &entity,
            std::size_t iterations,
            std::chrono::milliseconds refresh_rate
    ) -> OrbitVizualiser &
    {
        OrbitVizualiser visualizer {
          .parent = entity,
          .iterations = iterations,
          .refresh_rate = refresh_rate,
          .lines = {}
        };

        visualizer.lines.resize(iterations);

        // initializing lines.
        for (auto &id : visualizer.lines)
            id = world.create();

        auto compute_n_iterations = [&world, &visualizer, &iterations]() -> void
        {
            for (std::size_t it = 0; it < iterations; ++it)
                for (const auto &other : world.view<entt::tag<"CelestialBody"_hs>>()) {
                    // ! could break if id system change for an object.
                    // TODO: refactore this.
                    if (visualizer.parent == other)
                        continue;

                    const auto body_position = world.get<kawe::Position3f>(visualizer.parent).component;
                    const auto body_mass = static_cast<double>(world.get<CelestialBody::MassF>(visualizer.parent).mass);
                    const auto other_position = world.get<kawe::Position3f>(other).component;
                    const auto other_mass = static_cast<double>(world.get<CelestialBody::MassF>(other).mass);

                    const auto sqr_dist = glm::pow((body_position - other_position).length(), 2);
                    const auto force_dir = glm::normalize(body_position - other_position);
                    const auto force = force_dir * gravitational_constant * body_mass * other_mass / sqr_dist;
                    [[maybe_unused]] const auto acceleration = force / body_mass;

                    // updating the current body's velocity.
                    // visualizer.velocity.component += acceleration;
                }
        };

        // kawe::Clock::emplace(world, entity, refresh_rate, compute_n_iterations);

        // world.ctx<entt::dispatcher *>()->sink<kawe::TimeElapsed>().connect<&css::OrbitVizualiser::on_update_orbit_visualization>(visualizer);
        // world.on_update<OrbitVizualiser>().connect<compute_n_iterations>();

        return world.emplace_or_replace<OrbitVizualiser>(entity, visualizer);
    }
};

}

}