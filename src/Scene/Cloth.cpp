#include "Cloth.hpp"

#include <set>
#include <utility>

#include "../util.hpp"

void dmp::SpringDamper::accumulateForces()
{
  auto e = p2.pos - p1.pos;
  auto eHat = glm::normalize(e);
  float len = glm::length(e);
  float v1 = glm::dot(eHat, p1.velocity);
  float v2 = glm::dot(eHat, p2.velocity);

  auto fsd = -springConstant * (restLength - len) - dampingFactor * (v1 - v2);
  auto f = fsd * eHat;
  p1.accumulateForce(f);
  p2.accumulateForce(-f);
}

void dmp::Particle::integrate(float deltaT)
{
  auto acceleration = force / mass;

  auto vNext = velocity + acceleration * deltaT;
  auto pNext = pos + vNext * deltaT;
  force = {0.0f, 0.0f, 0.0f};
  velocity = vNext;
  pos = pNext;
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
    case ClothPrefab::flag: return 1.0f;
    case ClothPrefab::banner: return 1.0f;
    case ClothPrefab::rope: return 1.0f;
    case ClothPrefab::cube: return 1.0f;
    }
  impossible("non-exhaustive switch");
  return 0.0f;
}

static float springConstantOf(dmp::ClothPrefab p)
{
  using namespace dmp;
  switch(p)
    {
    case ClothPrefab::flag: return 1.0f;
    case ClothPrefab::banner: return 1.0f;
    case ClothPrefab::rope: return 1.0f;
    case ClothPrefab::cube: return 1.0f;
    }
  impossible("non-exhaustive switch");
  return 0.0f;
}

static float dampingFactorOf(dmp::ClothPrefab p)
{
  using namespace dmp;
  switch(p)
    {
    case ClothPrefab::flag: return 1.0f;
    case ClothPrefab::banner: return 1.0f;
    case ClothPrefab::rope: return 1.0f;
    case ClothPrefab::cube: return 1.0f;
    }
  impossible("non-exhaustive switch");
  return 0.0f;
}

static glm::vec3 spacingOf(dmp::ClothPrefab p)
{
  using namespace dmp;
  switch(p)
    {
    case ClothPrefab::flag: return {0.01f, 0.01f, 0.01f};
    case ClothPrefab::banner: return {0.01f, 0.01f, 0.001f};
    case ClothPrefab::rope: return {1.0f, 1.0f, 1.0f};
    case ClothPrefab::cube: return {1.0f, 1.0f, 1.0f};
    }
  impossible("non-exhaustive switch");
  return {};
}

static float dragCoeffOf(dmp::ClothPrefab p)
{
  using namespace dmp;
  switch(p)
    {
    case ClothPrefab::flag: return 1.0f;
    case ClothPrefab::banner: return 1.0f;
    case ClothPrefab::rope: return 1.0f;
    case ClothPrefab::cube: return 1.0f;
    }
  impossible("non-exhaustive switch");
  return 0.0f;
}

size_t dmp::Cloth::getIndex(size_t i, size_t j, size_t k)
{
  return (k * mWidth * mHeight) + (j * mWidth) + i;
}

dmp::Particle & dmp::Cloth::getParticle(size_t i, size_t j, size_t k)
{
  return mParticles[getIndex(i, j, k)];
}

float dmp::Cloth::getParticleDist(size_t i1, size_t j1, size_t k1,
                                  size_t i2, size_t j2, size_t k2)
{
  auto p1 = getParticle(i1, j1, k1);
  auto p2 = getParticle(i2, j2, k2);

  return glm::distance(p1.pos, p2.pos);
}

void dmp::Cloth::makeSpring(size_t i1, size_t j1, size_t k1,
                            size_t i2, size_t j2, size_t k2,
                            ClothPrefab type)
{
  auto springConstant = springConstantOf(type);
  auto dampingFactor = dampingFactorOf(type);

  SpringDamper sd = {springConstant,
                     dampingFactor,
                     getParticleDist(i1, j1, k1, i2, j2, k2),
                     getParticle(i1, j1, k1),
                     getParticle(i2, j2, k2)};

  mSpringDampers.push_back(sd);
}

