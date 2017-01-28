#ifndef DMP_CONFIG_HPP
#define DMP_CONFIG_HPP

#include <vector>
#include <map>
#include <glm/glm.hpp>


namespace dmp
{
  static const float fieldOfView = glm::radians(60.0f);

  static const float nearZ = 0.1f;
  static const float farZ = 1000.0f;

  static const size_t maxLights = 8;

  static const char * const basicShader = "res/shaders/basic";
  static const char * const skyboxShader = "res/shaders/skybox";

  static const char * const skyBox[6] = {
    "res/textures/skyRight.tga",
    "res/textures/skyLeft.tga",
    "res/textures/skyUp.tga",
    "res/textures/skyDown.tga",
    "res/textures/skyBack.tga",
    "res/textures/skyFront.tga"
  };

  static const char * const modelDir = "res/models";

}

#endif
