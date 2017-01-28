#ifndef DMP_MODEL_HPP
#define DMP_MODEL_HPP

#include <vector>
#include "../CommandLine.hpp"
#include "Object.hpp"
#include "Model/Skeleton.hpp"
#include "Model/Skin.hpp"

namespace dmp
{
  class Branch;

  class Model
  {
  public:
    Model() = delete;
    Model(const Model &) = delete;
    Model & operator=(const Model &) = delete;
    Model(Model &&) = default;
    Model & operator=(Model &&) = default;

    Model(const CommandLine & c,
          std::vector<Object *> & objs,
          size_t matIdx,
          size_t texIdx);

    Balljoint * getSkeletonAST() // TODO: some better get-dof mechanism
    {
      expect("Skeleton not null", mSkeleton);
      return mSkeleton->getAST();
    }

    void update(float deltaT, glm::mat4 M, bool dirty);

    std::string askTexturePath() const
    {
      if (!mSkin) return "";
      return mSkin->askTexturePath();
    }

  private:
    std::unique_ptr<Skeleton> mSkeleton;
    std::unique_ptr<Skin> mSkin;
    std::unique_ptr<Branch> mRoot;
  };
}

#endif
