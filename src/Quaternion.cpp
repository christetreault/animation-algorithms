#include "Quaternion.hpp"
#include "util.hpp"

#include <iostream>

dmp::Quaternion::Quaternion(float theta, dmp::RotationAxis a)
{
  switch (a)
    {
    case RotationAxis::X:
      makeRotation(theta, glm::vec3(1.0f, 0.0f, 0.0f));
      return;
    case RotationAxis::Y:
      makeRotation(theta, glm::vec3(0.0f, 1.0f, 0.0f));
      return;
    case RotationAxis::Z:
      makeRotation(theta, glm::vec3(0.0f, 0.0f, 1.0f));
      return;
    }
  impossible("axis switch non-exhaustive");
}

dmp::Quaternion::Quaternion(float theta, glm::vec3 a)
{
  makeRotation(theta, a);
}

dmp::Quaternion dmp::Quaternion::operator*(const dmp::Quaternion & rhs) const
{
  Quaternion q;

  q.real = real * rhs.real - glm::dot(imaginary, rhs.imaginary);
  q.imaginary = real * rhs.imaginary
    + rhs.real * imaginary
    + glm::cross(imaginary, rhs.imaginary);

  expect("multiply: does not contain NaN",
         !std::isnan(q.real) && !std::isnan(q.imaginary.x)
         && !std::isnan(q.imaginary.y) && !std::isnan(q.imaginary.z));

  return q.normalize();
}

dmp::Quaternion dmp::Quaternion::operator*(float rhs) const
{
  Quaternion q;

  q.real = rhs * real;
  q.imaginary = rhs * imaginary;

  expect("scalar multiply: does not contain NaN",
         !std::isnan(q.real) && !std::isnan(q.imaginary.x)
         && !std::isnan(q.imaginary.y) && !std::isnan(q.imaginary.z));

  return q;
}

dmp::Quaternion dmp::Quaternion::operator+(const dmp::Quaternion & rhs) const
{
  Quaternion q;

  q.real = real + rhs.real;
  q.imaginary = imaginary + rhs.imaginary;

  expect("addition: does not contain NaN",
         !std::isnan(q.real) && !std::isnan(q.imaginary.x)
         && !std::isnan(q.imaginary.y) && !std::isnan(q.imaginary.z));

  return q.normalize();
}

bool dmp::Quaternion::operator==(const dmp::Quaternion & rhs) const
{
  return roughEq(real, rhs.real, std::numeric_limits<float>::epsilon())
    && roughEq(imaginary.x, rhs.imaginary.x, std::numeric_limits<float>::epsilon())
    && roughEq(imaginary.y, rhs.imaginary.y, std::numeric_limits<float>::epsilon())
    && roughEq(imaginary.z, rhs.imaginary.z, std::numeric_limits<float>::epsilon());
}

dmp::Quaternion::operator glm::mat4() const
{
  auto q = normalize();
  float m11 = 1.0f - 2.0f * q.q2() * q.q2() - 2.0f * q.q3() * q.q3();
  float m12 =        2.0f * q.q1() * q.q2() - 2.0f * q.q0() * q.q3();
  float m13 =        2.0f * q.q1() * q.q3() + 2.0f * q.q0() * q.q2();
  float m21 =        2.0f * q.q1() * q.q2() + 2.0f * q.q0() * q.q3();
  float m22 = 1.0f - 2.0f * q.q1() * q.q1() - 2.0f * q.q3() * q.q3();
  float m23 =        2.0f * q.q2() * q.q3() - 2.0f * q.q0() * q.q1();
  float m31 =        2.0f * q.q1() * q.q3() - 2.0f * q.q0() * q.q2();
  float m32 =        2.0f * q.q2() * q.q3() + 2.0f * q.q0() * q.q1();
  float m33 = 1.0f - 2.0f * q.q1() * q.q1() - 2.0f * q.q2() * q.q2();

  // TODO: transpose?
  glm::mat4 M =
    {
      m11,  m12,  m13, 0.0f,
      m21,  m22,  m23, 0.0f,
      m31,  m32,  m33, 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f
    };
  return glm::transpose(M);
}

dmp::Quaternion dmp::Quaternion::operator-() const
{
  Quaternion q;

  q.real = -real;
  q.imaginary = -imaginary;

  expect("negation: does not contain NaN",
         !std::isnan(q.real) && !std::isnan(q.imaginary.x)
         && !std::isnan(q.imaginary.y) && !std::isnan(q.imaginary.z));

  return q.normalize();
}

