#include "Object.hpp"

#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

dmp::Object::Object(std::vector<ObjectVertex> verts,
                    GLenum format,
                    size_t matIdx,
                    size_t texIdx)
{
  mHasIndices = false;
  mPrimFormat = format;
  mMaterialIdx = matIdx;
  mTextureIdx = texIdx;

  initObject(&verts, nullptr);
}

dmp::Object::Object(std::vector<ObjectVertex> verts,
                    std::vector<GLuint> idxs,
                    GLenum format,
                    size_t matIdx,
                    size_t texIdx)
{
  mHasIndices = true;
  mPrimFormat = format;
  mMaterialIdx = matIdx;
  mTextureIdx = texIdx;

  initObject(&verts, &idxs);
}

void dmp::Object::initObject(std::vector<ObjectVertex> * verts,
                             std::vector<GLuint> * idxs)
{
  glGenVertexArrays(1,&mVAO);
  glGenBuffers(1, &mVBO);
  if (mHasIndices) glGenBuffers(1, &mEBO);

  expectNoErrors("Gen Buffers and Arrays");
  glBindVertexArray(mVAO);

  glBindBuffer(GL_ARRAY_BUFFER, mVBO);
  glBufferData(GL_ARRAY_BUFFER,
               verts->size() * sizeof(ObjectVertex),
               verts->data(),
               GL_STATIC_DRAW);

  drawCount = (GLsizei) verts->size();

  expectNoErrors("Upload Vertex data");

  if (mHasIndices)
    {
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBO);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                   idxs->size() * sizeof(GLuint),
                   idxs->data(),
                   GL_STATIC_DRAW);

      drawCount = (GLsizei) idxs->size();

      expectNoErrors("Upload Index data");
    }

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0,        // index
                        4,        // number of components
                        GL_FLOAT, // what is the type of this thing?
                        GL_FALSE, // normalize [intMin, intMax] to [-1,1]?
                        sizeof(ObjectVertex), // how much space between things?
                        (GLvoid *) 0);        // offset of this thing

  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1,
                        4,
                        GL_FLOAT,
                        GL_FALSE,
                        sizeof(ObjectVertex),
                        (GLvoid *) offsetof(ObjectVertex, normal));

  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2,
                        2,
                        GL_FLOAT,
                        GL_FALSE,
                        sizeof(ObjectVertex),
                        (GLvoid *) offsetof(ObjectVertex, texCoords));

  expectNoErrors("Set vertex attributes");

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  expectNoErrors("Complete object init");
  mValid = true;
}

void dmp::Object::sortByMaterial(std::vector<Object *> & objs)
{
  std::sort(objs.begin(), objs.end(), [](const Object * lhs, const Object * rhs)
            {
              return lhs->mMaterialIdx < rhs->mMaterialIdx;
            });

  for (auto & curr : objs)
    {
      curr->mDirty = true;
    }
}

// -----------------------------------------------------------------------------
// Primitive shape constructor
// -----------------------------------------------------------------------------

static const dmp::ObjectVertex cubeVerts[] =
  {
    {
      {-0.5f, -0.5f,  0.5f, 1.0f},
      {-0.5f, -0.5f,  0.5f, 0.0f},
      {} // TODO: tex coords
    },
    {
      {0.5f, -0.5f,  0.5f, 1.0f},
      {0.5f, -0.5f,  0.5f, 0.0f},
      {} // TODO: tex coords
    },
    {
      {0.5f,  0.5f,  0.5f, 1.0f},
      {0.5f,  0.5f,  0.5f, 0.0f},
      {} // TODO: tex coords
    },
    {
      {-0.5f,  0.5f,  0.5f, 1.0f},
      {-0.5f,  0.5f,  0.5f, 0.0f},
      {} // TODO: tex coords
    },
    {
      {-0.5f, -0.5f, -0.5f, 1.0f},
      {-0.5f, -0.5f, -0.5f, 0.0f},
      {} // TODO: tex coords
    },
    {
      {0.5f, -0.5f, -0.5f, 1.0f},
      {0.5f, -0.5f, -0.5f, 0.0f},
      {} // TODO: tex coords
    },
    {
      {0.5f,  0.5f, -0.5f, 1.0f},
      {0.5f,  0.5f, -0.5f, 0.0f},
      {} // TODO: tex coords
    },
    {
      {-0.5f,  0.5f, -0.5f, 1.0f},
      {-0.5f,  0.5f, -0.5f, 1.0f},
      {} // TODO: tex coords
    }
  };

static const GLuint cubeIdxs[] =
  {
    0, 1, 2, //2, 1, 0,
    2, 3, 0, //0, 3, 2,
    1, 5, 6, //6, 5, 1,
    6, 2, 1, //1, 2, 6,
    7, 6, 5, //5, 6, 7,
    5, 4, 7, //7, 4, 5,
    4, 0, 3, //3, 0, 4,
    3, 7, 4, //4, 7, 3,
    4, 5, 1, //1, 5, 4,
    1, 0, 4, //4, 0, 1,
    3, 2, 6, //6, 2, 3,
    6, 7, 3  //3, 7, 6
  };

dmp::Object::Object(Shape shape, glm::vec4 min,
                    glm::vec4 max, glm::vec4 origin,
                    size_t matIdx, size_t texIdx)
{
  mHasIndices = true;
  mPrimFormat = GL_TRIANGLES;
  mMaterialIdx = matIdx;
  mTextureIdx = texIdx;

  switch (shape)
    {
    case Cube:
      glm::vec3 scaleFac = glm::vec3(max - min);
      glm::vec3 offset = glm::vec3(glm::mix(min, max, 0.5f));
      glm::mat4 trans = glm::translate(glm::mat4(), offset);
      glm::mat4 scale = glm::scale(glm::mat4(), scaleFac);
      glm::mat4 M = trans * scale;
      glm::mat4 normalM = glm::mat4(glm::transpose(glm::inverse(glm::mat3(M))));

      std::vector<ObjectVertex> v;

      for (size_t i = 0; i < 8; ++i)
        {
          v.push_back({
              M * cubeVerts[i].position,
                normalM * cubeVerts[i].normal,
                cubeVerts[i].texCoords
                });

        }

      std::vector<GLuint> idxs;

      for (size_t i = 0; i < 36; ++i)
        {
          idxs.push_back(cubeIdxs[i]);
        }

      initObject(&v, &idxs);
      break;
    }
}
