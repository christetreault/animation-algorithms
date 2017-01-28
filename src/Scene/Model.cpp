#include "Model.hpp"
#include "Graph.hpp"
#include <fstream>

static bool fileExists(std::string path)
{
  std::ifstream f(path, std::ios::in);

  if (f)
    {
      f.close();
      return true;
    }
  else
    {
      return false;
    }
}

dmp::Model::Model(const CommandLine & c,
                  std::vector<Object *> & objs,
                  size_t matIdx,
                  size_t texIdx)
{
  bool skelExists = fileExists(c.skelPath);
  bool skinExists = fileExists(c.skinPath);

  expect("either skin or skel file exists", skelExists || skinExists);

  mRoot = std::make_unique<Branch>();
  if (skinExists)
    {
      mSkin = std::make_unique<Skin>(c.skinPath);
      mSkin->insertInScene(mRoot.get(), objs, matIdx, texIdx);
    }
  if (skelExists)
    {
      mSkeleton = std::make_unique<Skeleton>(c.skelPath);
      mSkeleton->insertInScene(mRoot.get(), objs, matIdx, texIdx);
      if (!skinExists) mSkeleton->show();
    }
}

void dmp::Model::update(float deltaT,
                        glm::mat4 M,
                        bool dirty)
{
  mRoot->update(deltaT, M, dirty);
}
