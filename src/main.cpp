#include "Universe.hpp"
#include "component.hpp"
#include "Player.hpp"

#include <numbers>

int main()
{

    kawe::Engine engine{};

    std::shared_ptr<css::Player> player;
    std::shared_ptr<css::Universe> universe;

    entt::registry *my_world;

    engine.on_create = [&my_world, &universe, &player](entt::registry &world) {
        my_world = &world;

        universe = std::make_shared<css::Universe>(world);
        world.ctx<entt::dispatcher *>()->sink<kawe::TimeElapsed>().connect<&css::Universe::on_update_bodies>(universe.get());

        // const auto map = world.create();
        // world.emplace<kawe::Parent>(cube, map);
        // world.get_or_emplace<kawe::Children>(map).component.push_back(cube);

        const auto player_id = world.create();
        world.emplace<kawe::Position3f>(player_id, glm::vec3{0, 10, 0});
        world.emplace<kawe::Collider>(player_id);
        world.emplace<kawe::Name>(player_id, "Player");

        player = std::make_shared<css::Player>(player_id, world);
        world.ctx<entt::dispatcher *>()->sink<kawe::Pressed<kawe::Key>>().connect<&css::Player::on_key_pressed>(
            player.get());

        //#ifdef TEST_THE_MESH_LOADER
        [[ maybe_unused ]] auto earth = universe->add_body(
            world,
            "Earth",
            "./asset/models/Earth.obj",
            "./asset/textures/Planet_4K.jpg",
            glm::vec3(0.f),
            glm::vec3(0.f),
            0.01f,
            100.f
        );

        [[ maybe_unused ]] auto asteroid = universe->add_body(
            world,
            "Asteroid",
            "./asset/models/Asteroid_Small_6X.obj",
            "./asset/textures/Aster_Small_1_Color.png",
            glm::vec3(3.f, 0.f, 0.f),
            glm::vec3(0.f, .1f, 0.f),
            1
        );
    };

    engine.on_imgui = [&my_world]() {
        ImGui::Begin("Cute Solar System - Control Panel");
        const auto &in = my_world->ctx<kawe::State *>()->clear_color;
        float temp[4] = {in.r, in.g, in.b, in.a};

        if (ImGui::ColorEdit4("clear color", temp))
            my_world->ctx<kawe::State *>()->clear_color = {temp[0], temp[1], temp[2], temp[3]};

        ImGui::End();
    };

    engine.start();
}
