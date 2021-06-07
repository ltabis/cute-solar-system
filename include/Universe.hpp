#pragma once

#include "celestial-bodies/CelestialBodyComponents.hpp"

namespace css
{

class Universe {
public:
    auto add_body(
        entt::registry &world,
        const std::string &name,
        const std::string &model_path,
        const std::string &texture_path = "",
        const glm::vec3 &position = glm::vec3(0.f),
        const float size = 1.f
    ) -> entt::entity {
        const auto body_model = world.create();

        kawe::Mesh::emplace(world, body_model, model_path);

        // loading texture if one is provided
        if (!texture_path.empty())
            kawe::Texture2D::emplace(
                world, body_model, *world.ctx<kawe::ResourceLoader *>(), "./asset/textures/Planet_4K.jpg");

        const auto vbo = world.get<kawe::Render::VBO<kawe::Render::VAO::Attribute::POSITION>>(body_model);
        const auto index_size = vbo.vertices.size() / vbo.stride_size;
        kawe::Render::VBO<kawe::Render::VAO::Attribute::COLOR>::emplace(
            world, body_model, std::vector<float>(index_size * 4, 1.0f), 4);
        world.emplace<CelestialBody::MassF>(body_model, 1.f);
        world.emplace<CelestialBody::SizeF>(body_model, size);
        world.emplace<kawe::Name>(body_model, name);
        world.emplace<kawe::Scale3f>(body_model, glm::vec3(size));
        world.emplace<kawe::Position3f>(body_model, position);

        m_Bodies.push_back(body_model);

        return body_model;
   }
private:
    std::vector<entt::entity> m_Bodies;
};

}