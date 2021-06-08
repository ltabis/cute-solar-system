#include "Engine.hpp"
#include "component.hpp"

#include <numbers>

namespace data {

// clang-format off
constexpr auto cube_positions = std::to_array({
    -0.5f, 0.5f,  -0.5f, // Point A 0
    -0.5f, 0.5f,  0.5f,  // Point B 1
    0.5f,  0.5f,  -0.5f, // Point C 2
    0.5f,  0.5f,  0.5f,  // Point D 3
    -0.5f, -0.5f, -0.5f, // Point E 4
    -0.5f, -0.5f, 0.5f,  // Point F 5
    0.5f,  -0.5f, -0.5f, // Point G 6
    0.5f,  -0.5f, 0.5f,  // Point H 7
});

constexpr auto cube_colors = std::to_array({
    0.0f, 0.0f, 0.0f, 1.0f, // Point A 0
    0.0f, 0.0f, 1.0f, 1.0f, // Point B 1
    0.0f, 1.0f, 0.0f, 1.0f, // Point C 2
    0.0f, 1.0f, 1.0f, 1.0f, // Point D 3
    1.0f, 0.0f, 0.0f, 1.0f, // Point E 4
    1.0f, 0.0f, 1.0f, 1.0f, // Point F 5
    1.0f, 1.0f, 0.0f, 1.0f, // Point G 6
    1.0f, 1.0f, 1.0f, 1.0f // Point H 7
});
constexpr auto cube_indices = std::to_array({
    /*Above ABC,BCD*/
    0u, 1u, 2u, 1u, 2u, 3u,
    /*Following EFG,FGH*/
    4u, 5u, 6u, 5u, 6u, 7u,
    /*Left ABF,AEF*/
    0u, 1u, 5u, 0u, 4u, 5u,
    /*Right side CDH,CGH*/
    2u, 3u, 7u, 2u, 6u, 7u,
    /*ACG,AEG*/
    0u, 2u, 6u, 0u, 4u, 6u,
    /*Behind BFH,BDH*/
    1u, 5u, 7u, 1u, 3u, 7u
});
// clang-format on

} // namespace data

void SolidSphere(entt::registry &world, float radius, unsigned int rings, unsigned int sectors)
{
    std::vector<GLfloat> sphere_vertices;
    std::vector<GLfloat> sphere_normals;
    std::vector<GLfloat> sphere_texcoords;
    std::vector<std::uint32_t> sphere_indices;

    const auto R = 1.0f / static_cast<float>(rings - 1);
    const auto S = 1.0f / static_cast<float>(sectors - 1);

    sphere_vertices.resize(rings * sectors * 3);
    sphere_normals.resize(rings * sectors * 3);
    sphere_texcoords.resize(rings * sectors * 2);
    auto v = sphere_vertices.begin();
    auto n = sphere_normals.begin();
    auto t = sphere_texcoords.begin();
    for (auto r = 0u; r < rings; r++)
        for (auto s = 0u; s < sectors; s++) {
            const auto y = std::sin(
                -std::numbers::pi_v<float> / 2.0f + std::numbers::pi_v<float> * static_cast<float>(r) * R);
            const auto x = std::cos(2.0f * std::numbers::pi_v<float> * static_cast<float>(s) * S)
                           * std::sin(std::numbers::pi_v<float> * static_cast<float>(r) * R);
            const auto z = std::sin(2.0f * std::numbers::pi_v<float> * static_cast<float>(s) * S)
                           * std::sin(std::numbers::pi_v<float> * static_cast<float>(r) * R);

            *t++ = static_cast<float>(s) * S;
            *t++ = static_cast<float>(r) * R;

            *v++ = x * radius;
            *v++ = y * radius;
            *v++ = z * radius;

            *n++ = x;
            *n++ = y;
            *n++ = z;
        }

    sphere_indices.resize(rings * sectors * 4);
    auto i = sphere_indices.begin();
    for (auto r = 0u; r < rings; r++)
        for (auto s = 0u; s < sectors; s++) {
            *i++ = r * sectors + (s);
            *i++ = r * sectors + (s + 1);
            *i++ = (r + 1) * sectors + (s + 1);
            *i++ = (r + 1) * sectors + (s);
        }

    const auto e = world.create();

    kawe::Render::VBO<kawe::Render::VAO::Attribute::POSITION>::emplace(world, e, sphere_vertices, 3);
    kawe::Render::EBO::emplace(world, e, sphere_indices);
    world.get<kawe::Render::VAO>(e).mode = kawe::Render::VAO::DisplayMode::TRIANGLE_STRIP_ADJACENCY;
    world.emplace_or_replace<kawe::Position3f>(e, glm::vec3{0.0f, 2.0f, 3.0f});
}

int main()
{
    kawe::Engine engine{};

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

    std::shared_ptr<Player> player;

    entt::registry *my_world;

    engine.on_create = [&my_world, &player](entt::registry &world) {
        my_world = &world;

        const auto map = world.create();
        for (int y = 0; y != 5; y++) {
            for (int x = 0; x != 5; x++) {
                const auto cube = world.create();
                world.emplace<kawe::Position3f>(cube, glm::vec3{x, 0, y});
                world.emplace<kawe::Scale3f>(cube, glm::vec3{0.5, 0.5, 0.5});
                if ((x & 1) || (y & 1)) world.emplace<kawe::Collider>(cube);
                kawe::Render::VBO<kawe::Render::VAO::Attribute::POSITION>::emplace(
                    world, cube, data::cube_positions, 3);
                kawe::Render::VBO<kawe::Render::VAO::Attribute::COLOR>::emplace(world, cube, data::cube_colors, 4);
                kawe::Render::EBO::emplace(world, cube, data::cube_indices);
                world.emplace<kawe::Parent>(cube, map);
                world.get_or_emplace<kawe::Children>(map).component.push_back(cube);
            }
        }

        const auto e = world.create();
        kawe::Render::VBO<kawe::Render::VAO::Attribute::POSITION>::emplace(world, e, data::cube_positions, 3);
        kawe::Render::VBO<kawe::Render::VAO::Attribute::COLOR>::emplace(world, e, data::cube_colors, 4);
        kawe::Render::EBO::emplace(world, e, data::cube_indices);
        world.emplace<kawe::Position3f>(e, glm::vec3{0, 1, 0});
        world.emplace<kawe::Collider>(e);
        world.emplace<kawe::Name>(e, "Player");


        player = std::make_shared<Player>(e, world);
        world.ctx<entt::dispatcher *>()->sink<kawe::Pressed<kawe::Key>>().connect<&Player::on_key_pressed>(
            player.get());

        SolidSphere(world, 1, 20, 20);

        //#ifdef TEST_THE_MESH_LOADER
        const auto model = world.create();

        kawe::Mesh::emplace(world, model, "./asset/models/viking_room.obj");
        kawe::Texture2D::emplace(
            world, model, *world.ctx<kawe::ResourceLoader *>(), "./asset/textures/viking_room.png");
        world.emplace<kawe::FillColor>(model, glm::vec4{1.0f, 1.0f, 1.0f, 1.0f});
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
