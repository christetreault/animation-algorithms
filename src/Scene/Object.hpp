#ifndef DMP_SCENE_OBJECT_HPP
#define DMP_SCENE_OBJECT_HPP

#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include "Types.hpp"
#include "../util.hpp"
#include "../Renderer/UniformBuffer.hpp"

#include <iostream>

namespace dmp
{
  class Model;

  enum Shape
    {
      Cube
    };

  struct ObjectVertex
  {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;

    glm::vec4 weights;
    glm::ivec4 idxs;
  };

  struct ObjectConstants
  {
    glm::mat4 M;
    glm::mat4 normalM;

    glm::mat4 WB[128];

    static size_t std140Size()
    {
      return dmp::std140PadStruct((std140MatSize<float, 4, 4>() * (2 + 128)));
    }

    operator GLvoid *() {return (GLvoid *) this;}
  };

  class Object
  {
  public:
    Object() = delete;
    Object(const Object &) = default;
    Object & operator=(const Object &) = default;

    ~Object() {}

    void freeObject()
    {
      if (!mValid) return;

      glDeleteVertexArrays(1, &mVAO);
      glDeleteBuffers(1, &mVBO);
      if (mHasIndices) glDeleteBuffers(1, &mEBO);
    }

    Object(std::vector<ObjectVertex> verts,
           std::vector<GLuint> idxs,
           GLenum format,
           size_t matIdx,
           size_t texIdx,
           GLenum cullFace = GL_BACK);

    Object(std::vector<ObjectVertex> verts,
           GLenum format,
           size_t matIdx,
           size_t texIdx);
    Object(std::vector<ObjectVertex> verts,
           std::vector<GLuint> idxs,
           GLenum format,
           size_t matIdx,
           size_t texIdx);

    Object(Shape shape, glm::vec4 min, glm::vec4 max,
           size_t matIdx, size_t texIdx);

    bool isDirty() const {return mDirty && mVisible;}
    void setClean() {mDirty = false;}
    void setM(glm::mat4 M)
    {
      mM = M;
      mDirty = true;
    }

    void bind() const
    {
      expect("Object valid", mValid);
      expectNoErrors("Pre-bind object");
      glBindVertexArray(mVAO);
      expectNoErrors("Bind object");
    }

    void draw() const
    {
      expect("Object valid", mValid);
      if (!mVisible) return;
      glCullFace(mCullFace);
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

    ObjectConstants getObjectConstants() const;
    void tellBindingMats(const std::vector<glm::mat4> & mats);
    void clearBindingMats();

    glm::mat4 getM() const {return mM;}

    size_t materialIndex() const {return mMaterialIdx;}
    size_t textureIndex() const {return mTextureIdx;}

    static void sortByMaterial(std::vector<Object *> & objs);

    void show()
    {
      if (mVisible) return;
      mVisible = true;
      mDirty = true;
    };

    void hide()
    {
      mVisible = false;
    }

  private:
    void initObject(std::vector<ObjectVertex> * verts,
                    std::vector<GLuint> * idxs);

    GLuint mVAO = 0;
    GLuint mVBO = 0;
    GLuint mEBO = 0;

    bool mDirty = true;
    glm::mat4 mM;
    bool mHasIndices;
    GLenum mPrimFormat;
    bool mValid = false;
    size_t mMaterialIdx;
    size_t mTextureIdx;

    GLsizei drawCount;
    bool mVisible = true;
    GLenum mCullFace = GL_BACK;

    std::vector<glm::mat4> mBindingMats;
  };
}

#endif
