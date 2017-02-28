#include "Cloth.hpp"

#include <set>
#include <utility>
#include <glm/gtc/matrix_transform.hpp>

#include "../util.hpp"

static const float maxDeltaT = 1.0f / 240.0f;
static const float speedLimit = 10.0f;
static const float groundPlane = 0.0f;
static const float yOffset = 2.25;

void dmp::Particle::accumulateForce(glm::vec3 inForce)
{
  if (fixed) return;
  force += inForce;
}

void dmp::Particle::integrateExplicitEuler(float deltaT)
{
  auto acceleration = force / mass;

  auto vNext = velocity + acceleration * deltaT;
  auto pNext = pos + vNext * deltaT;
  velocity = vNext;
  pos = pNext;
}

void dmp::Particle::integrateAdamsBashforth(float deltaT)
{
  auto accelerationCurr = force / mass;
  auto accelerationPrev = forcePrev / mass;

  auto vNext = velocity +
    ((deltaT / 2.0f) * ((3.0f * accelerationCurr) - accelerationPrev));
  auto pNext = pos +
    ((deltaT / 2.0f) * ((3.0f * vNext) - velocity));
  forcePrev = force;
  velocity = vNext;
  pos = pNext;
}

void dmp::Particle::integrate(float deltaT)
{
  if (fixed) return;
  static float highestSpeed = 0.0f;
  auto speed = fabsf(glm::length(velocity));

  if (speed > highestSpeed)
    {
      std::cerr << "new speed record: " << speed << std::endl;
      highestSpeed = speed;
    }

  bool enteredLoop = false;
  size_t iterations = 0;
  for (float step = glm::min(speedLimit / speed, 1.0f);
       step <= 1.0f;
       step = step + glm::min(speedLimit / speed, 1.0f))
    {
      enteredLoop = true;
      ++iterations;
      integrateAdamsBashforth(glm::mix(0.0f, deltaT, step));

      // if (step < 0.01f)
      //   {
      //     std::cerr << "deltaT = " << deltaT << std::endl;
      //     std::cerr << "step = " << step << std::endl;
      //     std::cerr << "speed = " << speed << std::endl;
      //     impossible("die");
      //   }
      //deltaT -= step;
      //if (deltaT > step) continue;

      // if (iterations >= 1000)
      //   {
      //     auto stepSize = glm::min(speedLimit / speed, 1.0f);
      //     std::cerr << "step = " << stepSize << std::endl;
      //     std::cerr << "deltaT = " << deltaT << std::endl;
      //     std::cerr << "increment = " << glm::mix(0.0f, deltaT, stepSize) << std::endl;
      //   }
      expect("iterations less than 1000", iterations < 1000);
    }
  expect("integrated at least once", enteredLoop);
  force = {0.0f, 0.0f, 0.0f};
}

void dmp::SpringDamper::accumulateForces()
{
  auto e = p2.pos - p1.pos;
  auto eHat = glm::normalize(e);
  float len = glm::length(e);
  float v1 = glm::dot(eHat, p1.velocity);
  float v2 = glm::dot(eHat, p2.velocity);


  auto fd = -dampingFactor * (v1 - v2);
  auto fs = -springConstant * (restLength - len);
  auto f = (fs + fd) * eHat;
  p1.accumulateForce(f);
  p2.accumulateForce(-f);

  // std::cerr << "fsd: " << fsd << std::endl;
  // std::cerr << "springConstant: " << springConstant << std::endl;
  // std::cerr << "dampingFactor: " << dampingFactor << std::endl;
  // std::cerr << "rest length: " << restLength << std::endl;
  // std::cerr << "eHat: " << glm::to_string(eHat) << std::endl;
  //std::cerr << "p1.velocity: " << glm::to_string(p1.velocity) << std::endl;
  //std::cerr << "p2.velocity: " << glm::to_string(p2.velocity) << std::endl;
}

void dmp::accumulateUniformGravity(dmp::Particle & p)
{
  auto g = p.mass * glm::vec3(0.0f, -9.8f, 0.0f);
  p.accumulateForce(g);
}

static float massOf(dmp::ClothPrefab p)
{
  using namespace dmp;
  switch(p)
    {
    default: return 1.0f;
    }
  impossible("non-exhaustive switch");
  return 0.0f;
}

static float elasticityOf(dmp::ClothPrefab p)
{
  using namespace dmp;
  switch(p)
    {
    default: return 0.1f;
    }
  impossible("non-exhaustive switch");
  return 0.0f;
}