struct Connection
{
  size_t x1, y1, z1, x2, y2, z2;

  Connection offset(size_t x, size_t y, size_t z) const
  {
    Connection retval;
    retval.x1 = x + x1;
    retval.x2 = x + x2;
    retval.y1 = y + y1;
    retval.y2 = y + y2;
    retval.z1 = z + z1;
    retval.z2 = z + z2;
    return retval;
  }
};

bool operator< (const Connection & lhs, const Connection & rhs)
{
  return (lhs.x1 < rhs.x1)
    && (lhs.x2 < rhs.x2)
    && (lhs.y1 < rhs.y1)
    && (lhs.y2 < rhs.y2)
    && (lhs.z1 < rhs.z1)
    && (lhs.z2 < rhs.z2);
};



void dmp::Cloth::connectInSteps(size_t step, ClothPrefab type)
{
  std::set<Connection> connections =
    {
      {0, 0, 0, 0, 0, step},
      {0, 0, 0, 0, step, 0},
      {0, 0, 0, 0, step, step},
      {0, 0, 0, step, 0, 0},
      {0, 0, 0, step, 0, step},
      {0, 0, 0, step, step, 0},
      {0, 0, 0, step, step, step},
      {step, 0, 0, 0, 0, step},
      {step, 0, 0, 0, step, 0},
      {step, 0, 0, 0, step, step},
      {0, step, 0, 0, 0, step},
      {0, step, 0, step, 0, 0},
      {0, step, 0, step, 0, step},
      {0, 0, step, 0, step, 0},
      {0, 0, step, step, 0, 0},
      {0, 0, step, step, step, 0}
    };
  for (size_t x = 0; x < mWidth; x = x + step)
    {
      for (size_t y = 0; y < mHeight; y = y + step)
        {
          for (size_t z = 0; z < mDepth; z = z + step)
            {
              for (const auto & curr : connections)
                {
                  auto conn = curr.offset(x, y, z);

                  if (conn.x1 < mWidth
                      && conn.x2 < mWidth
                      && conn.y1 < mHeight
                      && conn.y2 < mHeight
                      && conn.z1 < mDepth
                      && conn.z2 < mDepth)
                    {
                      makeSpring(conn.x1, conn.y1, conn.z1,
                                 conn.x2, conn.y2, conn.z2,
                                 type);
                    }
                }
            }
        }
    }
}

