#ifndef DMP_POSE_HPP
#define DMP_POSE_HPP

#include <vector>
#include <glm/glm.hpp>

namespace dmp
{
  struct Pose
  {
    glm::vec3 translation;
    std::vector<glm::vec3> rotations;
  };
}

#endif