static float frictionOf(dmp::ClothPrefab p)
{
  using namespace dmp;
  switch(p)
    {
    default: return 0.5f;
    }
  impossible("non-exhaustive switch");
  return 0.0f;
}

static float springConstantOf(dmp::ClothPrefab p, size_t step)
{
  using namespace dmp;
  switch(p)
    {
    default: return 3800.0f / (float) (step * step * step);
    }
  impossible("non-exhaustive switch");
  return 0.0f;
}

static float dampingFactorOf(dmp::ClothPrefab p, size_t step)
{
  using namespace dmp;
  switch(p)
    {
    default: return 6.5f / ((float) (step * step * step));
    }
  impossible("non-exhaustive switch");
  return 0.0f;
}

static glm::vec2 spacingOf(dmp::ClothPrefab p)
{
  using namespace dmp;
  switch(p)
    {
    default: return {1.0f, 1.0f};
    }
  impossible("non-exhaustive switch");
  return {};
}

static float dragCoeffOf(dmp::ClothPrefab p)
{
  using namespace dmp;
  switch(p)
    {
    default: return 4000.0f;
    }
  impossible("non-exhaustive switch");
  return 0.0f;
}

static float airDensityOf(dmp::ClothPrefab p)
  {
  using namespace dmp;
  switch(p)
    {
    default: return 1.0f;
    }
  impossible("non-exhaustive switch");
  return 0.0f;
}

size_t dmp::Cloth::getIndex(size_t i, size_t j)
{
  return (j * mWidth) + i;
}

dmp::Particle & dmp::Cloth::getParticle(size_t i, size_t j)
{
  return mParticles[getIndex(i, j)];
}

float dmp::Cloth::getParticleDist(size_t i1, size_t j1,
                                  size_t i2, size_t j2)
{
  auto p1 = getParticle(i1, j1);
  auto p2 = getParticle(i2, j2);

  return glm::distance(p1.pos, p2.pos);
}

void dmp::Cloth::makeSpring(size_t i1, size_t j1,
                            size_t i2, size_t j2,
                            size_t step, ClothPrefab type)
{
  auto springConstant = springConstantOf(type, step);
  auto dampingFactor = dampingFactorOf(type, step);

  SpringDamper sd = {springConstant,
                     dampingFactor,
                     getParticleDist(i1, j1, i2, j2),
                     getParticle(i1, j1),
                     getParticle(i2, j2)};

  mSpringDampers.push_back(sd);
}

struct Connection
{
  size_t x1, y1, x2, y2;

  Connection offset(size_t x, size_t y) const
  {
    Connection retval;
    retval.x1 = x + x1;
    retval.x2 = x + x2;
    retval.y1 = y + y1;
    retval.y2 = y + y2;
    return retval;
  }
};

bool operator< (const Connection & lhs, const Connection & rhs)
{
  if (lhs.x1 < rhs.x1) return true;
  else if (lhs.x2 < rhs.x2) return true;
  else if (lhs.y1 < rhs.y1) return true;
  else if (lhs.y2 < rhs.y2) return true;
  else return false;
};



void dmp::Cloth::connectInSteps(size_t step, ClothPrefab type)
{
  std::set<Connection> connections =
    {
      {0, 0, 0, step},
      {0, 0, step, 0},
      {0, 0, step, step},
      {step, 0, 0, step},
    };
  //std::cerr << "sizeof connections = " << connections.size() << std::endl;
  for (size_t x = 0; x < mWidth; x = x + step)
    {
      for (size_t y = 0; y < mHeight; y = y + step)
        {
          //std::cerr << "<x, y, z> = <" << x << ", " << y << ", " << z << ">" <<std::endl;
          //size_t count = 0;
          for (const auto & curr : connections)
            {
              auto conn = curr.offset(x, y);

              //std::cerr << conn.x1 << "->" << conn.x2;
              //std::cerr << " " << conn.y1 << "->" << conn.y2;
              //std::cerr << " " << conn.z1 << "->" << conn.z2 << std::endl;

              if (conn.x1 < mWidth
                  && conn.x2 < mWidth
                  && conn.y1 < mHeight
                  && conn.y2 < mHeight)
                {
                  //std::cerr << "taken" << std::endl;
                  makeSpring(conn.x1, conn.y1,
                             conn.x2, conn.y2,
                             step, type);
                }
              //++count;
            }
          //std::cerr << "counted: " << count << std::endl;
        }
    }
  //std::cerr << "sizeof springs: " << mSpringDampers.size() << std::endl;
  //for (const auto & curr : mSpringDampers)
  //  {
  //    std::cerr << "springConstant: " << curr.springConstant << std::endl;
  //    std::cerr << "dampingFactor: " << curr.dampingFactor << std::endl;
  //    std::cerr << "rest length: " << curr.restLength << std::endl;
  //    std::cerr << "p1: " << glm::to_string(curr.p1.pos) << std::endl;
  //    std::cerr << "p2: " << glm::to_string(curr.p2.pos) << std::endl;
  //  }
}

