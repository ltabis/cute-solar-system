#include "celestial-bodies/CelestialBody.hpp"
#include "component.hpp"
#include "Player.hpp"

#include <numbers>

int main()
{
    kawe::Engine engine{};

    std::shared_ptr<css::Player> player;

    entt::registry *my_world;

    engine.on_create = [&my_world, &player](entt::registry &world) {
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
        const auto model = world.create();

        kawe::Mesh::emplace(world, model, "./asset/models/Earth.obj");
        kawe::Texture2D::emplace(
            world, model, *world.ctx<kawe::ResourceLoader *>(), "./asset/textures/Planet_4K.jpg");
        const auto vbo = world.get<kawe::Render::VBO<kawe::Render::VAO::Attribute::POSITION>>(model);
        const auto index_size = vbo.vertices.size() / vbo.stride_size;
        kawe::Render::VBO<kawe::Render::VAO::Attribute::COLOR>::emplace(
            world, model, std::vector<float>(index_size * 4, 1.0f), 4);
        world.emplace<kawe::Scale3f>(model, glm::vec3(0.01f, 0.01f, 0.01f));
        world.emplace<kawe::Position3f>(model, glm::vec3(3.0f, 2.0f, 0.0f));
        //#endif
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
