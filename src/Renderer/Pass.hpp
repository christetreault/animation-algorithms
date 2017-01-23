#ifndef DMP_PASS_HPP
#define DMP_PASS_HPP

#include "../config.hpp"
#include "UniformBuffer.hpp"

namespace dmp
{
  static const unsigned int drawShaded = 0;
  static const unsigned int drawNormals = 1;

  struct PassConstants
  {
    // Lights
    glm::vec4 lightColor[maxLights];
    glm::vec4 lightDir[maxLights];

    unsigned int numLights;
    unsigned int drawMode;
    glm::uvec2 numLightsPadding; // barf

    glm::mat4 P;
    glm::mat4 invP;
    glm::mat4 V;
    glm::mat4 invV;
    glm::mat4 PV;
    glm::mat4 invPV;

    glm::vec4 E;

    float nearZ;
    float farZ;
    float deltaT;
    float totalT;

    static size_t std140Size()
    {
      return dmp::std140PadStruct((std140VecArraySize<float, 4, maxLights>() * 2) +
                                  (std140VecSize<unsigned int, 4>()) +
                                  (std140MatSize<float, 4, 4>() * 6) +
                                  (std140VecSize<float, 4>()) +
                                  (std140ScalarSize<float>() * 4));
    }

    operator GLvoid *() {return (GLvoid *) this;}
  };
}

#endif