dmp::Cloth::Cloth(size_t width, size_t height, size_t depth, ClothPrefab type)
{
  mHeight = height;
  mWidth = width;
  mDepth = depth;

  if (type == ClothPrefab::rope || type == ClothPrefab::cube)
    {
      todo("Implement rope and cube");
    }

  mParticles.resize(width * height * depth);

  auto spacing = spacingOf(type);
  auto mass = massOf(type);
  for (size_t i = 0; i < width; ++i)
    {
      for (size_t j = 0; j < height; ++j)
        {
          for (size_t k = 0; k < depth; ++k)
            {
              if (type == ClothPrefab::banner || type == ClothPrefab::flag)
                {
                  Particle p;
                  p.pos = {(float) i * spacing.x,
                           (float) j * spacing.y,
                           (float) k * spacing.z};

                  p.mass = mass;

                  if (i == 0) p.exterior = true;
                  if (i == width - 1) p.exterior = true;
                  if (j == 0) p.exterior = true;
                  if (j == height - 1) p.exterior = true;
                  if (k == 0) p.exterior = true;
                  if (k == depth - 1) p.exterior = true;

                  if (i == 0)
                    {
                      if (j == 0)
                        {
                          p.fixed = true;
                        }
                      else if (j == height - 1 && type == ClothPrefab::flag)
                        {
                          p.fixed = true;
                        }
                    }
                  else if (i == width - 1 && j == 0 && type == ClothPrefab::banner)
                    {
                      p.fixed = true;
                    }

                  getParticle(i, j, k) = p;
                }
            }
        }
    }

  connectInSteps(1, type);

  std::vector<size_t> topRightBottomLeft(0);
  std::vector<size_t> topLeftBottomRight(0);

  for (size_t x = 0; x < mWidth - 1; ++x)
    {
      for (size_t y = 0; y < mHeight - 1; ++y)
        {
          // topLeft / bottomRight

          size_t z = 0;

          topLeftBottomRight.push_back(getIndex(x, y, z));
          topLeftBottomRight.push_back(getIndex(x, y + 1, z));
          topLeftBottomRight.push_back(getIndex(x + 1, y, z));

          topLeftBottomRight.push_back(getIndex(x + 1, y + 1, z));
          topLeftBottomRight.push_back(getIndex(x + 1, y, z));
          topLeftBottomRight.push_back(getIndex(x, y + 1, z));

          z = mDepth - 1;

          topLeftBottomRight.push_back(getIndex(x + 1, y, z));
          topLeftBottomRight.push_back(getIndex(x, y + 1, z));
          topLeftBottomRight.push_back(getIndex(x, y, z));

          topLeftBottomRight.push_back(getIndex(x, y + 1, z));
          topLeftBottomRight.push_back(getIndex(x + 1, y, z));
          topLeftBottomRight.push_back(getIndex(x + 1, y + 1, z));

          // topRight / bottomLeft

          z = 0;

          topRightBottomLeft.push_back(getIndex(x, y, z));
          topRightBottomLeft.push_back(getIndex(x + 1, y + 1, z));
          topRightBottomLeft.push_back(getIndex(x + 1, y, z));

          topRightBottomLeft.push_back(getIndex(x + 1, y + 1, z));
          topRightBottomLeft.push_back(getIndex(x, y, z));
          topRightBottomLeft.push_back(getIndex(x, y + 1, z));

          z = mDepth - 1;

          topRightBottomLeft.push_back(getIndex(x + 1, y, z));
          topRightBottomLeft.push_back(getIndex(x + 1, y + 1, z));
          topRightBottomLeft.push_back(getIndex(x, y, z));

          topRightBottomLeft.push_back(getIndex(x, y + 1, z));
          topRightBottomLeft.push_back(getIndex(x, y, z));
          topRightBottomLeft.push_back(getIndex(x + 1, y + 1, z));

        }
    }

  for (size_t y = 0; y < mHeight - 1; ++y)
    {
      for (size_t z = 0; z < mDepth - 1; ++z)
        {
          // topLeft / bottomRight

          size_t x = 0;

          topLeftBottomRight.push_back(getIndex(x, y, z));
          topLeftBottomRight.push_back(getIndex(x, y, z + 1));
          topLeftBottomRight.push_back(getIndex(x, y + 1, z));

          topLeftBottomRight.push_back(getIndex(x, y + 1, z + 1));
          topLeftBottomRight.push_back(getIndex(x, y + 1, z));
          topLeftBottomRight.push_back(getIndex(x, y, z + 1));

          x = mWidth - 1;

          topLeftBottomRight.push_back(getIndex(x, y + 1, z));
          topLeftBottomRight.push_back(getIndex(x, y, z + 1));
          topLeftBottomRight.push_back(getIndex(x, y, z));

          topLeftBottomRight.push_back(getIndex(x, y, z + 1));
          topLeftBottomRight.push_back(getIndex(x, y + 1, z));
          topLeftBottomRight.push_back(getIndex(x, y + 1, z + 1));

          // topRight / bottomLeft

          x = 0;

          topRightBottomLeft.push_back(getIndex(x, y + 1, z));
          topRightBottomLeft.push_back(getIndex(x, y, z));
          topRightBottomLeft.push_back(getIndex(x, y + 1, z + 1));

          topRightBottomLeft.push_back(getIndex(x, y, z + 1));
          topRightBottomLeft.push_back(getIndex(x, y + 1, z + 1));
          topRightBottomLeft.push_back(getIndex(x, y, z));

          x = mWidth - 1;

          topRightBottomLeft.push_back(getIndex(x, y + 1, z + 1));
          topRightBottomLeft.push_back(getIndex(x, y, z));
          topRightBottomLeft.push_back(getIndex(x, y + 1, z));

          topRightBottomLeft.push_back(getIndex(x, y, z));
          topRightBottomLeft.push_back(getIndex(x, y + 1, z + 1));
          topRightBottomLeft.push_back(getIndex(x, y, z + 1));

        }
    }

  for (size_t x = 0; x < mWidth - 1; ++x)
    {
      for (size_t z = 0; z < mDepth - 1; ++z)
        {
          // topLeft / bottomRight

          size_t y = 0;

          topLeftBottomRight.push_back(getIndex(x, y, z));
          topLeftBottomRight.push_back(getIndex(x + 1, y, z));
          topLeftBottomRight.push_back(getIndex(x, y, z + 1));

          topLeftBottomRight.push_back(getIndex(x + 1, y, z + 1));
          topLeftBottomRight.push_back(getIndex(x, y, z + 1));
          topLeftBottomRight.push_back(getIndex(x + 1, y, z));

          y = mHeight - 1;

          topLeftBottomRight.push_back(getIndex(x, y, z + 1));
          topLeftBottomRight.push_back(getIndex(x + 1, y, z));
          topLeftBottomRight.push_back(getIndex(x, y, z));

          topLeftBottomRight.push_back(getIndex(x + 1, y, z));
          topLeftBottomRight.push_back(getIndex(x, y, z + 1));
          topLeftBottomRight.push_back(getIndex(x + 1, y, z + 1));

          // topRight / bottomLeft

          y = 0;

          topRightBottomLeft.push_back(getIndex(x, y, z));
          topRightBottomLeft.push_back(getIndex(x + 1, y, z + 1));
          topRightBottomLeft.push_back(getIndex(x, y, z + 1));

          topRightBottomLeft.push_back(getIndex(x + 1, y, z + 1));
          topRightBottomLeft.push_back(getIndex(x, y, z));
          topRightBottomLeft.push_back(getIndex(x + 1, y, z));

          y = mHeight - 1;

          topRightBottomLeft.push_back(getIndex(x, y, z + 1));
          topRightBottomLeft.push_back(getIndex(x + 1, y, z + 1));
          topRightBottomLeft.push_back(getIndex(x, y, z));

          topRightBottomLeft.push_back(getIndex(x + 1, y, z));
          topRightBottomLeft.push_back(getIndex(x, y, z));
          topRightBottomLeft.push_back(getIndex(x + 1, y, z + 1));

        }
    }

  mIdxs.resize(0);
  mIdxs.insert(mIdxs.end(),
               topLeftBottomRight.begin(),
               topLeftBottomRight.end());
  mIdxs.insert(mIdxs.end(),
               topRightBottomLeft.begin(),
               topRightBottomLeft.end());


  for (size_t i = 0; i < mIdxs.size(); i = i + 3)
    {
      Triangle tri = {};
      tri.dragCoeff = dragCoeffOf(type);
      tri.p1 = &(mParticles[mIdxs[i]]);
      tri.p2 = &(mParticles[mIdxs[i+1]]);
      tri.p3 = &(mParticles[mIdxs[i+2]]);
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

void dmp::Cloth::buildObject(Transform * xform,
                             std::vector<Object *> & objects,
                             size_t matIdx,
                             size_t texIdx)
{
  mObject = xform->insert(buildObjectImpl(matIdx, texIdx));
  objects.push_back(mObject);
}

void dmp::Cloth::buildObject(Branch * branch,
                             std::vector<Object *> & objects,
                             size_t matIdx,
                             size_t texIdx)
{
  auto obj = buildObjectImpl(matIdx, texIdx);
  mObject = branch->insert(obj);
  objects.push_back(mObject);
  expect("didn't just push null", objects.back());
}

dmp::Object dmp::Cloth::buildObjectImpl(size_t matIdx,
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

  for (size_t i = 0; i < mIdxs.size() / 2; ++i)
    {
      idxs.push_back((GLuint) mIdxs[i]);
    }

  Object retval(verts, idxs, GL_TRIANGLES, matIdx, texIdx, GL_DYNAMIC_DRAW);
  return retval;
}
