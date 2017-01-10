#ifndef DMP_CONFIG_HPP
#define DMP_CONFIG_HPP

#include <vector>
#include <map>
#include <glm/glm.hpp>


namespace dmp
{
  static const float fieldOfView = glm::radians(60.0f);

  // some random test model for the test scene implementation
  static const char * testModel = "res/models/model.ply";

  static const float nearZ = 0.1f;
  static const float farZ = 1000.0f;

  static const size_t maxLights = 8;

  static const char * basicShader = "res/shaders/basic";

  static const char * skyBox[6] = {
    "res/textures/skyRight.tga",
    "res/textures/skyLeft.tga",
    "res/textures/skyUp.tga",
    "res/textures/skyDown.tga",
    "res/textures/skyBack.tga",
    "res/textures/skyFront.tga"
  };

  static const char * testTexture = "res/textures/test.jpg";

}

#endif
