#ifndef DMP_QUATERNION_HPP
#define DMP_QUATERNION_HPP

#include <glm/glm.hpp>

namespace dmp
{
  enum class RotationAxis {X, Y, Z};

  struct Quaternion
  {
  public:
    // -------------------------------------------------------------------------
    // Constructors
    // -------------------------------------------------------------------------
    Quaternion() {real = 1.0f; imaginary = {0.0f, 0.0f, 0.0f};}
    Quaternion(const Quaternion &) = default;
    Quaternion & operator=(const Quaternion &) = default;
    Quaternion(Quaternion &&) = default;
    Quaternion & operator=(Quaternion &&) = default;
    Quaternion(float theta, RotationAxis a);
    Quaternion(float theta, glm::vec3 a);

    // -------------------------------------------------------------------------
    // Operations
    // -------------------------------------------------------------------------

    Quaternion operator*(const Quaternion & rhs) const;
    Quaternion operator*(float rhs) const;
    Quaternion operator+(const Quaternion & rhs) const;
    bool operator==(const Quaternion & rhs) const;
    operator glm::mat4() const;
    operator glm::vec4() const;
    Quaternion operator-() const;
    Quaternion normalize() const;
    float length() const;
    void makeRotation(float theta, glm::vec3 a);

    // -------------------------------------------------------------------------
    // Fields
    // -------------------------------------------------------------------------

    float real;
    glm::vec3 imaginary;

    float q0() const {return real;}
    float q1() const {return imaginary.x;}
    float q2() const {return imaginary.y;}
    float q3() const {return imaginary.z;}
  };

  float dot(const Quaternion & lhs, const Quaternion & rhs);
  float angleBetween(const Quaternion & lhs, const Quaternion & rhs);
  Quaternion slerp(float t,
                   const Quaternion & lhs,
                   const Quaternion & rhs,
                   bool forceShortPath = true);

  Quaternion catmullRom(float t,
                        const Quaternion & q0,
                        const Quaternion & q1,
                        const Quaternion & q2,
                        const Quaternion & q3,
                        bool forceShortPath = true);
}

#endif
