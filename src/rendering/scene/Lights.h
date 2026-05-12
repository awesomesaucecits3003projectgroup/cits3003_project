#ifndef LIGHTS_H
#define LIGHTS_H

#include <memory>
#include <vector>
#include <unordered_set>

#include <glm/glm.hpp>

/// A representation of a PointLight render scene element
struct PointLight {
    PointLight() = default;

    PointLight(const glm::vec3& position, const glm::vec4& colour, const glm::vec3& attenuation = glm::vec3{1.0f, 0.35f, 0.16f}) :
        position(position), colour(colour), attenuation(attenuation) {}

    static PointLight off() {
        return {glm::vec3{}, glm::vec4{}, glm::vec3{1.0f, 0.35f, 0.16f}};
    }

    static std::shared_ptr<PointLight> create(const glm::vec3& position, const glm::vec4& colour, const glm::vec3& attenuation = glm::vec3{1.0f, 0.35f, 0.16f}) {
        return std::make_shared<PointLight>(position, colour, attenuation);
    }

    glm::vec3 position{};
    // Alpha components are just used to store a scalar that is applied before passing to the GPU
    glm::vec4 colour{};

    // x = constant, y = linear, z = quadratic attenuation
    glm::vec3 attenuation{1.0f, 0.35f, 0.16f};

    // On GPU format
    // alignas used to conform to std140 for direct binary usage with glsl
    struct Data {
        alignas(16) glm::vec3 position;
        alignas(16) glm::vec3 colour;
        alignas(16) glm::vec3 attenuation;
    };
};

/// A representation of a DirectionalLight render scene element.
/// Direction is the direction the light travels through the world.
struct DirectionalLight {
    DirectionalLight() = default;

    DirectionalLight(const glm::vec3& direction, const glm::vec4& colour) :
        direction(direction), colour(colour) {}

    static DirectionalLight off() {
        return {glm::vec3{0.0f, -1.0f, 0.0f}, glm::vec4{}};
    }

    static std::shared_ptr<DirectionalLight> create(const glm::vec3& direction, const glm::vec4& colour) {
        return std::make_shared<DirectionalLight>(direction, colour);
    }

    // Direction the light travels in world space
    glm::vec3 direction{0.0f, -1.0f, 0.0f};

    // Alpha component stores intensity, not transparency
    glm::vec4 colour{1.0f};

    // On GPU format
    // alignas used to conform to std140 for direct binary usage with glsl
    struct Data {
        alignas(16) glm::vec3 direction;
        alignas(16) glm::vec3 colour;
    };
};

/// A collection of each light type, with helpers that allow for selecting a subset of
/// those lights on a proximity basis, since processing an unbounded number of lights on the GPU is bad idea.
struct LightScene {
    std::unordered_set<std::shared_ptr<PointLight>> point_lights;
    std::unordered_set<std::shared_ptr<DirectionalLight>> directional_lights;

    /// Will return up to `max_count` nearest point lights to `target`.
    std::vector<PointLight> get_nearest_point_lights(glm::vec3 target, size_t max_count, size_t min_count = 0) const;

    /// Will return up to `max_count` directional lights.
    std::vector<DirectionalLight> get_directional_lights(size_t max_count, size_t min_count = 0) const;

private:
    template<typename Light>
    static std::vector<Light> get_nearest_lights(const std::unordered_set<std::shared_ptr<Light>>& lights, glm::vec3 target, size_t max_count, size_t min_count = 0);
};

#endif //LIGHTS_H
