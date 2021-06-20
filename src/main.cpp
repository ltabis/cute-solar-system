#include "Universe.hpp"
#include "component.hpp"

#include "primitives/Line.hpp"
#include "primitives/Sphere.hpp"

#include "widgets/ComponentInspector.hpp"

using CSSComponent = std::variant<
    std::monostate,

    kawe::Position3f,
    kawe::Scale3f,
    kawe::Rotation3f,

    css::CelestialBody::MassF,
    css::CelestialBody::SizeF,
    css::CelestialBody::LifeTime

    >;

struct CuteSolarSystem {
    entt::registry *my_world = nullptr;
    std::shared_ptr<css::Universe> universe;

    kawe::ComponentInspector inspector{};

    auto on_create(entt::registry &world) -> void
    {
        my_world = &world;
        on_create_safe();
    }

    auto on_imgui() -> void
    {
        ImGui::Begin("Cute Solar System - Control Panel");
        const auto &in = my_world->ctx<kawe::State *>()->clear_color;
        float temp[4] = {in.r, in.g, in.b, in.a};

        if (ImGui::ColorEdit4("clear color", temp))
            my_world->ctx<kawe::State *>()->clear_color = {temp[0], temp[1], temp[2], temp[3]};

        inspector.draw<CSSComponent>(*my_world);

        ImGui::End();
    }

private:
    auto on_object_pick(entt::registry &reg, entt::entity picked) -> void
    {
        for (const auto &cam : reg.view<kawe::CameraData, kawe::Children>()) {
            for (const auto &i : reg.get<kawe::Children>(cam).component) {
                if (reg.get<kawe::Name>(i).component == fmt::format("<kawe:camera_target#{}>", cam)) {
                    reg.patch<kawe::Position3f>(i, [&reg, &picked](auto &pos) {
                        pos.component = reg.get<kawe::Position3f>(picked).component;
                    });
                }
            }
        }
    }

    auto on_create_safe() -> void
    {
        assert(my_world != nullptr);
        my_world->on_update<kawe::Pickable>().connect<&CuteSolarSystem::on_object_pick>(*this);

        universe = std::make_shared<css::Universe>(*my_world);
        my_world->ctx<entt::dispatcher *>()->sink<kawe::event::TimeElapsed>().connect<&css::Universe::on_update_bodies>(
            universe.get());

        create_line(*my_world, glm::vec3(0.f, -50.f, 0.f), glm::vec3(0.f, 50.f, 0.f));

        [[maybe_unused]] auto earth = universe->add_body(
            "Earth",
            "./asset/models/Earth.obj",
            "./asset/textures/Stars_1K.jpg",
            glm::vec3(0.f),
            glm::vec3(0.f),
            0.01f,
            100.f);

        [[maybe_unused]] auto asteroid = universe->add_body(
            "Asteroid",
            "./asset/models/Asteroid_Small_6X.obj",
            "./asset/textures/Aster_Small_1_Color.png",
            glm::vec3(3.f, 0.f, 0.f),
            glm::vec3(0.f, .1f, 0.f),
            1);

        my_world->ctx<kawe::State *>()->clear_color = {0.2, 0.2, 0.2, 1.0};
    }
};

int main()
{
    kawe::Engine engine{};
    CuteSolarSystem app{};

    engine.on_create = [&app](entt::registry &world) { app.on_create(world); };
    engine.on_imgui = [&app] { app.on_imgui(); };

    engine.start();
}
