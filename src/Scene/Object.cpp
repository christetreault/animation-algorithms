#include "Object.hpp"

#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include "Model.hpp"

#include <glm/gtx/string_cast.hpp>

dmp::Object::Object(std::vector<ObjectVertex> verts,
                    GLenum format,
                    size_t matIdx,
                    size_t texIdx)
{
  mHasIndices = false;
  mPrimFormat = format;
  mMaterialIdx = matIdx;
  mTextureIdx = texIdx;
  mNumVerts = verts.size();

  initObject(&verts, nullptr);
}

dmp::Object::Object(std::vector<ObjectVertex> verts,
                    std::vector<GLuint> idxs,
                    GLenum format,
                    size_t matIdx,
                    size_t texIdx,
                    GLenum drawMode)
{
  mHasIndices = true;
  mPrimFormat = format;
  mMaterialIdx = matIdx;
  mTextureIdx = texIdx;
  mDrawMode = drawMode;
  mNumVerts = verts.size();
  initObject(&verts, &idxs);
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
  mNumVerts = verts.size();
  initObject(&verts, &idxs);
}

void dmp::Object::initObject(std::vector<ObjectVertex> * verts,
                             std::vector<GLuint> * idxs)
{
  clearBindingMats();
  glGenVertexArrays(1,&mVAO);
  glGenBuffers(1, &mVBO);
  if (mHasIndices) glGenBuffers(1, &mEBO);

  expectNoErrors("Gen Buffers and Arrays");
  glBindVertexArray(mVAO);

  glBindBuffer(GL_ARRAY_BUFFER, mVBO);
  glBufferData(GL_ARRAY_BUFFER,
               verts->size() * sizeof(ObjectVertex),
               verts->data(),
               mDrawMode);

  drawCount = (GLsizei) verts->size();

  expectNoErrors("Upload Vertex data");

  if (mHasIndices)
    {
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBO);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                   idxs->size() * sizeof(GLuint),
                   idxs->data(),
                   mDrawMode);

      drawCount = (GLsizei) idxs->size();

      expectNoErrors("Upload Index data");
    }

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0,        // index
                        3,        // number of components
                        GL_FLOAT, // what is the type of this thing?
                        GL_FALSE, // normalize [intMin, intMax] to [-1,1]?
                        sizeof(ObjectVertex), // how much space between things?
                        (GLvoid *) 0);        // offset of this thing

  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1,
                        3,
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

  glEnableVertexAttribArray(3);
  glVertexAttribPointer(3,
                        4,
                        GL_FLOAT,
                        GL_FALSE,
                        sizeof(ObjectVertex),
                        (GLvoid *) offsetof(ObjectVertex, weights));

  glEnableVertexAttribArray(4);
  glVertexAttribIPointer(4,
                         4,
                         GL_INT,
                         sizeof(ObjectVertex),
                         (GLvoid *) offsetof(ObjectVertex, idxs));

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

dmp::ObjectConstants dmp::Object::getObjectConstants() const
{
  ObjectConstants retVal =
    {
      mM,
      glm::mat4(glm::transpose(glm::inverse(glm::mat3(mM)))),
      {}
    };

  for (size_t i = 0; i < mBindingMats.size(); ++i)
    {
      retVal.WB[i] = mBindingMats[i];
    }

  return retVal;
}

void dmp::Object::tellBindingMats(const std::vector<glm::mat4> & mats)
{
  clearBindingMats();

  mBindingMats.reserve(mats.size());

  for (const auto & curr : mats)
    {
      mBindingMats.push_back(curr);
    }

  mDirty = true;
}

void dmp::Object::clearBindingMats()
{
  mBindingMats.clear();
  mBindingMats.resize(0);
}

void dmp::Object::draw() const
{
  expect("Object valid", mValid);
  if (!mVisible) return;
  if (mHasIndices)
    {
      glDrawElements(mPrimFormat,
                     drawCount,
                     GL_UNSIGNED_INT,
                     0); // TODO: whats up with this parameter? (its a pointer)
    }
  else
    {
      glDrawArrays(mPrimFormat,
                   0,
                   drawCount);
    }
  glCullFace(GL_BACK);
  expectNoErrors("Draw object");
}

// -----------------------------------------------------------------------------
// Primitive shape constructor
// -----------------------------------------------------------------------------

static const dmp::ObjectVertex cubeVerts[] =
  {
    {
      {-0.5f, -0.5f,  0.5f},
      {-0.5f, -0.5f,  0.5f},
      {}, // TODO: tex coords
      {0.0f, 0.0f, 0.0f, 0.0f},
      {-1, -1, -1, -1}
    },
    {
      {0.5f, -0.5f,  0.5f},
      {0.5f, -0.5f,  0.5f},
      {}, // TODO: tex coords
      {0.0f, 0.0f, 0.0f, 0.0f},
      {-1, -1, -1, -1}
    },
    {
      {0.5f,  0.5f,  0.5f},
      {0.5f,  0.5f,  0.5f},
      {}, // TODO: tex coords
      {0.0f, 0.0f, 0.0f, 0.0f},
      {-1, -1, -1, -1}
    },
    {
      {-0.5f,  0.5f,  0.5f},
      {-0.5f,  0.5f,  0.5f},
      {}, // TODO: tex coords
      {0.0f, 0.0f, 0.0f, 0.0f},
      {-1, -1, -1, -1}
    },
    {
      {-0.5f, -0.5f, -0.5f},
      {-0.5f, -0.5f, -0.5f},
      {}, // TODO: tex coords
      {0.0f, 0.0f, 0.0f, 0.0f},
      {-1, -1, -1, -1}
    },
    {
      {0.5f, -0.5f, -0.5f},
      {0.5f, -0.5f, -0.5f},
      {}, // TODO: tex coords
      {0.0f, 0.0f, 0.0f, 0.0f},
      {-1, -1, -1, -1}
    },
    {
      {0.5f,  0.5f, -0.5f},
      {0.5f,  0.5f, -0.5f},
      {}, // TODO: tex coords
      {0.0f, 0.0f, 0.0f, 0.0f},
      {-1, -1, -1, -1}
    },
    {
      {-0.5f,  0.5f, -0.5f},
      {-0.5f,  0.5f, -0.5f},
      {}, // TODO: tex coords
      {0.0f, 0.0f, 0.0f, 0.0f},
      {-1, -1, -1, -1}
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

dmp::Object::Object(Shape shape, glm::vec4 min, glm::vec4 max,
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
              glm::vec3(M * glm::vec4(cubeVerts[i].position, 1.0f)),
              glm::vec3(normalM * glm::vec4(cubeVerts[i].normal, 0.0f)),
                cubeVerts[i].texCoords,
                cubeVerts[i].weights,
                cubeVerts[i].idxs
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

void dmp::Object::updateVertices(std::function<void(ObjectVertex * data,
                                                    size_t numElems)> updateFn)
{
  glBindBuffer(GL_ARRAY_BUFFER, mVBO);
  auto buf = glMapBufferRange(GL_ARRAY_BUFFER,
                              0,
                              mNumVerts * sizeof(ObjectVertex),
                              GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);
  expectNoErrors("Map the VBO");
  expect("buffer not null", buf);

  updateFn((ObjectVertex *) buf, mNumVerts);
  expectNoErrors("call updateFn on buf");

  glUnmapBuffer(GL_ARRAY_BUFFER);
  expectNoErrors("Unmap the buffer");
}
