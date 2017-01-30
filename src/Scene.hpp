#ifndef DMP_SCENE_HPP
#define DMP_SCENE_HPP

#include <memory>

#include "Scene/Types.hpp"
#include "Scene/Object.hpp"
#include "Scene/Graph.hpp"
#include "Scene/Camera.hpp"
#include "Scene/Skybox.hpp"
#include "Renderer/UniformBuffer.hpp"
#include "Renderer/Texture.hpp"
#include "Scene/Model.hpp"
#include "Scene/Model/Skeleton.hpp"
#include "CommandLine.hpp"

namespace dmp
{
  struct Scene
  {
    std::vector<Material> materials;
    std::unique_ptr<UniformBuffer> materialConstants;
    std::vector<Texture> textures;
    std::vector<Light> lights;
    std::vector<Camera> cameras;
    std::vector<Object *> objects;
    std::unique_ptr<UniformBuffer> objectConstants;
    std::unique_ptr<Branch> graph;
    std::unique_ptr<Skybox> skybox;
    Model * model;

    void build(std::function<bool(glm::mat4 &, float)> cameraFn,
               std::function<bool(glm::mat4 &, float)> lightFn,
               const CommandLine & cmd);
    void update(float deltaT);
    void free();
  };
}

#endif
