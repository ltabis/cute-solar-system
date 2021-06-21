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

    enum class CelestialObjectType {
        // ASTEROID,
        PLANET,
        STAR,
    };

    auto create_object(CelestialObjectType type_selected)
    {
        const auto get_texture = [](CelestialObjectType type) {
            constexpr auto array_texture_planet = std::to_array({
                "./asset/textures/Planet_4K.jpg",
                "./asset/textures/00A-BG-PT2-Venus.jpg",
                "./asset/textures/46k-venus-color-map-3d-model.jpg",
                "./asset/textures/042667fb183134f3210a110971376c351cb21fa5.jpg",
                "./asset/textures/d7pxfv4-6a44b369-8c61-41c8-937b-432f356e5ec7.png",
                "./asset/textures/grass-green-earth-texture-wallpaper-preview.jpg",
                "./asset/textures/images (1).jpeg",
                "./asset/textures/images.jpeg",
                "./asset/textures/Sedna.png",
                "./asset/textures/unnamed.jpg",
            });

            switch (type) {
            // case CelestialObjectType::ASTEROID: return "./asset/textures/Aster_Small_1_Color.png";
            case CelestialObjectType::PLANET:
                return array_texture_planet[std::rand() % array_texture_planet.size()];
            case CelestialObjectType::STAR: return "./asset/textures/8k_sun.jpg";
            default: return "";
            }
        };

        const auto get_model = [](CelestialObjectType type) {
            switch (type) {
            // case CelestialObjectType::ASTEROID: return "./asset/models/Asteroid_Small_6X.obj";
            case CelestialObjectType::PLANET: return "./asset/models/Earth.obj";
            case CelestialObjectType::STAR: return "./asset/models/Earth.obj";
            default: return "";
            }
        };

        const auto get_mass = [](CelestialObjectType type) {
            switch (type) {
            // case CelestialObjectType::ASTEROID: return 0.01f;
            case CelestialObjectType::PLANET: return 1.0f;
            case CelestialObjectType::STAR: return 1000.0f;
            default: return 1.0f;
            }
        };

        const auto get_size = [](CelestialObjectType type) {
            switch (type) {
            // case CelestialObjectType::ASTEROID: return 1.0f;
            case CelestialObjectType::PLANET: return 0.015f;
            case CelestialObjectType::STAR: return 0.1f;
            default: return 1.0f;
            }
        };

        const auto object = universe->add_body(
            fmt::format("{} n°{}", magic_enum::enum_name(type_selected).data(), std::rand() % 1000),
            get_model(type_selected),
            get_texture(type_selected),
            glm::vec3(
                static_cast<double>(std::rand() % 50) - 25.0,
                static_cast<double>(std::rand() % 50) - 25.0,
                static_cast<double>(std::rand() % 50) - 25.0),
            type_selected == CelestialObjectType::STAR ? glm::vec3(0.0f)
                                                       : glm::vec3(
                                                           static_cast<double>(std::rand() % 10) / 10.0f - 0.5,
                                                           static_cast<double>(std::rand() % 10) / 10.0f - 0.5,
                                                           static_cast<double>(std::rand() % 10) / 10.0f - 0.5),
            get_size(type_selected) * (std::rand() % 10 / 10.0 - 0.5),
            get_mass(type_selected));
        if (type_selected == CelestialObjectType::STAR) {
            my_world->emplace<kawe::PointLight>(object);
            const auto &shaders = my_world->ctx<kawe::State *>()->shaders;
            auto found = std::find_if(
                shaders.begin(), shaders.end(), [](auto &i) { return i->getName() == "texture_2D_emissif"; });
            my_world->get<kawe::Render::VAO>(object).shader_program = found->get();
        }
    }

    auto on_imgui() -> void
    {
        if (ImGui::Begin("Cute Solar System - Control Panel")) {
            const auto &in = my_world->ctx<kawe::State *>()->clear_color;
            float temp[4] = {in.r, in.g, in.b, in.a};

            if (ImGui::ColorEdit4("clear color", temp))
                my_world->ctx<kawe::State *>()->clear_color = {temp[0], temp[1], temp[2], temp[3]};

            ImGui::Separator();

            static bool show_form = true;
            if (show_form) {
                if (ImGui::Button("Create New Celestial Object")) { show_form = false; }
            } else {
                ImGui::Text("Select the type of object");
                static int type_selected = 0;
                for (const auto &type : magic_enum::enum_values<CelestialObjectType>()) {
                    ImGui::RadioButton(magic_enum::enum_name(type).data(), &type_selected, static_cast<int>(type));
                }

                if (ImGui::Button("Confirm")) {
                    show_form = true;
                    create_object(static_cast<CelestialObjectType>(type_selected));
                }

                ImGui::SameLine();
                if (ImGui::Button("Cancel")) { show_form = true; }
            }

            ImGui::Separator();

            for (const auto &i : my_world->view<entt::tag<"CelestialBody"_hs>>()) {
                kawe::ImGuiHelper::Text("{}", my_world->get<kawe::Name>(i).component);
            }
        }
        ImGui::End();
        inspector.draw<CSSComponent>(*my_world);
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

        create_object(CelestialObjectType::STAR);
        create_object(CelestialObjectType::PLANET);

        // create_line(*my_world, glm::vec3(0.f, -50.f, 0.f), glm::vec3(0.f, 50.f, 0.f));

        /*
                universe->add_body(
                    "Sun", "./asset/models/Earth.obj", "./asset/textures/8k_sun.jpg", glm::vec3(0.f), glm::vec3(0.f), 0.1f, 1000.f);

                universe->add_body(
                    "Asteroid",
                    "./asset/models/Earth.obj",
                    "./asset/textures/Planet_4K.jpg",
                    glm::vec3(3.f, 0.f, 0.f),
                    glm::vec3(.2f, 0.f, .2f),
                    0.005f,
                    1.f);
        */

        my_world->ctx<kawe::State *>()->clear_color = {0.2, 0.2, 0.2, 1.0};
        css::CelestialBody::OrbitVizualiser::compute_n_iterations(*my_world, 100);
    }
};

int main()
{
    kawe::Engine engine{};
    CuteSolarSystem app{};

    engine.on_create = [&app](entt::registry &world) {
        std::srand(static_cast<std::uint32_t>(std::time(nullptr)));
        app.on_create(world);
    };
    engine.on_imgui = [&app] { app.on_imgui(); };

    // engine.render_internal_gui = false;

    engine.start();
}
