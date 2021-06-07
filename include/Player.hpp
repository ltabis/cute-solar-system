#pragma once

#include "Engine.hpp"

namespace css {

struct Player {
    entt::entity entity;
    entt::registry &world;

    auto on_key_pressed(const kawe::Pressed<kawe::Key> &key)
    {
        std::optional<double> x{};
        std::optional<double> y{};
        std::optional<double> z{};

        switch (key.source.keycode) {
        case kawe::Key::Code::KEY_E: x = 0.5; break;
        case kawe::Key::Code::KEY_S: z = 0.5; break;
        case kawe::Key::Code::KEY_F: z = -0.5; break;
        case kawe::Key::Code::KEY_D: x = -0.5; break;
        case kawe::Key::Code::KEY_SPACE: y = 3.0; break;
        default: break;
        }
        if (x.has_value() || y.has_value() || z.has_value()) {
            const auto default_v =
                kawe::Velocity3f{glm::dvec3{x.value_or(0.0), y.value_or(0.0), z.value_or(0.0)}};
            const auto vel = world.try_get<kawe::Velocity3f>(entity);

            if (vel == nullptr) {
                world.emplace_or_replace<kawe::Velocity3f>(entity, default_v);
            } else {
                world.emplace_or_replace<kawe::Velocity3f>(
                    entity,
                    glm::dvec3{
                        x.has_value() ? x.value() : vel->component.x,
                        y.has_value() ? y.value() : vel->component.y,
                        z.has_value() ? z.value() : vel->component.z,
                    });
            }
        }
    }
};

} // namespace css
