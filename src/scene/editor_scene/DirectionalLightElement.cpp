#include "DirectionalLightElement.h"

#include <limits>

#include <glm/gtx/transform.hpp>
#include <glm/gtx/component_wise.hpp>

#include "rendering/imgui/ImGuiManager.h"
#include "scene/SceneContext.h"

namespace {
    glm::vec3 safe_normalize(glm::vec3 value, glm::vec3 fallback = glm::vec3{0.0f, -1.0f, 0.0f}) {
        float len = glm::length(value);
        if (len <= std::numeric_limits<float>::epsilon()) {
            return fallback;
        }
        return value / len;
    }

    glm::mat4 orientation_from_y_axis(glm::vec3 direction) {
        glm::vec3 up = safe_normalize(direction);

        glm::vec3 helper = std::abs(up.y) < 0.99f
            ? glm::vec3{0.0f, 1.0f, 0.0f}
            : glm::vec3{1.0f, 0.0f, 0.0f};

        glm::vec3 right = safe_normalize(glm::cross(helper, up), glm::vec3{1.0f, 0.0f, 0.0f});
        glm::vec3 forward = safe_normalize(glm::cross(right, up), glm::vec3{0.0f, 0.0f, 1.0f});

        glm::mat4 orientation{1.0f};
        orientation[0] = glm::vec4(right, 0.0f);
        orientation[1] = glm::vec4(up, 0.0f);
        orientation[2] = glm::vec4(forward, 0.0f);

        return orientation;
    }
}

std::unique_ptr<EditorScene::DirectionalLightElement> EditorScene::DirectionalLightElement::new_default(const SceneContext& scene_context, EditorScene::ElementRef parent) {
    glm::vec3 default_direction = glm::normalize(glm::vec3{-1.0f, -1.0f, -1.0f});
    glm::vec3 default_colour = glm::vec3{1.0f};

    auto light_element = std::make_unique<DirectionalLightElement>(
        parent,
        "New Directional Light",
        glm::vec3{0.0f, 2.0f, 0.0f},
        default_direction,
        DirectionalLight::create(
            default_direction,
            glm::vec4{default_colour, 1.0f}
        ),
        EmissiveEntityRenderer::Entity::create(
            scene_context.model_loader.load_from_file<EmissiveEntityRenderer::VertexData>("cube.obj"),
            EmissiveEntityRenderer::InstanceData{
                glm::mat4{}, // Set via update_instance_data()
                EmissiveEntityRenderer::EmissiveEntityMaterial{
                    glm::vec4{default_colour, 1.0f}
                }
            },
            EmissiveEntityRenderer::RenderData{
                scene_context.texture_loader.default_white_texture()
            }
        )
    );

    light_element->update_instance_data();
    return light_element;
}

std::unique_ptr<EditorScene::DirectionalLightElement> EditorScene::DirectionalLightElement::from_json(const SceneContext& scene_context, EditorScene::ElementRef parent, const json& j) {
    auto light_element = new_default(scene_context, parent);

    light_element->position = j["position"];
    light_element->direction = j["direction"];
    light_element->light->colour = j["colour"];
    light_element->visible = j["visible"];
    light_element->visual_scale = j["visual_scale"];

    light_element->update_instance_data();
    return light_element;
}

json EditorScene::DirectionalLightElement::into_json() const {
    return {
        {"position",     position},
        {"direction",    direction},
        {"colour",       light->colour},
        {"visible",      visible},
        {"visual_scale", visual_scale},
    };
}

void EditorScene::DirectionalLightElement::add_imgui_edit_section(MasterRenderScene& /*render_scene*/, const SceneContext& scene_context) {
    ImGui::Text("Directional Light");
    SceneElement::add_imgui_edit_section(*((MasterRenderScene*) nullptr), scene_context);

    ImGui::Text("Visual Position");
    bool updated = false;

    updated |= ImGui::DragFloat3("Position", &position[0], 0.01f);
    ImGui::DragDisableCursor(scene_context.window);

    ImGui::Spacing();
    ImGui::Text("Light Direction");

    updated |= ImGui::DragFloat3("Direction", &direction[0], 0.01f);
    ImGui::DragDisableCursor(scene_context.window);

    ImGui::Spacing();
    ImGui::Text("Light Properties");

    updated |= ImGui::ColorEdit3("Colour", &light->colour[0]);
    ImGui::DragDisableCursor(scene_context.window);

    updated |= ImGui::DragFloat("Intensity", &light->colour.a, 0.01f, 0.0f, FLT_MAX);
    ImGui::DragDisableCursor(scene_context.window);

    ImGui::Spacing();
    ImGui::Text("Visuals");

    updated |= ImGui::Checkbox("Show Visuals", &visible);
    updated |= ImGui::DragFloat("Visual Scale", &visual_scale, 0.01f, 0.0f, FLT_MAX);
    ImGui::DragDisableCursor(scene_context.window);

    if (updated) {
        update_instance_data();
    }
}

void EditorScene::DirectionalLightElement::update_instance_data() {
    glm::mat4 parent_transform{1.0f};
    if (!EditorScene::is_null(parent)) {
        parent_transform = (*parent)->transform;
    }

    glm::vec3 normalised_local_direction = safe_normalize(direction);
    glm::vec3 world_direction = safe_normalize(glm::mat3(parent_transform) * normalised_local_direction);

    light->direction = world_direction;

    glm::vec3 world_position = glm::vec3(parent_transform * glm::vec4(position, 1.0f));
    transform = glm::translate(world_position);

    if (visible) {
        glm::mat4 visual_translation = glm::translate(world_position + world_direction * (0.35f * visual_scale));
        glm::mat4 visual_orientation = orientation_from_y_axis(world_direction);
        glm::mat4 visual_scale_matrix = glm::scale(glm::vec3{0.06f * visual_scale, 0.7f * visual_scale, 0.06f * visual_scale});

        light_visual->instance_data.model_matrix =
            visual_translation *
            visual_orientation *
            visual_scale_matrix;
    } else {
        light_visual->instance_data.model_matrix =
            glm::scale(glm::vec3{std::numeric_limits<float>::infinity()}) *
            glm::translate(glm::vec3{std::numeric_limits<float>::infinity()});
    }

    glm::vec3 colour = glm::vec3(light->colour);
    float max_component = glm::compMax(colour);
    glm::vec3 normalised_colour = max_component <= std::numeric_limits<float>::epsilon()
        ? glm::vec3{0.0f}
        : colour / max_component;

    light_visual->instance_data.material.emission_tint =
        glm::vec4(normalised_colour, light_visual->instance_data.material.emission_tint.a);
}

const char* EditorScene::DirectionalLightElement::element_type_name() const {
    return ELEMENT_TYPE_NAME;
}