dmp::Cloth::Cloth(size_t width, size_t height, ClothPrefab type)
{
  mHeight = height;
  mWidth = width;

  if (type == ClothPrefab::rope || type == ClothPrefab::cube)
    {
      todo("Implement rope and cube");
    }

  mParticles.resize(width * height);

  auto spacing = spacingOf(type);
  auto mass = massOf(type);
  auto elasticity = elasticityOf(type);
  auto friction = frictionOf(type);
  float fWidth = (float) mWidth;
  for (size_t i = 0; i < width; ++i)
    {
      for (size_t j = 0; j < height; ++j)
        {
          if (type == ClothPrefab::banner)
            {
              Particle p;
              p.pos = {(((float) i * spacing.x) / fWidth) - 0.5f,
                       -(((float) j * spacing.y) / (float) mHeight) + yOffset,
                       0.0f};
              p.posInitial = p.pos;
              p.mass = mass;
              p.elasticity = elasticity;
              p.friction = friction;

              if (j == 0 && i == 0 && type == ClothPrefab::banner)
                {
                  p.fixed = true;
                }
              else if (j == 0 && i == width / 4 && type == ClothPrefab::banner)
                {
                  p.fixed = true;
                }
              else if (j == 0 && i == width / 2 && type == ClothPrefab::banner)
                {
                  p.fixed = true;
                }
              else if (j == 0 && i == (width / 2) + (width / 4) && type == ClothPrefab::banner)
                {
                  p.fixed = true;
                }
              else if (j == 0 && i == width - 1 && type == ClothPrefab::banner)
                {
                  p.fixed = true;
                }
              getParticle(i, j) = p;
            }
        }
    }

  mSpringDampers.clear();
  connectInSteps(1, type);
  connectInSteps(2, type);
  //connectInSteps(4, type);
  //connectInSteps(8, type);

  std::vector<size_t> frontFacingTopRightBottomLeft(0);
  std::vector<size_t> frontFacingTopLeftBottomRight(0);
  std::vector<size_t> backFacingTopRightBottomLeft(0);
  std::vector<size_t> backFacingTopLeftBottomRight(0);

  for (size_t x = 0; x < mWidth - 1; ++x)
    {
      for (size_t y = 0; y < mHeight - 1; ++y)
        {
          // topLeft / bottomRight

          frontFacingTopLeftBottomRight.push_back(getIndex(x, y));
          frontFacingTopLeftBottomRight.push_back(getIndex(x, y + 1));
          frontFacingTopLeftBottomRight.push_back(getIndex(x + 1, y));

          frontFacingTopLeftBottomRight.push_back(getIndex(x + 1, y + 1));
          frontFacingTopLeftBottomRight.push_back(getIndex(x + 1, y));
          frontFacingTopLeftBottomRight.push_back(getIndex(x, y + 1));

          backFacingTopLeftBottomRight.push_back(getIndex(x + 1, y));
          backFacingTopLeftBottomRight.push_back(getIndex(x, y + 1));
          backFacingTopLeftBottomRight.push_back(getIndex(x, y));

          backFacingTopLeftBottomRight.push_back(getIndex(x, y + 1));
          backFacingTopLeftBottomRight.push_back(getIndex(x + 1, y));
          backFacingTopLeftBottomRight.push_back(getIndex(x + 1, y + 1));

          // topRight / bottomLeft

          frontFacingTopRightBottomLeft.push_back(getIndex(x, y));
          frontFacingTopRightBottomLeft.push_back(getIndex(x + 1, y + 1));
          frontFacingTopRightBottomLeft.push_back(getIndex(x + 1, y));

          frontFacingTopRightBottomLeft.push_back(getIndex(x + 1, y + 1));
          frontFacingTopRightBottomLeft.push_back(getIndex(x, y));
          frontFacingTopRightBottomLeft.push_back(getIndex(x, y + 1));

          backFacingTopRightBottomLeft.push_back(getIndex(x + 1, y));
          backFacingTopRightBottomLeft.push_back(getIndex(x + 1, y + 1));
          backFacingTopRightBottomLeft.push_back(getIndex(x, y));

          backFacingTopRightBottomLeft.push_back(getIndex(x, y + 1));
          backFacingTopRightBottomLeft.push_back(getIndex(x, y));
          backFacingTopRightBottomLeft.push_back(getIndex(x + 1, y + 1));

        }
    }

  mIdxs.resize(0);
  mIdxs.insert(mIdxs.end(),
               frontFacingTopLeftBottomRight.begin(),
               frontFacingTopLeftBottomRight.end());
  mIdxs.insert(mIdxs.end(),
               backFacingTopLeftBottomRight.begin(),
               backFacingTopLeftBottomRight.end());
  mIdxs.insert(mIdxs.end(),
               frontFacingTopRightBottomLeft.begin(),
               frontFacingTopRightBottomLeft.end());
  mIdxs.insert(mIdxs.end(),
               backFacingTopRightBottomLeft.begin(),
               backFacingTopRightBottomLeft.end());

  std::vector<size_t> triIdxs = frontFacingTopLeftBottomRight;
  triIdxs.insert(triIdxs.end(),
                 frontFacingTopRightBottomLeft.begin(),
                 frontFacingTopRightBottomLeft.end());

  for (size_t i = 0; i < triIdxs.size(); i = i + 3)
    {
      Triangle tri = {};
      tri.dragCoeff = dragCoeffOf(type);
      tri.airDensity = airDensityOf(type);
      tri.p1 = &(mParticles[triIdxs[i]]);
      tri.p2 = &(mParticles[triIdxs[i+1]]);
      tri.p3 = &(mParticles[triIdxs[i+2]]);
      mTriangles.push_back(tri);
    }
}

