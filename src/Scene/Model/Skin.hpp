#ifndef DMP_SKIN_HPP
#define DMP_SKIN_HPP

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <boost/tokenizer.hpp>
#include <iostream>
#include <GL/glew.h>
#include <memory>

namespace dmp
{
  class Branch;
  class Object;
  class Model;
  class Morph;

  struct SkinWeight
  {
    size_t count;
    std::vector<GLuint> index;
    std::vector<float> weight;
  };

  struct SkinData
  {
    std::string filename;
    std::vector<glm::vec3> verts;
    std::vector<size_t> idxs;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texCoords;
    std::vector<SkinWeight> weights;
    std::vector<glm::mat4> invBindings;
    std::string texFile;
  };

  class Skin
  {
  public:
    Skin() = delete;
    Skin(const Skin &) = delete;
    Skin & operator=(const Skin &) = delete;
    Skin(Skin &&) = default;
    Skin & operator=(Skin &&) = default;

    Skin(const std::string & skinPath) {initSkin(skinPath);}

    void insertInScene(std::vector<Object *> & objs,
                       size_t matIdx,
                       size_t texIdx);

    const std::string & askTexturePath() const {return mSkinData.texFile;}
    const std::vector<glm::mat4> & askBindings() const
    {
      return mSkinData.invBindings;
    }
    const std::vector<glm::vec3> & askVerts() const
    {
      return mSkinData.verts;
    }
    const std::vector<glm::vec3> & askNormals() const
    {
      return mSkinData.normals;
    }

    void tellBindingMats(const std::vector<glm::mat4> & boneM);

    void update(float deltaT, glm::mat4 M, bool dirty);
    void hide();
    void show();
    void applyMorph(const Morph & morph);
  private:
    void initSkin(const std::string & skinPath);

    SkinData mSkinData;
    bool mIsTextured = false;
    std::unique_ptr<Object> mObject;
  };
}

#endif
