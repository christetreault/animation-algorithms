#ifndef DMP_CLOTH_HPP
#define DMP_CLOTH_HPP

#include <vector>
#include <list>
#include <utility>
#include <memory>
#include <glm/glm.hpp>

#include "Object.hpp"

namespace dmp
{
  enum class ClothPrefab
  {
    banner, rope, cube
  };

  struct Particle
  {
    glm::vec3 posPrev = {0.0f, 0.0f, 0.0f};
    glm::vec3 pos = {0.0f, 0.0f, 0.0f};
    glm::vec3 posInitial = {0.0f, 0.0f, 0.0f};
    glm::vec3 posNext = {0.0f, 0.0f, 0.0f};
    glm::vec3 velocity = {0.0f, 0.0f, 0.0f};
    glm::vec3 normal = {0.0f, 0.0f, 0.0f};

    float mass = 0.0f;
    float momentum = 0.0f;

    void clearForce() {force = {};}
    void accumulateForce(glm::vec3 inForce);
    void integrate(float deltaT);
    void integrateExplicitEuler(float deltaT);
    void integrateAdamsBashforth(float deltaT);
    glm::vec3 force = {0.0f, 0.0f, 0.0f};
    glm::vec3 forcePrev = {0.0f, 0.0f, 0.0f};
    bool fixed = false;

    std::vector<glm::vec3> participatingNormals = {};
  };

  void accumulateUniformGravity(Particle & p);

  struct SpringDamper
  {
    float springConstant = 0.0f;
    float dampingFactor = 0.0f;
    float restLength = 0.0f;
    Particle & p1;
    Particle & p2;

    void accumulateForces();
  };

  struct Triangle
  {
    glm::vec3 normal = {0.0f, 0.0f, 0.0f};
    glm::vec3 velocity = {0.0f, 0.0f, 0.0f};
    float area = 0.0f;
    float airDensity = 0.0f; // TODO: function of wind?
    float dragCoeff = 0.0f;

    Particle * p1 = nullptr;
    Particle * p2 = nullptr;
    Particle * p3 = nullptr;

    void accumulateDragForces(glm::vec3 velocityAir);
  };

  class Cloth
  {
  public:
    Cloth() = delete;
    Cloth(const Cloth &) = delete;
    Cloth & operator=(const Cloth &) = delete;
    Cloth(Cloth &&) = default;
    Cloth & operator=(Cloth &&) = default;

    Cloth(size_t width, size_t height, ClothPrefab type);

    void buildObject(std::vector<Object *> & objects,
                     size_t matIdx,
                     size_t texIdx);
    Particle & getParticle(size_t i, size_t j);

    void update(glm::mat4 M, float deltaT);
    void setWind(glm::vec3 windDir);
  private:
    void regenerateTriangleData();
    void collapseNormals();
    size_t getIndex(size_t i, size_t j);
    void connectInSteps(size_t step, ClothPrefab type);
    void makeSpring(size_t i1, size_t j1,
                    size_t i2, size_t j2,
                    size_t step, ClothPrefab type);
    float getParticleDist(size_t i1, size_t j1,
                          size_t i2, size_t j2);
    void buildObjectImpl(size_t matIdx,
                         size_t texIdx);
    std::unique_ptr<Object> mObject = nullptr;

    std::vector<Particle> mParticles;
    std::vector<SpringDamper> mSpringDampers;
    // mIdxs data layout:
    // [0, (size/2) - 1] ->
    // [topLeftTri[0], bottomRightTri[0]
    //  topLeftTri[1], bottomRightTri[1]
    // ...
    //  topLeftTri[N], bottomRightTri[N]]
    // [size/2, size-1] ->
    // [topRightTri[0], bottomLeftTri[0]
    //  topRightTri[1], bottomLeftTri[1]
    // ...
    //  topRightTri[N], bottomLeftTri[N]]
    // counterclockwise winding order
    std::vector<size_t> mIdxs;
    std::vector<Triangle> mTriangles;
    size_t mHeight;
    size_t mWidth;
    glm::vec3 mWindDir;
  };


}

#endif