dmp::Quaternion dmp::Quaternion::normalize() const
{
  Quaternion q;

  float l = length();
  if (roughEq(l, 0.0f)) return *this;

  q.real = real / l;
  q.imaginary = imaginary / l;

  expect("normalize: does not contain NaN",
         !std::isnan(q.real) && !std::isnan(q.imaginary.x)
         && !std::isnan(q.imaginary.y) && !std::isnan(q.imaginary.z));

  return q;
}

float dmp::Quaternion::length() const
{
  return glm::sqrt(glm::pow(real, 2.0f)
                   + glm::pow(imaginary.x, 2.0f)
                   + glm::pow(imaginary.y, 2.0f)
                   + glm::pow(imaginary.z, 2.0f));
}

void dmp::Quaternion::makeRotation(float theta, glm::vec3 a)
{
  real = glm::cos(theta / 2.0f);
  imaginary = a * glm::sin(theta / 2.0f);
}

float dmp::dot(const dmp::Quaternion & lhs,
               const dmp::Quaternion & rhs)
{
  auto l = lhs.normalize();
  auto r = rhs.normalize();
  return l.real * r.real + glm::dot(l.imaginary, r.imaginary);
}

float dmp::angleBetween(const dmp::Quaternion & lhs,
                        const dmp::Quaternion & rhs)
{
  //std::cerr << "lhs . rhs = " << dot(lhs, rhs) << std::endl;
  //std::cerr << "acos(lhs . rhs) = " << glm::acos(dot(lhs, rhs)) << std::endl;
  //std::cerr << "lhs = <" << lhs.real << ", " << glm::to_string(lhs.imaginary) << ">" << std::endl;
  //std::cerr << "rhs = <" << rhs.real << ", " << glm::to_string(rhs.imaginary) << ">" << std::endl;

  return glm::acos(glm::min(dot(lhs, rhs), 1.0f));
}

dmp::Quaternion dmp::slerp(float t,
                           const dmp::Quaternion & lhs,
                           const dmp::Quaternion & rhs,
                           bool forceShortPath)
{
  auto l = lhs.normalize();
  auto r = rhs.normalize();

  if (forceShortPath && dot(l, r) < 0.0f) l = -l;

  auto theta = angleBetween(l, r);
  auto sinTheta = glm::sin(theta);

  if (roughEq(sinTheta, 0.0f)) return lhs;

  //std::cerr << "theta = " << theta << std::endl;
  //std::cerr << "sin(theta) = " << sinTheta << std::endl;

  auto lc = (glm::sin(1.0f - t) * theta) / sinTheta;
  auto rc = (glm::sin(t) * theta) / sinTheta;

  auto q = (l * lc) + (r * rc);

  //std::cerr << "q = <" << q.real << ", " << glm::to_string(q.imaginary) << ">" << std::endl;
  expect("slerp: does not contain NaN",
         !std::isnan(q.real) && !std::isnan(q.imaginary.x)
         && !std::isnan(q.imaginary.y) && !std::isnan(q.imaginary.z));

  return q.normalize();
}

dmp::Quaternion dmp::catmullRom(float t,
                                const dmp::Quaternion & q0,
                                const dmp::Quaternion & q1,
                                const dmp::Quaternion & q2,
                                const dmp::Quaternion & q3,
                                bool forceShortPath)
{
  auto q00 = q0.normalize();
  auto q01 = q1.normalize();
  auto q02 = q2.normalize();
  auto q03 = q3.normalize();

  if (forceShortPath && dot(q00, q01) < 0.0f) q01 = -q01;
  if (forceShortPath && dot(q01, q02) < 0.0f) q02 = -q02;
  if (forceShortPath && dot(q02, q03) < 0.0f) q03 = -q03;

  auto q10 = slerp(t + 1.0f, q00, q01, false);
  auto q11 = slerp(t, q01, q02, false);
  auto q12 = slerp(t - 1.0f, q02, q03, false);
  auto q20 = slerp((t + 1.0f) / 2.0f, q10, q11, false);
  auto q21 = slerp(t / 2.0f, q11, q12, false);
  return slerp(t, q20, q21, false);
}