void dmp::Cloth::regenerateTriangleData()
{
  for (auto & curr : mTriangles)
    {
      curr.velocity = (curr.p1->velocity
                       + curr.p2->velocity
                       + curr.p2->velocity) / 3.0f;
      auto norm = glm::cross((curr.p2->pos - curr.p1->pos),
                             (curr.p3->pos - curr.p1->pos));
      curr.area = glm::length(norm) / 2.0f;
      curr.normal = glm::normalize(norm);
      curr.p1->participatingNormals.push_back(curr.normal);
      curr.p2->participatingNormals.push_back(curr.normal);
      curr.p3->participatingNormals.push_back(curr.normal);

      expect("area not zero", curr.area != 0.0f);
    }
}

void dmp::Cloth::collapseNormals()
{
  for (auto & p : mParticles)
    {
      p.normal = {0.0f, 0.0f, 0.0f};
      for (auto & curr : p.participatingNormals)
        {
          p.normal += curr;
        }
      p.normal = glm::normalize(p.normal);
      p.participatingNormals.clear();
    }
}



void dmp::Cloth::buildObject(std::vector<Object *> & objects,
                             size_t matIdx,
                             size_t texIdx)
{
  buildObjectImpl(matIdx, texIdx);
  objects.push_back(mObject.get());
}

void dmp::Cloth::buildObjectImpl(size_t matIdx,
                                 size_t texIdx)
{
  regenerateTriangleData();
  collapseNormals();

  std::vector<ObjectVertex> verts(0);

  for (const auto & curr : mParticles)
    {
      ObjectVertex v = {};
      v.position = curr.pos;
      v.normal = curr.normal;
      v.texCoords = {0.0f, 0.0f};
      v.weights = {0.0f, 0.0f, 0.0f, 0.0f};
      v.idxs = {0, 0, 0, 0};
      verts.push_back(v);
    }

  std::vector<GLuint> idxs(0);

  for (size_t i = 0; i < mIdxs.size() / 4; ++i)
    {
      idxs.push_back((GLuint) mIdxs[i]);
    }

  mObject = std::make_unique<Object>(verts, idxs, GL_TRIANGLES,
                                     matIdx, texIdx, GL_DYNAMIC_DRAW);

  auto offset = glm::translate(glm::mat4(),
                               glm::vec3(0.0f, -yOffset / 2.0f, 0.0f));

  mObject->setM(offset);
}

