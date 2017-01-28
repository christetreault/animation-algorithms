#ifndef DMP_SKIN_HPP
#define DMP_SKIN_HPP

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <boost/tokenizer.hpp>

namespace dmp
{
  class Branch;
  class Object;

  struct SkinWeight
  {
    size_t count;
    std::vector<size_t> index;
    std::vector<float> weight;
  };

  struct SkinData
  {
    std::string filename;
    std::vector<glm::vec4> verts;
    std::vector<size_t> idxs;
    std::vector<glm::vec4> normals;
    std::vector<glm::vec2> texCoords;
    std::vector<SkinWeight> weights;
    std::vector<glm::mat4> invBindings;
    std::string texFile;
  };

  class Skin // TODO: need to send binding matricies to shader using texture map
  {
  public:
    Skin() = delete;
    Skin(const Skin &) = delete;
    Skin & operator=(const Skin &) = delete;
    Skin(Skin &&) = default;
    Skin & operator=(Skin &&) = default;

    Skin(std::string skinPath) {initSkin(skinPath);}
    std::vector<glm::mat4> & getBindings() {return mSkinData.invBindings;}

    void insertInScene(Branch * graph,
                       std::vector<Object *> & objs,
                       size_t matIdx,
                       size_t texIdx);

    const std::string & askTexturePath() const {return mSkinData.texFile;}

    void hide();
    void show();
  private:
    void initSkin(std::string skinPath);

    SkinData mSkinData;
    bool mIsTextured = false;
    Object * mObjectCache;
  };
}

#endif
