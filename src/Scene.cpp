#include "Scene.hpp"

#include "Scene/Object.hpp"
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include "config.hpp"

void dmp::Scene::build(std::function<bool(glm::mat4 &, float)> cameraFn,
                       std::function<bool(glm::mat4 &, float)> lightFn,
                       const CommandLine & c)
{
  graph = std::make_unique<Branch>();

  model = graph->insert(Model(c, objects,
                              1, 0));

  Object::sortByMaterial(objects);

  materials.push_back( // Ruby = 0
    {
      {
        0.1745f, 0.01175f, 0.01175f, 1.0f
      },
      {
        0.61424f, 0.04136f, 0.04136f, 1.0f
      },
      {
        0.727811f, 0.626959f, 0.626959f, 1.0f
      },
      0.6f
    });
  materials.push_back( // Pearl = 1
    {
      {
        0.25f, 0.20725f, 0.20725f, 1.0f
      },
      {
        1.0f, 0.829f, 0.829f, 1.0f
      },
      {
        0.296648, 0.296648, 0.296648, 1.0f
      },
      0.6f
    });

  std::string tex = model->askTexturePath();
  textures.emplace_back(tex);

  materialConstants =
    std::make_unique<UniformBuffer>(materials.size(),
                                    Material::std140Size());

  for (size_t i = 0; i < materials.size(); ++i)
    {
      materialConstants->update(i, materials[i]);
    }

  lights.push_back({
       {0.05f, 1.0f, 0.05f, 1.0f},
       {0.0f, 0.9f, 1.0f, 0.0f},
       glm::mat4()
     });
   lights.push_back({
       {1.0f, 0.05f, 0.05f, 1.0f},
       {0.0f, 0.0f, 1.0f, 0.0f},
       glm::mat4()
     });
   lights.push_back({
       {0.05f, 0.05f, 1.0f, 1.0f},
       {0.0f, -0.9f, 1.0f, 0.0f},
       glm::mat4()
     });
   lights.push_back({
       {0.3f, 0.3f, 0.3f, 1.0f},
       {-1.0f, 0.0f, 0.0f, 0.0f},
       glm::mat4()
     });


   auto lightRot = graph->transform(lightFn);
   auto lightGroup = lightRot->branch();
   auto redLightRotation = glm::rotate(glm::mat4(),
                                       glm::pi<float>() / 2.0f,
                                       glm::vec3(0.0f, 1.0f, 0.0f));
   auto redLight = lightGroup->transform(redLightRotation);
   auto greenLightRotation = glm::rotate(glm::mat4(),
                                         (7.0f * glm::pi<float>()) / 6.0f,
                                         glm::vec3(0.0f, 1.0f, 0.0f));
   auto greenLight = lightGroup->transform(greenLightRotation);
   auto blueLightRotation = glm::rotate(glm::mat4(),
                                        (11.0f * glm::pi<float>()) / 6.0f,
                                        glm::vec3(0.0f, 1.0f, 0.0f));
   auto blueLight = lightGroup->transform(blueLightRotation);
   auto cam = graph->transform(cameraFn);
   redLight->insert(lights[1]);
   greenLight->insert(lights[0]);
   blueLight->insert(lights[2]);
   graph->insert(lights[3]);

   cameras.emplace_back();
   graph->insert(cameras[0].focus());
   cam->insert(cameras[0].pos());

   objectConstants
     = std::make_unique<UniformBuffer>(objects.size(),
                                       ObjectConstants::std140Size());

  std::vector<const char *> sb;
  for (size_t i = 0; i < 6; ++i) sb.push_back(skyBox[i]);

  skybox = std::make_unique<Skybox>(sb);

  graph->update(0.0f, glm::mat4(), true);
}

void dmp::Scene::update(float deltaT)
{
  expect("Object constant buffer not null",
         objectConstants);

  graph->update(deltaT);

  for (size_t i = 0; i < objects.size(); ++i)
    {
      if (objects[i]->isDirty())
        {
          objectConstants->update(i, objects[i]->getObjectConstants());
          objects[i]->setClean();
        }
    }

  for (auto & curr : cameras)
    {
      curr.update();
    }
}

void dmp::Scene::free()
{
  for (auto & curr : objects)
    {
      curr->freeObject();
    }

  for (auto & curr : textures)
    {
      curr.freeTexture();
    }

  skybox->freeSkybox();
}