void dmp::Cloth::update(glm::mat4 M, float deltaT)
{
  // First attempt to move all fixed particles per the scene graph
  // Find posNext as the goal position

  mTime += deltaT;
  auto windCoeff = 1.0f;
  if (mFancyWind)
    {
      windCoeff = 1.0f + 0.5f * (glm::sin(mod(mTime / 8.0f,
                                              glm::pi<float>() * 2.0f)));
    }

  std::list<Particle *> fixedParticles = {};
  for (auto & curr : mParticles)
    {
      if (curr.fixed)
        {
          curr.posPrev = curr.pos;
          curr.posNext = glm::vec3(M * glm::vec4(curr.posInitial, 1.0f));
          fixedParticles.push_back(&curr);
        }
    }

  regenerateTriangleData();
  collapseNormals();

  bool enteredLoop = false;
  float step = glm::min(maxDeltaT / deltaT, 1.0f);
  auto scaledDeltaT = glm::mix(0.0f, deltaT, step);
  for (; step <= 1.0f;
       step = step + glm::min(maxDeltaT / deltaT, 1.0f))
    {
      enteredLoop = true;

      // step the fixed particles towards posNext
      for (auto curr : fixedParticles)
        {
          curr->pos = glm::mix(curr->posPrev, curr->posNext, step);
        }

      // Then accumulate all forces
      for (auto & curr : mParticles)
        {
          accumulateUniformGravity(curr);
        }

      for (auto & curr : mSpringDampers)
        {
          curr.accumulateForces();
        }

      for (auto & curr : mTriangles)
        {
          curr.accumulateDragForces(mWindConstant * windCoeff * mWindDir);
        }

      // step the integration forward
      for (auto & curr : mParticles)
        {
          curr.integrate(scaledDeltaT);
        }
    }
  expect("integrated at least once", enteredLoop);

  for (auto & curr : mParticles)
    {
      if (curr.pos.y < groundPlane)
        {
          curr.pos.y = groundPlane - curr.pos.y;
          curr.velocity = {(1.0f - curr.friction) * curr.velocity.x,
                           -curr.elasticity * curr.velocity.y,
                           (1.0f - curr.friction) * curr.velocity.z};
          curr.forcePrev = {0.0f, 0.0f, 0.0f};
        }
    }

  // Now that the particles have moved, we need to update the Object

  auto updateFn = [&](ObjectVertex * data,
                      size_t numElems)
    {
      for (size_t i = 0; i < numElems; ++i)
        {
          data[i].position = mParticles[i].pos;
          data[i].normal = mParticles[i].normal;
        }
    };
  mObject->updateVertices(updateFn);
}

void dmp::Triangle::accumulateDragForces(glm::vec3 velocityAir)
{
  auto v = velocity - velocityAir;
  if (glm::length(v) == 0.0f) return;
  auto vHat = glm::normalize(v);
  auto a = area * glm::dot(vHat, normal);

  auto f = -0.5f
    * airDensity
    * glm::pow(glm::length(v), 2.0f)
    * dragCoeff
    * a
    * normal;

  // if (!(velocityAir.x == 0.0f && velocityAir.y == 0.0f && velocityAir.z == 0.0f))
  //   {
  //     std::cerr << "air density = " << airDensity << std::endl;
  //     std::cerr << "|v|^2 = " << glm::pow(glm::length(v), 2.0f) << std::endl;
  //     std::cerr << "drag coeff = " << dragCoeff << std::endl;
  //     std::cerr << "area = " << a << std::endl;
  //     std::cerr << "normal = " << glm::to_string(normal) << std::endl;
  //     std::cerr << "force / 3 = " << glm::to_string(f / 3.0f) << std::endl;
  //     expect("force not zero if there is wind",
  //            (f.x != 0.0f || f.y != 0.0f || f.z != 0.0f));
  //   }
  auto fParticle = f / 3.0f;

  expect("p1 not null", p1);
  p1->accumulateForce(fParticle);
  expect("p2 not null", p2);
  p2->accumulateForce(fParticle);
  expect("p3 not null", p3);
  p3->accumulateForce(fParticle);
}

void dmp::Cloth::setWind(glm::vec3 windDir, float windConstant, bool fancyWind)
{
  if (roughEq(windDir.x, mWindDir.x)
      && roughEq(windDir.y, mWindDir.y)
      && roughEq(windDir.z, mWindDir.z)
      && roughEq(windConstant, mWindConstant))
    {
      std::cerr << "clear wind" << std::endl;
      mWindDir = {0.0f, 0.0f, 0.0f};
    }
  else mWindDir = windDir;

  mWindConstant = windConstant;
  mFancyWind = fancyWind;
}
