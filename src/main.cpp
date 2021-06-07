#include "Universe.hpp"
#include "component.hpp"
#include "Player.hpp"

#include <numbers>

int main()
{
    css::Universe universe{};
    kawe::Engine engine{};

    std::shared_ptr<css::Player> player;

    entt::registry *my_world;

    engine.on_create = [&universe, &my_world, &player](entt::registry &world) {
        my_world = &world;

        // const auto map = world.create();
        // world.emplace<kawe::Parent>(cube, map);
        // world.get_or_emplace<kawe::Children>(map).component.push_back(cube);

        const auto player_id = world.create();
        world.emplace<kawe::Position3f>(player_id, glm::vec3{0, 1, 0});
        world.emplace<kawe::Collider>(player_id);
        world.emplace<kawe::Name>(player_id, "Player");

        player = std::make_shared<css::Player>(player_id, world);
        world.ctx<entt::dispatcher *>()->sink<kawe::Pressed<kawe::Key>>().connect<&css::Player::on_key_pressed>(
            player.get());

        //#ifdef TEST_THE_MESH_LOADER
        auto earth = universe.add_body(
            world,
            "Earth",
            "./asset/models/Earth.obj",
            "./asset/textures/Planet_4K.jpg",
            glm::vec3(0.f),
            0.01f
        );

        auto asteroid = universe.add_body(
            world,
            "Asteroid",
            "./asset/models/Asteroid_Small_6X.obj",
            "./asset/textures/Aster_Small_1_Color.png",
            glm::vec3(0.f),
            0.01f
        );

        spdlog::info("bodies generated: {} & {}", earth, asteroid);
    };

    engine.on_imgui = [&my_world]() {
        ImGui::Begin("Cute Solar System - Control Panel");
        const auto &in = my_world->ctx<kawe::State *>()->clear_color;
        float temp[4] = {in.r, in.g, in.b, in.a};
        if (ImGui::ColorEdit4("clear color", temp)) {
            my_world->ctx<kawe::State *>()->clear_color = {temp[0], temp[1], temp[2], temp[3]};
        }

        ImGui::End();
    };

    engine.start();
}
