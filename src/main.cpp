#include "Universe.hpp"
#include "component.hpp"
#include "Player.hpp"

#include "primitives/Line.hpp"
#include "primitives/Sphere.hpp"

int main()
{
    kawe::Engine engine{};

    std::shared_ptr<css::Player> player;
    std::shared_ptr<css::Universe> universe;

    entt::registry *my_world;

    engine.on_create = [&my_world, &universe, &player](entt::registry &world) {
        my_world = &world;

        universe = std::make_shared<css::Universe>(world);
        world.ctx<entt::dispatcher *>()->sink<kawe::event::TimeElapsed>().connect<&css::Universe::on_update_bodies>(
            universe.get());

        const auto player_id = world.create();
        world.emplace<kawe::Position3f>(player_id, glm::vec3{0, 10, 0});
        world.emplace<kawe::Collider>(player_id);
        world.emplace<kawe::Name>(player_id, "Player");

        player = std::make_shared<css::Player>(player_id, world);
        world.ctx<entt::dispatcher *>()
            ->sink<kawe::event::Pressed<kawe::event::Key>>()
            .connect<&css::Player::on_key_pressed>(player.get());

        //#ifdef TEST_THE_MESH_LOADER
        [[maybe_unused]] auto earth = universe->add_body(
            "Earth",
            "./asset/models/Earth.obj",
            "./asset/textures/Stars_1K.jpg",
            glm::vec3(0.f),
            glm::vec3(0.f),
            0.01f,
            1000.f);

        [[maybe_unused]] auto asteroid = universe->add_body(
            "Asteroid",
            "./asset/models/Earth.obj",
            "./asset/textures/Planet_4K.jpg",
            glm::vec3(3.f, 0.f, 0.f),
            glm::vec3(.2f, 0.f, .2f),
            0.005f,
            1.f);

        css::CelestialBody::OrbitVizualiser::compute_n_iterations(*my_world, 100);

        // kawe::Clock::emplace(*my_world, world.create(), 1000ms, [&my_world]() {
        //     // running orbit simulation.
        // });
    };

    engine.on_imgui = [&my_world]() {
        ImGui::Begin("Cute Solar System - Control Panel");
        const auto &in = my_world->ctx<kawe::State *>()->clear_color;
        float temp[4] = {in.r, in.g, in.b, in.a};

        if (ImGui::ColorEdit4("clear color", temp))
            my_world->ctx<kawe::State *>()->clear_color = {temp[0], temp[1], temp[2], temp[3]};

        ImGui::End();
    };

    // engine.render_internal_gui = false;

    engine.start();
}
